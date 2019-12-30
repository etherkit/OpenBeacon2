void drawOLED()
{
  static char temp_str[41];
  static char freq_str[10];
  static char freq_str_1[4];
  static char freq_str_2[4];
  static char freq_str_3[4];
  static char temp_chr[2];
  uint32_t freq;
  uint8_t zero_pad = 0;
  uint8_t underline;
  char menu_1[16];
  char menu_2[16];
  uint8_t menu_1_x, menu_2_x;

  if(!disable_display_loop)
  {
    // u8g2 draw loop
    // --------------
    u8g2.clearBuffer();          // clear the internal memory
    yield();

    if(!screen_saver_enable)
    {
      if(display_mode == DisplayMode::Main || display_mode == DisplayMode::Menu)
      {
        u8g2.setFont(u8g2_font_logisoso16_tn);
        // yield();
        u8g2.setDrawColor(1);
        //u8g2.setFont(u8g2_font_inb19_mn);
      
        // MHz
        yield(); // you need an odd number of yields or you get a strange bimodal distribution
                // of transmit times, with half being too long
        freq = cur_config.base_freq;
    
        if(cur_config.base_freq / 1000000UL > 0)
        {
          sprintf(temp_str, "%3lu", freq / 1000000UL);
          zero_pad = 1;
        }
        else
        {
          sprintf(temp_str, "   ");
          //u8g2.drawStr(0, 17, temp_str);
        }
//        yield();
        // We do this because the desired font isn't quite monospaced :-/
        for(uint8_t i = 0; i < 3; ++i)
        {
          //memmove(temp_chr, temp_str + i, 1);
          sprintf(temp_chr, "%c", temp_str[i]);
//          yield();
          u8g2.drawStr(i * 9, 17, temp_chr);
//          yield();
        }
        freq %= 1000000UL;
        
        // kHz
        yield();
        if(zero_pad == 1)
        {
          sprintf(temp_str, "%03lu", freq / 1000UL);
        }
        else if(freq / 1000UL > 0)
        {
          sprintf(temp_str, "%3lu", freq / 1000UL);
          zero_pad = 1;
        }
        else
        {
          sprintf(temp_str, "   ");
        }
//        yield();
        for(uint8_t i = 0; i < 3; ++i)
        {
          //memmove(temp_chr, temp_str + i, 1);
          sprintf(temp_chr, "%c", temp_str[i]);
//          yield();
          u8g2.drawStr(i * 9 + 29, 17, temp_chr);
//          yield();
        }
        freq %= 1000UL;
        
        // Hz
        yield();
        if(zero_pad == 1)
        {
          sprintf(temp_str, "%03lu", freq);
        }
        else
        {
          sprintf(temp_str, "%3lu", freq);
        }
//        yield();
        for(uint8_t i = 0; i < 3; ++i)
        {
          //memmove(temp_chr, temp_str + i, 1);
          sprintf(temp_chr, "%c", temp_str[i]);
          // yield();
          u8g2.drawStr(i * 9 + 58, 17, temp_chr);
//          yield();
        }
        
        // Indicate step size
        switch(tune_step)
        {
        case 5:
          underline = 29;
          break;
        case 4:
          underline = 38;
          break;
        case 3:
          underline = 47;
          break;
        case 2:
          underline = 59;
          break;
        case 1:
          underline = 68;
          break;
        case 0:
          underline = 77;
          break;
        }
    
        //yield();
      
        // Draw step size indicator
        u8g2.drawBox(underline, 18, 9, 2);
      
        // Draw Hz
        //u8g2.setFont(u8g2_font_5x8_mr);
      //  u8g2.drawStr(120, 20, "Hz");
      
        // Draw ADC
        //u8g2.setFont(u8g2_font_5x8_mr);
      //  sprintf(temp_str, "%u %u", band_id, band_index);
      //  u8g2.drawStr(90, 10, temp_str);
      
        // Draw clock
//        yield();
        //u8g2.setFont(u8g2_font_6x10_mr);
        u8g2.setFont(u8g2_font_5x7_tn);
        sprintf(temp_str, "%02u:%02u:%02u", rtc.getHours(),
          rtc.getMinutes(), rtc.getSeconds());
        u8g2.drawStr(88, 6, temp_str);
        yield();
        
      
        // Draw mode
        // yield();
        u8g2.setFont(u8g2_font_6x10_mr);
        sprintf(temp_str, "%s", mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name);
        u8g2.drawStr(87, 15, temp_str);
        yield();
      
        // Draw index
        //sprintf(temp_str, "%s", settings["Callsign"].c_str());
    //    sprintf(temp_str, "%s", cur_setting.c_str());
    //    u8g2.drawStr(87, 15, temp_str);
    
        // Draw key
    //    sprintf(temp_str, "%s", settings["PA Bias"].first.c_str());
    //    u8g2.drawStr(87, 15, temp_str);
    
        // Draw timer and next event
    //    sprintf(temp_str, "%lu", cur_timer);
    //    u8g2.drawStr(0, 30, temp_str);
    //    sprintf(temp_str, "%lu", next_event);
    //    u8g2.drawStr(50, 30, temp_str);
    //    sprintf(temp_str, "%i", RTC->MODE2.FREQCORR.reg);
    //    u8g2.drawStr(0, 30, temp_str);
  //      sprintf(temp_str, "%u", mfsk_buffer[cur_symbol]);
  //      u8g2.drawStr(100, 30, temp_str);
    
    // DEBUGGING HERE
//         sprintf(temp_str, "%lu", cur_timer);
//         u8g2.drawStr(0, 8, temp_str);
  //      sprintf(temp_str, "%u", cur_symbol);
  //      u8g2.drawStr(111, 15, temp_str);
    //    sprintf(temp_str, "%u", morse.tx);
    //    u8g2.drawStr(122, 15, temp_str);
        
    //      for(uint8_t i = 0; i < 18; ++i)
    //      {
    //        sprintf(temp_str, "%u", mfsk_buffer[i]);
    //        u8g2.drawStr(i * 6, 30, temp_str);
    //      }
  //      sprintf(temp_str, "%lu", rtc.getEpoch());
  //      u8g2.drawStr(39, 30, temp_str);
    //    sprintf(temp_str, "%lu", next_time_sync);
    //    u8g2.drawStr(66, 30, temp_str);
  //      sprintf(temp_str, "%lu", time_sync_expire);
  //      u8g2.drawStr(39, 30, temp_str);
  //      sprintf(temp_str, "%ld", time_sync_expire - rtc.getEpoch());
  //      u8g2.drawStr(39, 30, temp_str);
    
    
        // Draw band ID
  //      sprintf(temp_str, "%lu", band_id_1);
  //      u8g2.drawStr(50, 29, temp_str);
  //      sprintf(temp_str, "%lu", band_id_2);
  //      u8g2.drawStr(88, 29, temp_str);

  //      sprintf(temp_str, "%u", cur_band_module);
  //      u8g2.drawStr(88, 29, temp_str);


        // Draw callsign
        //sprintf(temp_str, "%s", settings["Callsign"].substr(1).c_str());
        //sprintf(temp_str, "%s", settings["Grid"].substr(1).c_str());
    //    sprintf(temp_str, "%s", settings["Power"].substr(1).c_str());
    //    u8g2.drawStr(26, 30, temp_str);
        
      
        // Draw TX lock
    //    if(tx_lock)
    //    {
    //      u8g2.drawXBM(120, 8, 8, 8, lock_bits);
    //    }
      }
      else if (display_mode == DisplayMode::Setting) // Draw settings menu
      {
        // Draw setting name
        u8g2.setFont(u8g2_font_6x10_mr);
        sprintf(temp_str, "%s", cur_setting_label.c_str());
        u8g2.drawStr(64 - u8g2.getStrWidth(temp_str) / 2, 10, temp_str);
    
        //const char* setting_val = cur_setting_str.c_str();
    
    //    switch(cur_setting_type)
    //    {
    //    case SettingType::Uint:
    //      sprintf(temp_str, "%lu", cur_setting_uint);
    //      break;
    //    case SettingType::Int:
    //      sprintf(temp_str, "%l", cur_setting_int);
    //      break;
    //    case SettingType::Str:
    ////      if(cur_setting_selected > 10)
    ////      {
    ////        setting_val += cur_setting_selected - 10;
    ////      }
    //      //sprintf(temp_str, "%s", cur_setting_str.c_str());
    //      sprintf(temp_str, "%s", settings[cur_setting].c_str());
    ////      SerialUSB.print("\v");
    ////      SerialUSB.println(temp_str);
    //      //sprintf(temp_str, "%s", setting_val);
    //      break;
    //    }
    
        uint8_t str_x;
    
        switch(settings[cur_setting_key].second[0])
        {
        case 'U':
          sprintf(temp_str, "%lu", cur_setting_uint);
          str_x = 61 - (num_digits(cur_setting_uint) * SETTING_FONT_WIDTH) + ((cur_setting_selected + 1) * SETTING_FONT_WIDTH);
          break;
        case 'I':
          sprintf(temp_str, "%i", cur_setting_int);
          str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
          break;
        case 'B':
          if(cur_setting_bool)
          {
            sprintf(temp_str, "On");
            str_x = 61;
          }
          else
          {
            sprintf(temp_str, "Off");
            str_x = 61;
          }
          break;
        case 'S':
    //      if(cur_setting_selected > 10)
    //      {
    //        setting_val += cur_setting_selected - 10;
    //      }
          sprintf(temp_str, "%s<", cur_setting_str.c_str());
          str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
          //sprintf(temp_str, "%s", settings[cur_setting].c_str());
    //      SerialUSB.print("\v");
    //      SerialUSB.println(temp_str);
          //sprintf(temp_str, "%s", setting_val);
          break;
        }
    
        cur_setting_len = strlen(temp_str) - 1;
    
        // Put active digit/char at X pos 61
    //    uint8_t str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
    //    uint8_t str_x = 61 - ((cur_setting_selected - 1 - cur_setting_len) * SETTING_FONT_WIDTH);
        
        //uint8_t str_x = (cur_setting_selected * SETTING_FONT_WIDTH > 60 ? 0 : 60 - cur_setting_selected * SETTING_FONT_WIDTH);
        //u8g2.setDrawColor(0);
        u8g2.drawStr(str_x, 20, temp_str);
        //u8g2.setDrawColor(1);
    
        cur_setting_char = cur_setting_str[cur_setting_selected];
    
        // Find char in allowable list
    //    std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
    //    if(pos != std::string::npos)
    //    {
    //      cur_setting_index = pos;
    //    }
    
        // debugging stuff
  //      sprintf(temp_str, "%d", cur_setting_selected);
  //      u8g2.drawStr(0, 10, temp_str);
    //    sprintf(temp_str, "%c", cur_setting_char);
    //    u8g2.drawStr(20, 10, temp_str);
    //    sprintf(temp_str, "%d", cur_setting_index);
    //    u8g2.drawStr(90, 10, temp_str);
    //    sprintf(temp_str, "%d", cur_setting_type);
    //    u8g2.drawStr(110, 10, temp_str);
        
    
        // Underline the current setting selection
        u8g2.drawLine(60, 21, 66, 21);
      }
      else // Draw buffer menu
      {
        uint8_t str_x;
    
        // Draw buffer name
        u8g2.setFont(u8g2_font_6x10_mr);
        sprintf(temp_str, "Buffer %d", cur_edit_buffer);
        u8g2.drawStr(64 - u8g2.getStrWidth(temp_str) / 2, 10, temp_str);
        
        sprintf(temp_str, "%s<", cur_setting_str.c_str());
        str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
        cur_setting_len = strlen(temp_str) - 1;
        u8g2.drawStr(str_x, 20, temp_str);
    
        // Underline the current setting selection
        u8g2.drawLine(60, 21, 66, 21);
    
        // debugging stuff
    //    sprintf(temp_str, "%d", cur_edit_buffer);
    //    u8g2.drawStr(0, 10, temp_str);
        
      }
    
      //yield();
    
      // Draw buffer or menu items
      
      // Arrows
      constexpr uint8_t triangle_top = 21;
      constexpr uint8_t triangle_bottom = 30;
      constexpr uint8_t triangle_center = 26;
      constexpr uint8_t left_triangle_left = 0;
      constexpr uint8_t left_triangle_right = 4;
      constexpr uint8_t right_triangle_left = 110;
      constexpr uint8_t right_triangle_right = 115;
        
      if(display_mode == DisplayMode::Menu)
      {
        // Menu items
//        std::string temp_menu_1 = menu.getActiveChildLabel();
//        std::string temp_menu_2 = menu.getActiveChildLabel(1);
        char temp_menu_1[21];
        char temp_menu_2[21];
//        temp_menu_1 = menu.getActiveChildLabel();
//        temp_menu_2 = menu.getActiveChildLabel(1);
//        sprintf(menu_1, "%s", temp_menu_1.c_str());
//        sprintf(menu_2, "%s", temp_menu_2.c_str());
        snprintf(temp_menu_1, 20, "%s", menu.getActiveChildLabel());
        snprintf(temp_menu_2, 20, "%s", menu.getActiveChildLabel(1));
        menu_1_x = 6;
        menu_2_x = 58;
        u8g2.setFont(u8g2_font_6x10_mf);
        //u8g2.setDrawColor(0);
        u8g2.drawStr(menu_1_x, 30, temp_menu_1);
        u8g2.drawStr(menu_2_x, 30, temp_menu_2);
        char json[80];
//        sprintf(json, "{\"level\":0,\"text\":\"Menu 1\",\"data\":\"%s\"}", temp_menu_1);
//        sendSerialPacket(0xFE, json);
//        sprintf(json, "{\"level\":0,\"text\":\"Menu 2\",\"data\":\"%s\"}", temp_menu_2);
//        sendSerialPacket(0xFE, json);
        
        if(menu.countChildren() > 2)
        {
          u8g2.drawTriangle(left_triangle_right, triangle_top,
            left_triangle_right, triangle_bottom, 
            left_triangle_left, triangle_center);
          u8g2.drawTriangle(right_triangle_left, triangle_top,
            right_triangle_left, triangle_bottom, 
            right_triangle_right, triangle_center);
        }
        
        // Back icon
        u8g2.setFont(u8g2_font_m2icon_9_tf);
        u8g2.drawGlyph(118, 31, 0x0061);
      }
      else if(display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
      {
        if(ins_del_mode)
        {
          sprintf(menu_1, "%s", "Ins");
          sprintf(menu_2, "%s", "Del");
        }
        else
        {
          if (cur_setting_type == SettingType::Str)
          {
            sprintf(menu_1, "%s", "Ins/Del");
          }
          else
          {
            sprintf(menu_1, "%s", "");
          }
          sprintf(menu_2, "%s", "OK");
        }
        menu_1_x = 6;
        menu_2_x = 80;
        u8g2.setFont(u8g2_font_6x10_mf);
        u8g2.drawStr(menu_1_x, 29, menu_1);
        u8g2.drawStr(menu_2_x, 29, menu_2);
    
        // Back icon
        u8g2.setFont(u8g2_font_m2icon_9_tf);
        u8g2.drawGlyph(118, 30, 0x0061);
      }
      else // Show the current buffer
      {
        // Draw buffer contents if transmitting
        if(cur_state != TxState::Idle)
        {
//          char buffer_str[41];
//          memset(buffer_str, 0, 41);
          //std::string wspr_buffer;
  //        yield();
          uint8_t tx_progress_line;
          
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
              snprintf(temp_str, 40, "CWID:%s<", cur_config.callsign);
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 29;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            else
            {
              snprintf(temp_str, 40, "%d:%s<", cur_config.buffer, msg_buffer.c_str());
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 11;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            break;
            
          case Mode::WSPR:
            if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
            {
              snprintf(temp_str, 40, "CWID:%s<", cur_config.callsign);
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 29;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            else
            {
              snprintf(temp_str, 40, "%s<", wspr_buffer.c_str());
//              sprintf(buffer_str, "%s<", "a test");
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Draw TX progress bar
              yield();
              uint8_t tx_progress_line = (cur_symbol * 128 / WSPR_SYMBOL_COUNT);
              u8g2.drawLine(0, 31, tx_progress_line, 31);
            }
            break;
            
          case Mode::JT65:
            if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
            {
              snprintf(temp_str, 40, "CWID:%s<", cur_config.callsign);
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 29;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            else
            {
              snprintf(temp_str, 40, "%d:%s<", cur_config.buffer, msg_buffer.c_str());
//              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Draw TX progress bar
//              yield();
              tx_progress_line = (cur_symbol * 128 / JT65_SYMBOL_COUNT);
              u8g2.drawLine(0, 31, tx_progress_line, 31);
            }
            break;
            
          case Mode::JT9:
            if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
            {
              snprintf(temp_str, 40, "CWID:%s<", cur_config.callsign);
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 29;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            else
            {
              snprintf(temp_str, 40, "%d:%s<", cur_config.buffer, msg_buffer.c_str());
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Draw TX progress bar
              yield();
              uint8_t tx_progress_line = (cur_symbol * 128 / JT9_SYMBOL_COUNT);
              u8g2.drawLine(0, 31, tx_progress_line, 31);
            }
            break;
            
          case Mode::JT4:
            if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
            {
              snprintf(temp_str, 40, "CWID:%s<", cur_config.callsign);
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 29;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            else
            {
              snprintf(temp_str, 40, "%d:%s<", cur_config.buffer, msg_buffer.c_str());
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Draw TX progress bar
              yield();
              uint8_t tx_progress_line = (cur_symbol * 128 / JT4_SYMBOL_COUNT);
              u8g2.drawLine(0, 31, tx_progress_line, 31);
            }
            break;
            
          case Mode::FT8:
            if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
            {
              snprintf(temp_str, 40, "CWID:%s<", cur_config.callsign);
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Underline current TX char
              uint8_t line_x = (morse.cur_char * 6) + 29;
              u8g2.drawLine(line_x, 31, line_x + 6, 31);
            }
            else
            {
              snprintf(temp_str, 40, "%d:%s<", cur_config.buffer, msg_buffer.c_str());
              yield();
              u8g2.drawStr(0, 30, temp_str);

              // Draw TX progress bar
              yield();
              uint8_t tx_progress_line = (cur_symbol * 128 / FT8_SYMBOL_COUNT);
              u8g2.drawLine(0, 31, tx_progress_line, 31);
              yield();
            }
            break;
          }
    
  //        yield();
  //        u8g2.drawStr(0, 30, buffer_str);
        }
        else // otherwise show TX enb/dis and current band module choice
        {
          if(tx_enable)
          {
            u8g2.setDrawColor(0);
            u8g2.drawStr(0, 29, "TX Dis");
            u8g2.setDrawColor(1); 
            sprintf(temp_str, "%1u: %s", cur_config.buffer, next_tx_time);
            u8g2.drawStr(45, 29, temp_str);
          }
          else
          {
            u8g2.setDrawColor(0);
            u8g2.drawStr(0, 29, "TX Enb");
            u8g2.setDrawColor(1);

            u8g2.setDrawColor(0);
            sprintf(temp_str, "%s", band_name.c_str());
            u8g2.drawStr(60, 29, temp_str);
            u8g2.setDrawColor(1);

  //          if(cur_band_module == 0)
  //          {
  //            u8g2.setDrawColor(0);
  //            u8g2.drawStr(60, 29, "Mod 1");
  //            u8g2.setDrawColor(1);
  //          }
  //          else if(cur_band_module == 1)
  //          {
  //            u8g2.setDrawColor(0);
  //            u8g2.drawStr(60, 29, "Mod 2");
  //            u8g2.setDrawColor(1);
  //          }
          }
        }
        yield();
        
        // Menu icon
        //yield();
        if(cur_state == TxState::Idle)
        {
          u8g2.setFont(u8g2_font_m2icon_9_tf);
          u8g2.drawGlyph(121, 30, 0x0042);
        }
        else
        {
          u8g2.setFont(u8g2_font_m2icon_9_tf);
          u8g2.drawGlyph(121, 30, 0x0043);
        }
      }
      //yield();
    //  sprintf(menu_1, "%s", "DFCW");
    //  sprintf(menu_2, "%s", "TX Enb");
    //  menu_1_x = 32 - ((u8g2.getStrWidth(menu_1) / 2) > 32 ? 32 : u8g2.getStrWidth(menu_1) / 2);
    //  menu_2_x = 96 - ((u8g2.getStrWidth(menu_2) / 2) > 32 ? 32 : u8g2.getStrWidth(menu_2) / 2);
    //  
    //  u8g2.setFont(u8g2_font_6x10_mr);
    //  //u8g2.setDrawColor(0);
    //  u8g2.drawStr(menu_1_x, 31, menu_1);
    //  u8g2.drawStr(menu_2_x, 31, menu_2);

    }
    else // screen saver enabled
    {
      if(tx_lock)
      {
        u8g2.setFont(u8g2_font_6x10_mr);
//        char screen_saver_msg[40];
//        sprintf(screen_saver_msg, "TX: %s %s %d%%", band_name.c_str(),
//          mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name, tx_progress);
        sprintf(screen_saver_msg, "TX: %s %s %d%%", band_name.c_str(),
          mode_name.c_str(), tx_progress);
        yield();
        u8g2.drawStr(cur_screen_saver_x, cur_screen_saver_y, screen_saver_msg);
      }
      else
      {
        u8g2.setFont(u8g2_font_prospero_bold_nbp_tr);
        yield();
        sprintf(screen_saver_msg,"%s", SCREEN_SAVER_MESSAGE);
        u8g2.drawStr(cur_screen_saver_x, cur_screen_saver_y, screen_saver_msg);
      }
    }
    yield();
    u8g2.sendBuffer();          // transfer internal memory to the display
  }
  yield();
}
