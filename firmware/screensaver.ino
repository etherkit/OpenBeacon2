#define ARDUINOJSON_USE_LONG_LONG 1

void processScreenSaver()
{
  // Turn on screen saver if necessary
  if(cur_timer >= screen_saver_timeout && !screen_saver_enable)
  {
    cur_screen_saver_x = random(1, 16);
    cur_screen_saver_y = random(1, 16);
    screen_saver_enable = true;
  }

  // Update the TX progress percentage
  if(cur_state != TxState::Idle)
  {    
    switch(cur_config.mode)
    {
    case Mode::DFCW3:
    case Mode::DFCW6:
    case Mode::DFCW10:
    case Mode::DFCW120:
    case Mode::QRSS3:
    case Mode::QRSS6:
    case Mode::QRSS10:
    case Mode::QRSS120:
    case Mode::CW:
    case Mode::HELL:
      if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
      {
        tx_progress = (morse.cur_char * 100) / strlen(cur_config.callsign);
      }
      else
      {
//        tx_progress = (morse.cur_char * 100) / strlen(msg_buffer);
        tx_progress = (morse.cur_char * 100) / msg_buffer.length();
      }
      yield();
      break;
      
    case Mode::WSPR:
      if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
      {
        tx_progress = (morse.cur_char * 100) / strlen(cur_config.callsign);
      }
      else
      {
        tx_progress = (cur_symbol * 100) / WSPR_SYMBOL_COUNT;
      }
      yield();
      break;
      
    case Mode::JT65:
      if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
      {
        tx_progress = (morse.cur_char * 100) / strlen(cur_config.callsign);
      }
      else
      {
        tx_progress = (cur_symbol * 100) / JT65_SYMBOL_COUNT;
      }
      yield();
      break;
      
    case Mode::JT9:
      if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
      {
        tx_progress = (morse.cur_char * 100) / strlen(cur_config.callsign);
      }
      else
      {
        tx_progress = (cur_symbol * 100) / JT9_SYMBOL_COUNT;
      }
      yield();
      break;
      
    case Mode::JT4:
      if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
      {
        tx_progress = (morse.cur_char * 100) / strlen(cur_config.callsign);
      }
      else
      {
        tx_progress = (cur_symbol * 100) / JT4_SYMBOL_COUNT;
      }
      yield();
      break;
      
    case Mode::FT8:
      if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
      {
        tx_progress = (morse.cur_char * 100) / strlen(cur_config.callsign);
      }
      else
      {
        tx_progress = (cur_symbol * 100) / FT8_SYMBOL_COUNT;
      }
      yield();
      break;
    }
  }
  else
  {
    tx_progress = 0;
    yield();
  }
//  yield();

  // Screen saver animation
  if(screen_saver_enable)
  {
//    yield();
    if(cur_timer >= screen_saver_update)
    {
      if((cur_screen_saver_x >= (u8g2.getDisplayWidth() - u8g2.getStrWidth(screen_saver_msg) - 1)) && screen_saver_x_accel == 1)
      {
        screen_saver_x_accel = -1;
      }
      else if(cur_screen_saver_x == 0 && screen_saver_x_accel == -1)
      {
        screen_saver_x_accel = 1;
      }
      yield();

      // if((cur_screen_saver_y >= u8g2.getDisplayHeight() - u8g2.getMaxCharHeight() - 1) && screen_saver_y_accel == 1)
      if((cur_screen_saver_y >= (u8g2.getDisplayHeight() - 1)) && screen_saver_y_accel == 1)
      {
        screen_saver_y_accel = -1;
      }
      else if((cur_screen_saver_y <= u8g2.getMaxCharHeight() - 1) && screen_saver_y_accel == -1)
      {
        screen_saver_y_accel = 1;
      }
      yield();

      cur_screen_saver_x += screen_saver_x_accel;
      cur_screen_saver_y += screen_saver_y_accel;
      screen_saver_update = cur_timer + SCREEN_SAVER_TIMER_INTERVAL;
    }
  }
  yield();
}
