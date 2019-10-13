void selectMode(uint8_t sel)
{
  switch (static_cast<Mode>(sel))
  {
    case Mode::DFCW3:
//      mode = Mode::DFCW3;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW3;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      settings["TX Intv"] = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::DFCW6:
//      mode = Mode::DFCW6;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW6;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::DFCW10:
//      mode = Mode::DFCW10;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW10;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::DFCW120:
//      mode = Mode::DFCW120;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW120;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS3:
//      mode = Mode::QRSS3;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS3;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS6:
//      mode = Mode::QRSS6;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS6;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS10:
//      mode = Mode::QRSS10;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS10;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS120:
//      mode = Mode::QRSS120;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS120;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::CW:
//      mode = Mode::CW;
//      cur_config.mode = mode;
      cur_config.mode = Mode::CW;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::HELL:
//      mode = Mode::HELL;
//      cur_config.mode = mode;
      cur_config.mode = Mode::HELL;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::WSPR:
//      mode = Mode::WSPR;
//      cur_config.mode = mode;
      cur_config.mode = Mode::WSPR;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
//      selectBuffer(cur_buffer);
//      composeMFSKMessage();
//      composeBuffer(1);
//      memset(mfsk_buffer, 0, 255);
//      jtencode.wspr_encode(cur_callsign, cur_grid, cur_power, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].wspr_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::JT65:
//      mode = Mode::JT65;
//      cur_config.mode = mode;
      cur_config.mode = Mode::JT65;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
      selectBuffer(cur_config.buffer);
//      composeMFSKMessage();
//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.jt65_encode(msg_buffer, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].jt65_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::JT9:
//      mode = Mode::JT9;
//      cur_config.mode = mode;
      cur_config.mode = Mode::JT9;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
      selectBuffer(cur_config.buffer);
//      composeMFSKMessage();
//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.jt9_encode(msg_buffer, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].jt9_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::JT4:
//      mode = Mode::JT4;
//      cur_config.mode = mode;
      cur_config.mode = Mode::JT4;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
      selectBuffer(cur_config.buffer);
//      composeMFSKMessage();
//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.jt4_encode(msg_buffer, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].jt9_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::FT8:
//      mode = Mode::FT8;
//      cur_config.mode = mode;
      cur_config.mode = Mode::FT8;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
//      selectBuffer(cur_buffer);
      composeMFSKMessage();

//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.ft8_encode(msg_buffer, mfsk_buffer);
//      SerialUSB.write('\v');
//      for(uint8_t i = 0; i < FT8_SYMBOL_COUNT; ++i)
//      {
//        SerialUSB.print(mfsk_buffer[i]);
//      }
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
//      SerialUSB.write('\v');
//      SerialUSB.print(cur_tone_spacing, DEC);
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].ft8_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
  }
  serializeConfig();
  if(next_tx != UINT32_MAX)
  {
    setNextTx(cur_config.tx_intv);
  }
  yield();
}

void setBuffer(const char * b, const char * value)
//void setBuffer(std::string b, std::string value)
{
  std::size_t pos;
  uint8_t buf;
  buf = atoi(b);

  if (buf >= 1 && buf <= 4)
  {
    switch (buf)
    {
      case 1:
        cur_setting_str = cur_config.msg_buffer_1;
        break;
      case 2:
        cur_setting_str = cur_config.msg_buffer_2;
        break;
      case 3:
        cur_setting_str = cur_config.msg_buffer_3;
        break;
      case 4:
        cur_setting_str = cur_config.msg_buffer_4;
        break;
    }
    cur_edit_buffer = buf;
    cur_setting_type = SettingType::Str;
//    display_mode = DisplayMode::Setting;
    display_mode = DisplayMode::Buffer;
    cur_setting_selected = 0;
    pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
    if (pos != std::string::npos)
    {
      cur_setting_index = (uint8_t)pos;
    }
  }
}

void selectBuffer(const uint8_t buf)
{
  switch(buf)
  {
  case 1:
    strcpy(msg_buffer, cur_config.msg_buffer_1);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  case 2:
    strcpy(msg_buffer, cur_config.msg_buffer_2);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  case 3:
    strcpy(msg_buffer, cur_config.msg_buffer_3);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  case 4:
    strcpy(msg_buffer, cur_config.msg_buffer_4);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  }
//  cur_config.buffer = buf;
  composeMFSKMessage();
  serializeConfig();
}

void setConfig(const char * key, const char * label)
//void setConfig(std::string key, std::string label)
{
  display_mode = DisplayMode::Setting;
  cur_setting_key = std::string(key);
  cur_setting_label = std::string(label);
  std::string val = settings[key].second;
//  char temp_str[41];
  char type = val[0];
  std::size_t pos;

  switch (type)
  {
    case 'U':
      cur_setting_uint = atoll(settings[key].second.substr(1).c_str());
      cur_setting_type = SettingType::Uint;
//      sprintf(temp_str, "%lu", cur_setting_uint);
      cur_setting_selected = 0;
      break;
    case 'I':
      cur_setting_int = atoll(settings[key].second.substr(1).c_str());
      cur_setting_type = SettingType::Int;
//      sprintf(temp_str, "%l", cur_setting_uint);
      cur_setting_selected = 0;
      break;
    case 'S':
      cur_setting_str = settings[key].second.substr(1);
      cur_setting_type = SettingType::Str;
//      sprintf(temp_str, "%s", cur_setting_str.c_str());
      //    cur_setting_selected = strlen(temp_str) - 1;
      cur_setting_selected = 0;
      pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
      if (pos != std::string::npos)
      {
        cur_setting_index = (uint8_t)pos;
      }
      break;
    case 'F':
      cur_setting_float = atof(settings[key].second.substr(1).c_str());
      cur_setting_type = SettingType::Float;
//      sprintf(temp_str, "%f", cur_setting_uint);
      cur_setting_selected = 0;
      break;
    case 'B':
      if(val[1] == '1')
      {
        cur_setting_bool = true;
      }
      else
      {
        cur_setting_bool = false;
      }
      cur_setting_type = SettingType::Bool;
      break;
  }
}

void resetConfig(const uint8_t x)
{
  disable_display_loop = true;
  
  // TODO: get confirmation
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_6x10_mr);
  u8g2.drawStr(64 - u8g2.getStrWidth("Reset Configuration?") / 2, 10, "Reset Configuration?");
  u8g2.drawStr(6, 29, "OK");
  u8g2.drawStr(80, 29, "Cancel");
  u8g2.sendBuffer();
  delay(500);
  while (digitalRead(BTN_DSP_1) == LOW || digitalRead(BTN_DSP_2) == LOW)
  {
  }
  delay(50);
  
  while (digitalRead(BTN_DSP_1) == HIGH && digitalRead(BTN_DSP_2) == HIGH)
  {
    delay(50);
  }
  if (digitalRead(BTN_DSP_2) == LOW)
  {
    delay(50);
    if (digitalRead(BTN_DSP_2) == LOW)
    {
      disable_display_loop = false;
      return;
    }
  }
  else if (digitalRead(BTN_DSP_1) == LOW)
  {
    delay(50);
    if (digitalRead(BTN_DSP_1) == LOW)
    {
      // Display wait message
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_6x10_mr);
      u8g2.drawStr(64 - u8g2.getStrWidth("Resetting.") / 2, 10, "Resetting.");
      u8g2.drawStr(64 - u8g2.getStrWidth("Please wait...") / 2, 25, "Please wait...");
      u8g2.sendBuffer();

      #ifdef EXT_EEPROM
      // Erase EEPROM
      eraseEEPROM();
      #endif
      
      // Load default values into config
      char temp_str[81];
      cur_config.valid = true;
      cur_config.version = CONFIG_SCHEMA_VERSION;
      cur_config.mode = DEFAULT_MODE;
      cur_config.band = DEFAULT_BAND_INDEX;
      cur_config.base_freq = DEFAULT_FREQUENCY;
      cur_config.wpm = DEFAULT_WPM;
      cur_config.tx_intv = DEFAULT_TX_INTERVAL;
      cur_config.dfcw_offset = DEFAULT_DFCW_OFFSET;
      cur_config.buffer = DEFAULT_CUR_BUFFER;
      strcpy(cur_config.callsign, DEFAULT_CALLSIGN);
      strcpy(cur_config.grid, DEFAULT_GRID);
      cur_config.power = DEFAULT_POWER;
      cur_config.pa_bias = DEFAULT_PA_BIAS;
      cur_config.cwid = DEFAULT_CWID;
      strcpy(cur_config.msg_buffer_1, DEFAULT_MSG_1);
      strcpy(cur_config.msg_buffer_2, DEFAULT_MSG_2);
      strcpy(cur_config.msg_buffer_3, DEFAULT_MSG_3);
      strcpy(cur_config.msg_buffer_4, DEFAULT_MSG_4);
      cur_config.si5351_int_corr = DEFAULT_SI5351_INT_CORR;
//      mode = cur_config.mode;
//      strcpy(msg_buffer_1, cur_config.msg_buffer_1);
//      strcpy(msg_buffer_2, cur_config.msg_buffer_2);
//      strcpy(msg_buffer_3, cur_config.msg_buffer_3);
//      strcpy(msg_buffer_4, cur_config.msg_buffer_4);
//      cur_buffer = cur_config.buffer;
      selectBuffer(cur_config.buffer);
      cur_tone_spacing = cur_config.dfcw_offset;
      sprintf(temp_str, "U%lu", cur_config.pa_bias);
      settings["pa_bias"].second = std::string(temp_str);
      sprintf(temp_str, "S%s", cur_config.callsign);
      settings["callsign"].second = std::string(temp_str);
      sprintf(temp_str, "S%s", cur_config.grid);
      settings["grid"].second = std::string(temp_str);
      sprintf(temp_str, "U%u", cur_config.power);
      settings["power"].second = std::string(temp_str);
      sprintf(temp_str, "U%lu", cur_config.tx_intv);
      settings["tx_intv"].second = std::string(temp_str);
      sprintf(temp_str, "U%lu", cur_config.wpm);
      settings["wpm"].second = std::string(temp_str);
      if(cur_config.cwid)
      {
        settings["cwid"].second = "B1";
      }
      else
      {
        settings["cwid"].second = "B0";
      }
    
      // Save config to EEPROM
      serializeConfig();

      select_band_reset = true;
    }
  }

  disable_display_loop = false;
}
