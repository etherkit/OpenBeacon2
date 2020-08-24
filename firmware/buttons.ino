#define ARDUINOJSON_USE_LONG_LONG 1

void pollButtons()
{
  // Read buttons
  // ------------
  yield();
  if(!disable_display_loop)
  {
    // Handle up button
    if (digitalRead(BTN_UP) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_UP) == LOW)
      {
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        // Disable screen saver
        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
  
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
              if (cur_setting_uint + power_10(cur_setting_selected) < UINT64_MAX)
              {
                cur_setting_uint += power_10(cur_setting_selected);
              }
              break;
            case SettingType::Int:
              ++cur_setting_int;
              break;
            case SettingType::Bool:
              cur_setting_bool = !cur_setting_bool;
              break;
            case SettingType::Str:
              if (cur_setting_index >= settings_str_chars.size() - 1)
              {
                cur_setting_index = 0;
              }
              else
              {
                cur_setting_index++;
              }
  
              cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
              break;
          }
        }
        else
        {
          //if(cur_state == TxState::Idle)
          if (!tx_lock)
          {
            if (cur_config.base_freq + power_10(tune_step) > upper_freq_limit)
            {
              cur_config.base_freq = upper_freq_limit;
            }
            else
            {
              cur_config.base_freq += power_10(tune_step);
            }
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one;
      }
    }
  
    // Handle down button
    if (digitalRead(BTN_DOWN) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_DOWN) == LOW)
      {
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        // Disable screen saver
        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
  
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
              if(cur_setting_uint >0)
              {
                if ((cur_setting_uint - power_10(cur_setting_selected)) >= 0)
                {
                  cur_setting_uint -= power_10(cur_setting_selected);
                }
                if (num_digits(cur_setting_uint) - 1 < cur_setting_selected)
                {
                  --cur_setting_selected;
                }
              }
              break;
            case SettingType::Int:
              --cur_setting_int;
              break;
            case SettingType::Bool:
              cur_setting_bool = !cur_setting_bool;
              break;
            case SettingType::Str:
              if (cur_setting_index == 0)
              {
                cur_setting_index = settings_str_chars.size() - 1;
              }
              else
              {
                cur_setting_index--;
              }
              cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
              break;
          }
        }
        else
        {
          //if(cur_state == TxState::Idle)
          if (!tx_lock)
          {
            if (cur_config.base_freq - power_10(tune_step) < lower_freq_limit)
            {
              cur_config.base_freq = lower_freq_limit;
            }
            else
            {
              cur_config.base_freq -= power_10(tune_step);
            }
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle left button
    if (digitalRead(BTN_LEFT) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_LEFT) == LOW)
      {
        // Disable screen saver
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
          if (menu.countChildren() > 2)
          {
            if (menu.getActiveChild() == 0 && menu.countChildren() % 2 == 1)
            {
              menu--;
            }
            else
            {
              menu--;
              menu--;
            }
          }
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
            case SettingType::Int:
              if (cur_setting_selected < num_digits(cur_setting_uint) - 1)
              {
                ++cur_setting_selected;
              }
              break;
            case SettingType::Str:
              if (cur_setting_selected > 0)
              {
                --cur_setting_selected;
  
                // Find char in allowable list
                std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
                if (pos != std::string::npos)
                {
                  cur_setting_index = (uint8_t)pos;
                  //cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
                }
              }
              break;
          }
        }
        else
        {
          if (tune_step < 5)
          {
            tune_step++;
          }
          else
          {
            tune_step = 0;
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle right button
    if (digitalRead(BTN_RIGHT) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_RIGHT) == LOW)
      {
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        // Disable screen saver
        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
          if (menu.countChildren() > 2)
          {
            if (menu.getActiveChild() == menu.countChildren() - 1)
            {
              menu++;
            }
            else
            {
              menu++;
              menu++;
            }
          }
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
            case SettingType::Int:
              if (cur_setting_selected > 0)
              {
                --cur_setting_selected;
              }
              break;
            case SettingType::Str:
              if (cur_setting_selected < cur_setting_len - 1)
              {
                ++cur_setting_selected;
                // Find char in allowable list
                std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
                if (pos != std::string::npos)
                {
                  cur_setting_index = (uint8_t)pos;
                  //cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
                }
              }
              break;
              //        case SettingType::Int:
              //          --cur_setting_int;
              //          break;
              //        case SettingType::Str:
              //          if(cur_setting_selected < cur_setting_str.size())
              //          {
              //            cur_setting_selected++;
              //          }
              //          break;
          }
        }
        else
        {
          if (tune_step == 0)
          {
            tune_step = 5;
          }
          else
          {
            tune_step--;
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle display 1 button
    if (digitalRead(BTN_DSP_1) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_DSP_1) == LOW)
      {
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        // Disable screen saver
        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
          MenuType type = menu.selectChild(menu.getActiveChild());
          if (type == MenuType::Action)
          {
            display_mode = DisplayMode::Main;
            menu.selectRoot();
          }
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          // INS/DEL button
          if (ins_del_mode)
          {
            // Insert
            cur_setting_str.insert(static_cast<std::size_t>(++cur_setting_selected), " ");
            cur_setting_index = settings_str_chars.length() - 1;
          }
          else
          {
            ins_del_mode = true;
          }
          //        switch(cur_setting_type)
          //        {
          //        case SettingType::Str:
          //          // Loop here to see if a long or short press
          //          // Short press == INS, long press == DEL
          //          uint32_t btn_timer_end = cur_timer + 1000;
          //          bool short_press = false;
          //
          //          while(cur_timer < btn_timer_end)
          //          {
          //            if(digitalRead(BTN_DSP_1) == HIGH)
          //            {
          //              // Insert a space
          ////              cur_setting_str.insert(static_cast<std::size_t>(++cur_setting_selected), " ");
          ////              cur_setting_index = settings_str_chars.length() - 1;
          //              short_press = true;
          //              break;
          //            }
          //            yield();
          //          }
          //          // If we made it this far, the button was held down for 1 s,
          //          // so do an DEL
          ////          cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--));
          //          if(short_press)
          //          {
          //            cur_setting_str.insert(static_cast<std::size_t>(++cur_setting_selected), " ");
          //            cur_setting_index = settings_str_chars.length() - 1;
          //          }
          //          else
          //          {
          //            cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--));
          //            while(digitalRead(BTN_DSP_1) == LOW)
          //            {
          //              yield();
          //            }
          //          }
          //          break;
          //        }
        }
        else
        {
          if (tx_enable)
          {
            tx_enable = false;
            next_tx = UINT32_MAX;
          }
          else
          {
            tx_enable = true;
            setTxState(TxState::Idle);
            // Re-compose the buffers to reflect changes
            selectBuffer(cur_config.buffer);
            composeWSPRBuffer();
//            composeMFSKMessage();
            setNextTx(0);
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle display 2 button
    if (digitalRead(BTN_DSP_2) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_DSP_2) == LOW)
      {
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        // Disable screen saver
        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
          MenuType type = menu.selectChild(menu.getActiveChild() + 1);
          if (type == MenuType::Action)
          {
            selectBuffer(cur_config.buffer);
//            composeMFSKMessage();  // logic so executed only on Sel Buf?
            display_mode = DisplayMode::Main;
            menu.selectRoot();
          }
        }
        else if (display_mode == DisplayMode::Setting)
        {
          // OK button
          char temp_str[81];
          switch (cur_setting_type)
          {
            case SettingType::Uint:
              sprintf(temp_str, "U%lu", cur_setting_uint);
              settings[cur_setting_key].second = temp_str;
              break;
            case SettingType::Int:
              sprintf(temp_str, "I%l", cur_setting_int);
              settings[cur_setting_key].second = temp_str;
              break;
            case SettingType::Bool:
              if(cur_setting_bool)
              {
                settings[cur_setting_key].second = "B1";
              }
              else
              {
                settings[cur_setting_key].second = "B0";
              }
              break;
            case SettingType::Str:
              if (ins_del_mode)
              {
                // Delete
                cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--), 1);
              }
              else
              {
                sprintf(temp_str, "S%s", cur_setting_str.c_str());
                settings[cur_setting_key].second = temp_str;
              }
  
              break;
          }
  
          // Yeah, inelegant. Hopefully we can get C++17 in Arduino soon for better data structures
          if (cur_setting_key == "pa_bias")
          {
            cur_config.pa_bias = cur_setting_uint;
            setPABias(cur_setting_uint);
          }
          else if (cur_setting_key == "callsign")
          {
            strcpy(cur_config.callsign, cur_setting_str.c_str());
            composeWSPRBuffer();
            composeMFSKMessage();
            // TODO: 
          }
          else if (cur_setting_key == "grid")
          {
            strcpy(cur_config.grid, cur_setting_str.c_str());
            composeWSPRBuffer();
            composeMFSKMessage();
          }
          else if (cur_setting_key == "power")
          {
            cur_config.power = cur_setting_int;
            composeWSPRBuffer();
            composeMFSKMessage();
          }
          else if (cur_setting_key == "tx_intv")
          {
            cur_config.tx_intv = cur_setting_uint;
          }
          else if (cur_setting_key == "wpm")
          {
            cur_config.wpm = cur_setting_uint;
          }
          else if (cur_setting_key == "cwid")
          {
            cur_config.cwid = cur_setting_bool;
          }
          else if (cur_setting_key == "rnd_tx")
          {
            cur_config.rnd_tx = cur_setting_bool;
          }
  
          // If we need to make any immediate setting changes to hardware
  //        if (cur_setting_key == "pa_bias")
  //        {
  //          setPABias(cur_setting_uint);
  //        }
          
  
          // Re-compose the buffers to reflect changes
//          composeWSPRBuffer();
//          composeJTBuffer(1);
//          composeMorseBuffer(1);
          selectBuffer(cur_config.buffer);
//          composeMFSKMessage();
//          selectMode(static_cast<uint8_t>(mode));
  
          // Save the config to NVM
          serializeConfig();
  
          if (!ins_del_mode)
          {
            cur_setting_selected = 0;
            display_mode = DisplayMode::Main;
            menu.selectRoot();
          }
        }
        else if (display_mode == DisplayMode::Buffer)
        {
          if (ins_del_mode)
          {
            // Delete
            cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--), 1);
          }
          else
          {
            switch (cur_edit_buffer)
            {
              case 1:
                sprintf(cur_config.msg_buffer_1, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_1, msg_buffer_1);
                break;
              case 2:
                sprintf(cur_config.msg_buffer_2, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_2, msg_buffer_2);
                break;
              case 3:
                sprintf(cur_config.msg_buffer_3, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_3, msg_buffer_3);
                break;
              case 4:
                sprintf(cur_config.msg_buffer_4, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_4, msg_buffer_4);
                break;
            }
  
            cur_setting_selected = 0;
            display_mode = DisplayMode::Main;
            menu.selectRoot();
            selectBuffer(cur_config.buffer);
            serializeConfig();
//            composeMFSKMessage();
//            selectMode(static_cast<uint8_t>(mode));
          }
        }
        else
        {
          if (!tx_enable)
          {
            uint32_t new_freq, new_qrss_freq, new_wspr_freq, new_jt65_freq, new_jt9_freq;
            uint32_t new_lower_freq_limit, new_upper_freq_limit;
            std::string new_band_name;
            bool retune = false;
  
            // Switch bands

            // First, check if there is another higher band in this band module
            if(band_table[cur_config.band].module_index == band_table[cur_config.band + 1].module_index)
            {
              ++cur_config.band;
              new_lower_freq_limit = band_table[cur_config.band].lower_limit;
              new_upper_freq_limit = band_table[cur_config.band].upper_limit;
              new_qrss_freq = band_table[cur_config.band].qrss_freq;
              new_wspr_freq = band_table[cur_config.band].wspr_freq;
              new_jt65_freq = band_table[cur_config.band].jt65_freq;
              new_jt9_freq = band_table[cur_config.band].jt9_freq;
              new_band_name = band_table[cur_config.band].name;
              retune = true;
            }
            // Then, check if there is another band module to switch to
            else
            {
              if(cur_band_module == 0)
              {
                if(band_id_2 > 150)
                {
                  cur_band_module = 1;
                  digitalWrite(BAND_SW, HIGH);

                  // Set band to first in band module list
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_2)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
//                      new_lower_freq_limit = band.lower_limit;
//                      new_upper_freq_limit = band.upper_limit;
//                      new_qrss_freq = band.cw_freq;
//                      new_wspr_freq = band.wspr_freq;
//                      new_jt65_freq = band.jt65_freq;
//                      new_jt9_freq = band.jt9_freq;
//                      new_band_name = band.name;
//                      retune = true;
                      new_lower_freq_limit = band_table[cur_config.band].lower_limit;
                      new_upper_freq_limit = band_table[cur_config.band].upper_limit;
                      new_qrss_freq = band_table[cur_config.band].qrss_freq;
                      new_wspr_freq = band_table[cur_config.band].wspr_freq;
                      new_jt65_freq = band_table[cur_config.band].jt65_freq;
                      new_jt9_freq = band_table[cur_config.band].jt9_freq;
                      new_band_name = std::string(band_table[cur_config.band].name);
                      retune = true;
                      break;
                    }
                  }
                }
                else
                {
                  // If not, go back to the first band on this module
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_1)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
//                      new_lower_freq_limit = band.lower_limit;
//                      new_upper_freq_limit = band.upper_limit;
//                      new_qrss_freq = band.cw_freq;
//                      new_wspr_freq = band.wspr_freq;
//                      new_jt65_freq = band.jt65_freq;
//                      new_jt9_freq = band.jt9_freq;
//                      new_band_name = band.name;
//                      retune = true;
                      new_lower_freq_limit = band_table[cur_config.band].lower_limit;
                      new_upper_freq_limit = band_table[cur_config.band].upper_limit;
                      new_qrss_freq = band_table[cur_config.band].qrss_freq;
                      new_wspr_freq = band_table[cur_config.band].wspr_freq;
                      new_jt65_freq = band_table[cur_config.band].jt65_freq;
                      new_jt9_freq = band_table[cur_config.band].jt9_freq;
                      new_band_name = std::string(band_table[cur_config.band].name);
                      retune = true;
                      break;
                    }
                  }
                }
              }
              else
              {
                if(band_id_1 > 150)
                {
                  cur_band_module = 0;
                  digitalWrite(BAND_SW, LOW);

                  // Set band to first in band module list
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_1)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
//                      new_lower_freq_limit = band.lower_limit;
//                      new_upper_freq_limit = band.upper_limit;
//                      new_qrss_freq = band.cw_freq;
//                      new_wspr_freq = band.wspr_freq;
//                      new_jt65_freq = band.jt65_freq;
//                      new_jt9_freq = band.jt9_freq;
//                      new_band_name = band.name;
//                      retune = true;
                      new_lower_freq_limit = band_table[cur_config.band].lower_limit;
                      new_upper_freq_limit = band_table[cur_config.band].upper_limit;
                      new_qrss_freq = band_table[cur_config.band].qrss_freq;
                      new_wspr_freq = band_table[cur_config.band].wspr_freq;
                      new_jt65_freq = band_table[cur_config.band].jt65_freq;
                      new_jt9_freq = band_table[cur_config.band].jt9_freq;
                      new_band_name = std::string(band_table[cur_config.band].name);
                      retune = true;
                      break;
                    }
                  }
                }
                else
                {
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_2)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
//                      new_lower_freq_limit = band.lower_limit;
//                      new_upper_freq_limit = band.upper_limit;
//                      new_qrss_freq = band.cw_freq;
//                      new_wspr_freq = band.wspr_freq;
//                      new_jt65_freq = band.jt65_freq;
//                      new_jt9_freq = band.jt9_freq;
//                      new_band_name = band.name;
//                      retune = true;
                      new_lower_freq_limit = band_table[cur_config.band].lower_limit;
                      new_upper_freq_limit = band_table[cur_config.band].upper_limit;
                      new_qrss_freq = band_table[cur_config.band].qrss_freq;
                      new_wspr_freq = band_table[cur_config.band].wspr_freq;
                      new_jt65_freq = band_table[cur_config.band].jt65_freq;
                      new_jt9_freq = band_table[cur_config.band].jt9_freq;
                      new_band_name = std::string(band_table[cur_config.band].name);
                      retune = true;
                      break;
                    }
                  }
                }
              }
            }

            if(retune)
            {
              lower_freq_limit = new_lower_freq_limit;
              upper_freq_limit = new_upper_freq_limit;
              band_name = new_band_name;

              switch (cur_config.mode)
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
                  cur_config.base_freq = new_qrss_freq;
                  break;
      
                case Mode::WSPR:
                  cur_config.base_freq = new_wspr_freq;
                  break;
                case Mode::JT65:
                  cur_config.base_freq = new_jt65_freq;
                  break;
                case Mode::JT9:
                case Mode::JT4:
                case Mode::FT8: // TODO
                  cur_config.base_freq = new_jt9_freq;
                  break;
              }
            }
            
//            if(cur_band_module == 0)
//            {
//              cur_band_module = 1;
//              digitalWrite(BAND_SW, HIGH);
//            }
//            else
//            {
//              cur_band_module = 0;
//              digitalWrite(BAND_SW, LOW);
//            }
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle menu button
    if (digitalRead(BTN_BACK) == LOW)
    {
      delay(50);   // delay to debounce
      // yield();
      if (digitalRead(BTN_BACK) == LOW)
      {
        screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer

        // Disable screen saver
        if(screen_saver_enable)
        {
          screen_saver_enable = false;
          delay(200);
          return;
        }

        if (display_mode == DisplayMode::Menu)
        {
          if (menu.selectParent()) // if we are at root menu, exit
          {
            display_mode = DisplayMode::Main;
          }
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          if (ins_del_mode)
          {
            ins_del_mode = false;
          }
          else
          {
            display_mode = DisplayMode::Menu;
          }
        }
        else
        {
          if (cur_state == TxState::Idle)
          {
            cur_setting_selected = 0;
            display_mode = DisplayMode::Menu;
          }
          else
          {
            setTxState(TxState::Idle);
//            morse.reset();
            setNextTx(0);

//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_MAX_SIZE + 6];
            sprintf(send_json, "{\"level\":0,\"text\":\"TX End\",\"freq\":%lu,\"mode\":\"%s\"}",
                cur_config.base_freq, mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name);
            sendSerialPacket(0xFE, send_json);
          }
        }
        // yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  }
}
