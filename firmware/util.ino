constexpr unsigned long power_10(unsigned long exponent)
{
  // bounds checking pls
  return (exponent == 0) ? 1 : 10 * power_10(exponent - 1);
}

uint8_t num_digits(uint64_t number)
{
  uint8_t result = 0;

  do
  {
    number /= 10;
    result++;
  }while(number > 0);

  return result;
}

// Voltage specified in millivolts
void setPABias(uint16_t voltage)
{
  uint32_t reg;
  uint8_t reg1, reg2;

  // Bounds checking
  if (voltage > MCP4725A1_VREF)
  {
    voltage = MCP4725A1_VREF;
  }

  // Convert millivolts to the correct register value
  reg = ((uint32_t)voltage * 4096UL) / MCP4725A1_VREF;
  reg1 = (uint8_t)((reg >> 8) & 0xFF);
  reg2 = (uint8_t)(reg & 0xFF);

  // Write the register to the MCP4725A1
  noInterrupts();
  Wire.beginTransmission(MCP4725A1_BUS_BASE_ADDR);
  Wire.write(reg1);
  Wire.write(reg2);
  Wire.endTransmission();
  interrupts();
}

void initMenu()
{
  menu.addChild("Mode");
  menu.selectChild(0);
  for (auto i : mode_table)
  {
    uint8_t index = static_cast<uint8_t>(i.index);
    menu.addChild(i.mode_name, selectMode, static_cast<uint8_t>(i.index));
  }
  menu.selectParent();
  menu.addChild("Buf Sel");
  menu.selectChild(1);
  menu.addChild("1", selectBuffer, 1);
  menu.addChild("2", selectBuffer, 2);
  menu.addChild("3", selectBuffer, 3);
  menu.addChild("4", selectBuffer, 4);
  menu.selectParent();
  menu.addChild("Buf Edit");
  menu.selectChild(2);
  menu.addChild("1", setBuffer, "1");
  menu.addChild("2", setBuffer, "2");
  menu.addChild("3", setBuffer, "3");
  menu.addChild("4", setBuffer, "4");
  menu.selectParent();
  menu.addChild("Settings");
  menu.selectChild(3);
  for (auto& c : settings_table)
  {
    menu.addChild(c[1].c_str(), setConfig, c[0].c_str());
  }
  menu.selectParent();
  menu.addChild("Reset", resetConfig, 0);
  menu.selectParent();
  menu.selectRoot();
}

bool isTimeValid()
{
  if (rtc.getEpoch() < time_sync_expire)
  {
    return true;
  }
  else
  {
    return false;
  }
}
