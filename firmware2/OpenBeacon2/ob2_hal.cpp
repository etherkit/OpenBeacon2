#include "ob2_hal.h"
#include "ob2_pwm.h"

#include <Wire.h>
#include <si5351.h>

static_assert(!OB2_RELAY_PWM_SHIPPED, "release relays stay GPIO");
static_assert(!OB2_TCA0_SERVES_PA5_AND_PB1, "TCA0 has one PORTMUX setting");
static_assert(!OB2_TCB_DRIVES_RELAY_PINS, "no TCB waveform on PA5, PA6, or PB1");

static const uint8_t kButtonPins[Ob2BtnCount] = {
    OB2_BTN_UP,   OB2_BTN_DOWN, OB2_BTN_LEFT, OB2_BTN_RIGHT,
    OB2_BTN_BACK, OB2_BTN_DSP_1, OB2_BTN_DSP_2,
};

static Si5351 g_si5351;
static bool g_synth_ok = false;
static bool g_boost_on = false;
static bool g_cc_sampled = false;
static bool g_eeprom_ok = false;
static bool g_eeprom_busy = false;
static uint32_t g_eeprom_write_ms = 0;

static void relay_pin_low(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

static void clear_pullup(volatile uint8_t* ctrl) {
  *ctrl = static_cast<uint8_t>(*ctrl & ~PORT_PULLUPEN_bm);
}

static bool eeprom_read(uint8_t* out) {
  Wire.beginTransmission(OB2_EEPROM_ADDR);
  Wire.write(static_cast<uint8_t>(0));
  Wire.write(static_cast<uint8_t>(0));
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  if (Wire.requestFrom(static_cast<int>(OB2_EEPROM_ADDR),
                       static_cast<int>(OB2_EEPROM_MAGIC_LEN)) != OB2_EEPROM_MAGIC_LEN) {
    return false;
  }
  for (uint8_t i = 0; i < OB2_EEPROM_MAGIC_LEN; ++i) {
    out[i] = static_cast<uint8_t>(Wire.read());
  }
  return true;
}

static bool eeprom_magic_ok(const uint8_t* bytes) {
  for (uint8_t i = 0; i < OB2_EEPROM_MAGIC_LEN; ++i) {
    if (bytes[i] != OB2_EEPROM_MAGIC[i]) {
      return false;
    }
  }
  return true;
}

static void eeprom_start_write(uint32_t now_ms) {
  Wire.beginTransmission(OB2_EEPROM_ADDR);
  Wire.write(static_cast<uint8_t>(0));
  Wire.write(static_cast<uint8_t>(0));
  for (uint8_t i = 0; i < OB2_EEPROM_MAGIC_LEN; ++i) {
    Wire.write(OB2_EEPROM_MAGIC[i]);
  }
  if (Wire.endTransmission() != 0) {
    g_eeprom_ok = false;
    g_eeprom_busy = false;
    return;
  }
  g_eeprom_busy = true;
  g_eeprom_ok = false;
  g_eeprom_write_ms = now_ms;
}

static void gpio_begin() {
  relay_pin_low(OB2_BAND_A);
  relay_pin_low(OB2_BAND_B);
  relay_pin_low(OB2_BAND_C);
  /* Section 4.5 still says pull PB1 up. PB1 is BAND_C with R91 to ground. */
  clear_pullup(&PORTA.PIN5CTRL);
  clear_pullup(&PORTA.PIN6CTRL);
  clear_pullup(&PORTB.PIN1CTRL);

  relay_pin_low(OB2_TX);
  clear_pullup(&PORTA.PIN4CTRL);
  g_boost_on = false;

  pinMode(OB2_SYNC_LED, OUTPUT);
  digitalWrite(OB2_SYNC_LED, LOW);

  for (uint8_t i = 0; i < Ob2BtnCount; ++i) {
    pinMode(kButtonPins[i], INPUT);
  }
}

static void adc_begin() {
  /* DxCore init leaves the ADC in 10-bit mode on VDD. Section 4.14 wants 12-bit and 2.5 V. */
  VREF.ADC0REF = static_cast<uint8_t>((VREF.ADC0REF & ~VREF_REFSEL_gm) | VREF_REFSEL_2V500_gc);
  ADC0.CTRLA = static_cast<uint8_t>((ADC0.CTRLA & ~ADC_RESSEL_gm) | ADC_ENABLE_bm | ADC_RESSEL_12BIT_gc);
  PORTE.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTE.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTE.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
}

static uint16_t adc_convert(uint8_t channel) {
  ADC0.MUXPOS = static_cast<uint8_t>(channel << ADC_MUXPOS_gp);
  ADC0.INTFLAGS = ADC_RESRDY_bm;
  ADC0.COMMAND = ADC_STCONV_bm;
  while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) {
  }
  /* DA-series erratum: point the mux at a spare channel before reading RES. */
  ADC0.MUXPOS = 0x40;
  return ADC0.RES;
}

void ob2_adc_sample(uint16_t* isense_counts, uint16_t* cc1_counts, uint16_t* cc2_counts) {
  const uint16_t isense = adc_convert(static_cast<uint8_t>(OB2_AIN_TX_ISENSE));
  const uint16_t cc1 = adc_convert(static_cast<uint8_t>(OB2_AIN_CC1_SENSE));
  const uint16_t cc2 = adc_convert(static_cast<uint8_t>(OB2_AIN_CC2_SENSE));
  g_cc_sampled = true;
  if (isense_counts != nullptr) {
    *isense_counts = isense;
  }
  if (cc1_counts != nullptr) {
    *cc1_counts = cc1;
  }
  if (cc2_counts != nullptr) {
    *cc2_counts = cc2;
  }
}

bool ob2_cc_sampled() {
  return g_cc_sampled;
}

void ob2_relays_write(Ob2Relay relay) {
  if (!relay.legal) {
    relay.a = 0;
    relay.b = 0;
    relay.c = 0;
  }
  digitalWrite(OB2_BAND_A, relay.a ? HIGH : LOW);
  digitalWrite(OB2_BAND_B, relay.b ? HIGH : LOW);
  digitalWrite(OB2_BAND_C, relay.c ? HIGH : LOW);
}

void ob2_boost(bool on) {
  digitalWrite(OB2_TX, on ? HIGH : LOW);
  g_boost_on = on;
}

bool ob2_boost_is_on() {
  return g_boost_on;
}

void ob2_sync(bool on) {
  digitalWrite(OB2_SYNC_LED, on ? HIGH : LOW);
}

bool ob2_button_raw(uint8_t index) {
  if (index >= Ob2BtnCount) {
    return false;
  }
  return digitalRead(kButtonPins[index]) == LOW;
}

void ob2_bias_mv(uint16_t mv) {
  const uint16_t code = ob2_mcp4725_code(mv);
  Wire.beginTransmission(OB2_MCP4725_ADDR);
  Wire.write(static_cast<uint8_t>((code >> 8) & 0x0F));
  Wire.write(static_cast<uint8_t>(code & 0xFF));
  Wire.endTransmission();
}

static void eeprom_begin(uint32_t now_ms) {
  uint8_t bytes[OB2_EEPROM_MAGIC_LEN] = {0, 0, 0, 0};
  if (eeprom_read(bytes) && eeprom_magic_ok(bytes)) {
    g_eeprom_ok = true;
    g_eeprom_busy = false;
    return;
  }
  eeprom_start_write(now_ms);
}

void ob2_eeprom_poll(uint32_t now_ms) {
  if (!g_eeprom_busy) {
    return;
  }
  Wire.beginTransmission(OB2_EEPROM_ADDR);
  const uint8_t err = Wire.endTransmission();
  if (err != 0) {
    if ((now_ms - g_eeprom_write_ms) > 10) {
      g_eeprom_busy = false;
      g_eeprom_ok = false;
    }
    return;
  }
  uint8_t bytes[OB2_EEPROM_MAGIC_LEN] = {0, 0, 0, 0};
  g_eeprom_busy = false;
  g_eeprom_ok = eeprom_read(bytes) && eeprom_magic_ok(bytes);
}

bool ob2_eeprom_ok() {
  return g_eeprom_ok;
}

bool ob2_eeprom_busy() {
  return g_eeprom_busy;
}

static void synth_begin() {
  g_synth_ok = g_si5351.init(SI5351_CRYSTAL_LOAD_0PF, 25000000UL, 0);
  if (!g_synth_ok) {
    return;
  }
  /* 8 mA is the old CLK0 drive. Section 4.4 does not restate it. */
  g_si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  g_si5351.output_enable(SI5351_CLK0, 0);
  g_si5351.output_enable(SI5351_CLK2, 0);
  g_si5351.output_enable(SI5351_CLK4, 0);
}

bool ob2_synth_ok() {
  return g_synth_ok;
}

void ob2_carrier(uint32_t hz, bool on) {
  if (!g_synth_ok) {
    return;
  }
  if (!on) {
    g_si5351.output_enable(SI5351_CLK0, 0);
    return;
  }
  g_si5351.set_freq(static_cast<uint64_t>(hz) * 100ULL, SI5351_CLK0);
  g_si5351.output_enable(SI5351_CLK0, 1);
}

void ob2_hal_begin() {
  gpio_begin();
  adc_begin();

  Wire.begin();
  Wire.setClock(OB2_TWI_HZ);
  ob2_bias_mv(0);

  const uint32_t now = millis();
  eeprom_begin(now);
  synth_begin();
  Wire.setClock(OB2_TWI_HZ);
  const bool display = ob2_display_begin();
  ob2_adc_sample(nullptr, nullptr, nullptr);

  Serial.begin(115200);
  Serial.print(F("OB2 M1 synth="));
  Serial.print(g_synth_ok ? F("ok") : F("no"));
  Serial.print(F(" oled="));
  Serial.print(display ? F("ok") : F("no"));
  Serial.print(F(" ee="));
  Serial.println(g_eeprom_busy ? F("wr") : (g_eeprom_ok ? F("ok") : F("fail")));
}
