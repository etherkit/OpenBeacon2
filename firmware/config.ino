#define ARDUINOJSON_USE_LONG_LONG 1

void serializeConfig()
{
  //flash_config = flash_store.read();
  #ifdef EXT_EEPROM
  byte * byte_array = (byte*) &cur_config;
  for(uint16_t i = 0; i < sizeof(cur_config); i += EEP_24AA64T_BLOCK_SIZE)
  {
    uint8_t eep_status = eeprom.write(i, byte_array, EEP_24AA64T_BLOCK_SIZE); // EEPROM_CONFIG_SIZE
    byte_array += EEP_24AA64T_BLOCK_SIZE;
  }
//  byte write_byte = 11;
//  uint8_t eep_status = eeprom.write(0, &write_byte, 1);
//  if(eep_status != 0)
//  {
//    if(eep_status == EEPROM_ADDR_ERR)
//    {
//      // Tried to write past the end of address space
//      SerialUSB.write('\v');
//      SerialUSB.print("Tried to write past the end of EEPROM address space");
//    }
//    else
//    {
//      SerialUSB.write('\v');
//      SerialUSB.print("EEPROM write error");
//    }
//  }
  #else
  flash_config.valid = true;
  flash_config.version = cur_config.version;
  flash_config.mode = cur_config.mode;
  flash_config.band = cur_config.band;
  flash_config.base_freq = cur_config.base_freq;
  flash_config.wpm = cur_config.wpm;
  flash_config.tx_intv = cur_config.tx_intv;
  flash_config.dfcw_offset = cur_config.dfcw_offset;
  flash_config.buffer = cur_config.buffer;
  strcpy(flash_config.callsign, cur_config.callsign);
  strcpy(flash_config.grid, cur_config.grid);
  flash_config.power = cur_config.power;
  flash_config.pa_bias = cur_config.pa_bias;
  flash_config.cwid = cur_config.cwid;
  strcpy(flash_config.msg_buffer_1, cur_config.msg_buffer_1);
  strcpy(flash_config.msg_buffer_2, cur_config.msg_buffer_2);
  strcpy(flash_config.msg_buffer_3, cur_config.msg_buffer_3);
  strcpy(flash_config.msg_buffer_4, cur_config.msg_buffer_4);
  flash_config.si5351_int_corr = cur_config.si5351_int_corr;
  flash_config.rnd_tx = cur_config.rnd_tx;
  flash_store.write(flash_config);
  #endif
}

void deserializeConfig()
{
  char temp_str[81];

  // TODO: need to add version checking
  #ifdef EXT_EEPROM
  byte * byte_array = (byte*) &cur_config;
  for(uint16_t i = 0; i < sizeof(cur_config); ++i)
  {
    *byte_array++ = eeprom.read(i);
  }
//  byte read_byte = 0;
//  uint8_t eep_status = eeprom.read(0, &read_byte, 1);
//  if(eep_status != 0)
//  {
//    SerialUSB.write('\v');
//    SerialUSB.print("EEPROM read error");
//  }
//  else
//  {
//    SerialUSB.write('\v');
//    SerialUSB.print("EEPROM read: ");
//    SerialUSB.print(read_byte);
//  }
//  mode = cur_config.mode;
//  strcpy(msg_buffer_1, cur_config.msg_buffer_1);
//  strcpy(msg_buffer_2, cur_config.msg_buffer_2);
//  strcpy(msg_buffer_3, cur_config.msg_buffer_3);
//  strcpy(msg_buffer_4, cur_config.msg_buffer_4);
//  cur_buffer = cur_config.buffer;
  selectBuffer(cur_config.buffer);
  selectMode(static_cast<uint8_t>(cur_config.mode));
  cur_tone_spacing = cur_config.dfcw_offset;
  sprintf(temp_str, "U%lu", cur_config.pa_bias);
  settings["pa_bias"].second = std::string(temp_str);
  sprintf(temp_str, "S%s", cur_config.callsign);
  settings["callsign"].second = std::string(temp_str);
  sprintf(temp_str, "S%s", cur_config.grid);
  settings["grid"].second = std::string(temp_str);
  sprintf(temp_str, "I%u", cur_config.power);
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
  if(cur_config.rnd_tx)
  {
    settings["rnd_tx"].second = "B1";
  }
  else
  {
    settings["rnd_tx"].second = "B0";
  }
  #else
  flash_config = flash_store.read();
  if(flash_config.valid == true)
  {
    cur_config.mode = flash_config.mode;
    cur_config.band = flash_config.band;
    cur_config.base_freq = flash_config.base_freq;
    cur_config.wpm = flash_config.wpm;
    cur_config.tx_intv = flash_config.tx_intv;
    cur_config.dfcw_offset = flash_config.dfcw_offset;
    cur_config.buffer = flash_config.buffer;
    strcpy(cur_config.callsign, flash_config.callsign);
    strcpy(cur_config.grid, flash_config.grid);
    cur_config.power = flash_config.power;
    cur_config.pa_bias = flash_config.pa_bias;
    cur_config.cwid = flash_config.cwid;
    strcpy(cur_config.msg_buffer_1, flash_config.msg_buffer_1);
    strcpy(cur_config.msg_buffer_2, flash_config.msg_buffer_2);
    strcpy(cur_config.msg_buffer_3, flash_config.msg_buffer_3);
    strcpy(cur_config.msg_buffer_4, flash_config.msg_buffer_4);
    cur_config.si5351_int_corr = flash_config.si5351_int_corr;
//    mode = flash_config.mode;
    strcpy(msg_buffer_1, cur_config.msg_buffer_1);
    strcpy(msg_buffer_2, cur_config.msg_buffer_2);
    strcpy(msg_buffer_3, cur_config.msg_buffer_3);
    strcpy(msg_buffer_4, cur_config.msg_buffer_4);
//    cur_buffer = cur_config.buffer;
    selectBuffer(cur_config.buffer);
    cur_tone_spacing = cur_config.dfcw_offset;
    sprintf(temp_str, "U%lu", flash_config.pa_bias);
    settings["pa_bias"].second = std::string(temp_str);
    sprintf(temp_str, "S%s", flash_config.callsign);
    settings["callsign"].second = std::string(temp_str);
    sprintf(temp_str, "S%s", flash_config.grid);
    settings["grid"].second = std::string(temp_str);
    sprintf(temp_str, "U%u", flash_config.power);
    settings["power"].second = std::string(temp_str);
    sprintf(temp_str, "U%lu", flash_config.tx_intv);
    settings["tx_intv"].second = std::string(temp_str);
    sprintf(temp_str, "U%lu", flash_config.wpm);
    settings["wpm"].second = std::string(temp_str);
    if(flash_config.cwid)
    {
      settings["cwid"].second = "B1";
    }
    else
    {
      settings["cwid"].second = "B0";
    }
    if(cur_config.rnd_tx)
    {
      settings["rnd_tx"].second = "B1";
    }
    else
    {
      settings["rnd_tx"].second = "B0";
    }
  }
  else
  {
//    SerialUSB.print('\v');
//    SerialUSB.print("Deserialize failure");
  }
  #endif

  selectMode(static_cast<uint8_t>(cur_config.mode));
//  composeWSPRBuffer();
  setTxState(TxState::Idle);
  next_tx = UINT32_MAX;
  frequency = (cur_config.base_freq * 100ULL);
  change_freq = true;
}

#ifdef EXT_EEPROM
void eraseEEPROM() // Erase by writing 0xFF to every byte
{
  uint8_t data[EEP_24AA64T_BLOCK_SIZE];
  for(uint16_t i = 0; i < EEP_24AA64T_BLOCK_SIZE; ++i)
  {
    data[i] = 0xff;
  }
  for(uint16_t i = 0; i < EEP_24AA64T_CAPACITY; i += EEP_24AA64T_BLOCK_SIZE)
  {
    uint8_t eep_status = eeprom.write(i, data, EEP_24AA64T_BLOCK_SIZE);
  }
}
#endif
