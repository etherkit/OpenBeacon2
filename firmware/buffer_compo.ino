void composeMorseBuffer(uint8_t buf)
{
  char temp_call[16];
  char temp_grid[6];

  sprintf(temp_call, "%s", cur_config.callsign);
  switch(buf)
  {
  case 1:
    sprintf(cur_config.msg_buffer_1, "%s", temp_call);
    break;
  case 2:
    sprintf(cur_config.msg_buffer_2, "%s", temp_call);
    break;
  case 3:
    sprintf(cur_config.msg_buffer_3, "%s", temp_call);
    break;
  case 4:
    sprintf(cur_config.msg_buffer_4, "%s", temp_call);
    break;
  }
      
  yield();
}

void composeWSPRBuffer()
{
  char temp_power[10];
  itoa(cur_config.power, temp_power, 10);
  wspr_buffer = std::string(cur_config.callsign) + " " + std::string(cur_config.grid) + " " + std::string(temp_power);
}

void composeJTBuffer(uint8_t buf)
{
  char temp_call[16];
  char temp_grid[6];

  sprintf(temp_call, "%s", settings["callsign"].second.substr(1).c_str());
  sprintf(temp_grid, "%s", settings["grid"].second.substr(1).c_str());
  switch(buf)
  {
  case 1:
    sprintf(cur_config.msg_buffer_1, "%s %s", temp_call, temp_grid);
    break;
  case 2:
    sprintf(cur_config.msg_buffer_2, "%s %s", temp_call, temp_grid);
    break;
  case 3:
    sprintf(cur_config.msg_buffer_3, "%s %s", temp_call, temp_grid);
    break;
  case 4:
    sprintf(cur_config.msg_buffer_4, "%s %s", temp_call, temp_grid);
    break;
  }
}

void composeMFSKMessage()
{
  char temp_msg_buffer[15];
  strncpy(temp_msg_buffer, msg_buffer.c_str(), 13);
  
  switch(cur_config.mode)
  {
  case Mode::WSPR:
    memset(mfsk_buffer, 0, 255);
    jtencode.wspr_encode(cur_config.callsign, cur_config.grid, cur_config.power, mfsk_buffer);
    break;
  case Mode::JT65:
    memset(mfsk_buffer, 0, 255);
    jtencode.jt65_encode(temp_msg_buffer, mfsk_buffer);
    break;
  case Mode::JT9:
    memset(mfsk_buffer, 0, 255);
    jtencode.jt9_encode(temp_msg_buffer, mfsk_buffer);
    break;
  case Mode::JT4:
    memset(mfsk_buffer, 0, 255);
    jtencode.jt4_encode(temp_msg_buffer, mfsk_buffer);
    break;
  case Mode::FT8:
    memset(mfsk_buffer, 0, 255);
    jtencode.ft8_encode(temp_msg_buffer, mfsk_buffer);
    break;
  }
}
