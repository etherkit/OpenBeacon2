void txStateMachine()
{
  static bool prev_morse_tx = false;

  switch (meta_mode)
  {
    case MetaMode::CW:
      switch (cur_state)
      {
        case TxState::Idle:
          break;
        case TxState::CW:
          if (!morse.busy)
          {
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
//            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
            char send_json[JSON_PACKET_MAX_SIZE];
            sprintf(send_json, "{\"level\":0,\"text\":\"TX End\",\"freq\":%lu,\"mode\":\"%s\"}",
                cur_config.base_freq, mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name);
//            json_tx_doc["level"] = 0;
//            json_tx_doc["text"] = "TX End";
//            json_tx_doc["data"] = cur_config.base_freq;
//            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0xFE, send_json);
            yield();
            setTxState(TxState::Idle);
            //frequency = (cur_config.base_freq * 100) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
            //frequency = (cur_config.base_freq * 100);
            //change_freq = true;
            //setNextTx(atoi(settings["TX Intv"].substr(1).c_str()));
//            selectMode(static_cast<uint8_t>(mode));

            setNextTx(0);
            cur_screen_saver_x = random(1, 16);
            cur_screen_saver_y = random(1, 16);
          }
          break;
      }
      break;
    case MetaMode::DFCW:
      switch (cur_state)
      {
        case TxState::Idle:
          break;
        case TxState::IDDelay:
          if (cur_timer >= cwid_start)
          {
            setTxState(TxState::CWID);
          }
          break;
        case TxState::CWID:
          if (!morse.busy)
          {
            setNextTx(cur_config.tx_intv);
            setTxState(TxState::Idle);
            morse.tx_enable = false;
            cur_screen_saver_x = random(1, 16);
            cur_screen_saver_y = random(1, 16);
//            #ifdef REV_B
//            digitalWrite(TX_KEY, HIGH);
//            #endif
          }
          break;
        case TxState::DFCW:
          if (morse.tx != prev_morse_tx)
          {
            if (morse.tx)
            {
              frequency = (cur_config.base_freq * 100ULL) + cur_tone_spacing;
              change_freq = true;
            }
            else
            {
              frequency = (cur_config.base_freq * 100ULL);
              change_freq = true;
            }

            prev_morse_tx = morse.tx;
          }

          if (!morse.busy)
          {
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
//            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
            char send_json[JSON_PACKET_MAX_SIZE];
            sprintf(send_json, "{\"level\":0,\"text\":\"TX End\",\"freq\":%lu,\"mode\":\"%s\"}",
                cur_config.base_freq, mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name);
//            json_tx_doc["level"] = 0;
//            json_tx_doc["text"] = "TX End";
//            json_tx_doc["data"] = cur_config.base_freq;
//            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0xFE, send_json);
            yield();
            prev_morse_tx = false;
            frequency = (cur_config.base_freq * 100ULL);
            change_freq = true;
              
            if(cur_config.cwid)
            {
              setTxState(TxState::IDDelay);
              cur_screen_saver_x = random(1, 16);
              cur_screen_saver_y = random(1, 16);
            }
            else
            {
              setTxState(TxState::Idle);
              setNextTx(cur_config.tx_intv);
              morse.tx_enable = false;
              cur_screen_saver_x = random(1, 16);
              cur_screen_saver_y = random(1, 16);
            }
          }
          break;
      }
      break;
    case MetaMode::MFSK:
      switch (cur_state)
      {
        case TxState::Idle:
          break;
        case TxState::IDDelay:
          if (cur_timer >= cwid_start)
          {
            setTxState(TxState::CWID);
          }
          break;
        case TxState::CWID:
          if (!morse.busy)
          {
            setTxState(TxState::Idle);

            setNextTx(cur_config.tx_intv);
            composeMFSKMessage();
            cur_screen_saver_x = random(1, 16);
            cur_screen_saver_y = random(1, 16);
          }
          break;
        case TxState::MFSK:
          if (cur_timer >= next_event)
          {
            ++cur_symbol;
            if (cur_symbol >= cur_symbol_count) //reset everything and switch to idle
            {
//              StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
//              DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
              char send_json[JSON_PACKET_MAX_SIZE];
              sprintf(send_json, "{\"level\":0,\"text\":\"TX End\",\"freq\":%lu,\"mode\":\"%s\"}",
                cur_config.base_freq, mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name);
                // cur_config.base_freq
//              json_tx_doc["level"] = 0;
//              json_tx_doc["text"] = "TX End";
//              json_tx_doc["data"] = cur_config.base_freq;
//              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0xFE, send_json);
              if(cur_config.cwid)
              {
                setTxState(TxState::IDDelay);
                cur_screen_saver_x = random(1, 16);
                cur_screen_saver_y = random(1, 16);
              }
              else
              {
                setTxState(TxState::Idle);
                setNextTx(cur_config.tx_intv);
//                setNextTx(0);
                composeMFSKMessage();
                cur_screen_saver_x = random(1, 16);
                cur_screen_saver_y = random(1, 16);
              }
              //frequency = (cur_config.base_freq * 100) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
              //frequency = (cur_config.base_freq * 100);
              //change_freq = true;
//              setNextTx(cur_config.tx_intv);
//              setNextTx(atoi(settings["tx_intv"].second.substr(1).c_str()));
            }
            else // next symbol
            {
              next_event = cur_timer + cur_symbol_time;
              frequency = (cur_config.base_freq * 100ULL) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
              change_freq = true;
            }
          }
          break;
      }
      break;
  }
  yield();
}

void setTxState(TxState state)
{
  uint8_t key_val;
  switch (state)
  {
    case TxState::Idle:
      tx_lock = false;
      morse.reset();
      morse.tx_enable = false;
      #ifdef REV_A
      digitalWrite(TX_KEY, LOW);
      #endif
      #ifdef REV_B
      if(cur_config.mode == Mode::DFCW3 || cur_config.mode == Mode::DFCW6 || cur_config.mode == Mode::DFCW10 ||
         cur_config.mode == Mode::DFCW120 || cur_config.mode == Mode::QRSS3 || cur_config.mode == Mode::QRSS6 || 
         cur_config.mode == Mode::QRSS10 || cur_config.mode == Mode::QRSS120)
      {
        digitalWrite(TX_KEY, HIGH);
        setPABias(cur_config.pa_bias + 800);
      }
      else
      {
        digitalWrite(TX_KEY, LOW);
      }
      digitalWrite(TX_LED, LOW);
      #endif
      yield();
      si5351.output_enable(SI5351_CLK0, 0);
//      frequency = (cur_config.base_freq * 100ULL);
//      change_freq = true;
//      next_state = prev_state;
      prev_state = cur_state;
      cur_state = state;
//      key_val = digitalRead(TX_KEY);
//      SerialUSB.write('\v');
//      SerialUSB.println(key_val);
      break;
    case TxState::MFSK:
      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      tx_lock = true;
      cur_symbol = 0;
      frequency = (cur_config.base_freq * 100ULL) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
      change_freq = true;
      digitalWrite(TX_KEY, HIGH);
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      digitalWrite(TX_LED, HIGH);
      #endif
      yield();
      si5351.output_enable(SI5351_CLK0, 1);
      //    setPABias(PA_BIAS_FULL);
      next_state = TxState::MFSK;
      prev_state = cur_state;
      cur_state = state;
      next_event = cur_timer + cur_symbol_time;
      break;
    case TxState::CW:
      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      si5351.output_enable(SI5351_CLK0, 1);
      tx_lock = true;
      frequency = (cur_config.base_freq * 100ULL);
      change_freq = true;
      morse.tx_enable = true;
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      #endif
//      morse.output_pin = TX_KEY;
      morse.dfcw_mode = false;
      morse.setWPM(cur_config.wpm);
      next_state = TxState::CW;
      prev_state = cur_state;
      cur_state = state;
      selectBuffer(cur_config.buffer);
//      morse.send(msg_buffer);
      morse.send(msg_buffer.c_str());
      break;
    case TxState::IDDelay:
//      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      digitalWrite(TX_KEY, LOW);
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      digitalWrite(TX_LED, LOW);
      #endif
      si5351.output_enable(SI5351_CLK0, 0);
//      tx_lock = true;
//      frequency = (cur_config.base_freq * 100ULL);
//      change_freq = true;
//      morse.output_pin = TX_KEY;
//      morse.setWPM(wpm);
//      next_state = prev_state;
//      prev_state = cur_state;
      cur_state = state;
      cwid_start = cur_timer + cwid_delay;
      break;
    case TxState::CWID:
//      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      si5351.output_enable(SI5351_CLK0, 1);
      tx_lock = true;
      frequency = (cur_config.base_freq * 100ULL);
      change_freq = true;
      morse.tx_enable = true;
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      #endif
//      morse.output_pin = TX_KEY;
      morse.dfcw_mode = false;
//      morse.setWPM(wpm);
      morse.setWPM(cwid_wpm);
//      next_state = prev_state;
//      prev_state = cur_state;
      cur_state = state;
//      strcpy(cur_callsign, cur_config.callsign);
//      morse.send(cur_callsign);
      morse.send(cur_config.callsign);
      break;
    case TxState::DFCW:
      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      si5351.output_enable(SI5351_CLK0, 1);
      tx_lock = true;
      digitalWrite(TX_KEY, HIGH);
      morse.reset();
      morse.tx_enable = true;
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      digitalWrite(TX_LED, HIGH);
      #endif
      frequency = (cur_config.base_freq * 100ULL);
      change_freq = true;
      morse.dfcw_mode = true;
      morse.setWPM(cur_config.wpm);
      next_state = TxState::DFCW;
      prev_state = cur_state;
      cur_state = state;
      morse.preamble_enable = true;
      selectBuffer(cur_config.buffer);
//      morse.send(msg_buffer);
      morse.send(msg_buffer.c_str());
      break;
    default:
      break;
  }
  yield();
}

void setNextTx(uint8_t minutes)
{
  uint16_t sec_to_add;
  uint8_t ten_min_delay, one_min_delay;

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
    case Mode::HELL:
      one_min_delay = (60 - rtc.getSeconds());
      ten_min_delay = 9 - (rtc.getMinutes() % 10);
      sec_to_add = (one_min_delay) + (ten_min_delay * 60) + ((minutes / 10) * 60);
      break;

    case Mode::CW:
      sec_to_add = (60 - rtc.getSeconds()) + (minutes * 60);
      break;

    case Mode::WSPR:
      sec_to_add = (60 - rtc.getSeconds()) + (rtc.getMinutes() % 2 ? 0 : 60) 
        + (minutes % 2 ? ((minutes * 60) +   60) : (minutes * 60));
      break;

    case Mode::JT65:
    case Mode::JT9:
    case Mode::JT4:
      sec_to_add = (60 - rtc.getSeconds()) + (minutes * 60);
      break;

    case Mode::FT8:
      sec_to_add = (60 - rtc.getSeconds()) + (minutes * 60);
      break;
  }

  next_tx = rtc.getEpoch() + sec_to_add;
  yield();

  // Build next TX time string
  const time_t ntx = static_cast<time_t>(next_tx);
  struct tm * n_tx = gmtime(&ntx);
  sprintf(next_tx_time, "%02u:%02u:%02u", n_tx->tm_hour, n_tx->tm_min, n_tx->tm_sec);
}

void processTxTrigger()
{
  if (rtc.getEpoch() >= next_tx && isTimeValid())
  {
    if(cur_config.rnd_tx)
    {
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
        case Mode::HELL:
          if(random_tx_guard_band > 
            (band_table[cur_config.band].qrss_upper_limit - band_table[cur_config.band].qrss_lower_limit) / 2)
          {
            random_tx_guard_band = 
              (band_table[cur_config.band].qrss_upper_limit - band_table[cur_config.band].qrss_lower_limit) / 2 - 10;
          }
          cur_config.base_freq = band_table[cur_config.band].qrss_lower_limit + random_tx_guard_band +
            random(band_table[cur_config.band].qrss_upper_limit - band_table[cur_config.band].qrss_lower_limit -
              2 * random_tx_guard_band);
          break;
    
        case Mode::CW:
          break;
    
        case Mode::WSPR:
          if(random_tx_guard_band > 
            (band_table[cur_config.band].wspr_upper_limit - band_table[cur_config.band].wspr_lower_limit) / 2)
          {
            random_tx_guard_band = 
              (band_table[cur_config.band].wspr_upper_limit - band_table[cur_config.band].wspr_lower_limit) / 2 - 10;
          }
          cur_config.base_freq = band_table[cur_config.band].wspr_lower_limit + random_tx_guard_band +
            random(band_table[cur_config.band].wspr_upper_limit - band_table[cur_config.band].wspr_lower_limit -
              2 * random_tx_guard_band);
          break;
    
        case Mode::JT65:
          if(random_tx_guard_band > 
            (band_table[cur_config.band].jt65_upper_limit - band_table[cur_config.band].jt65_lower_limit) / 2)
          {
            random_tx_guard_band = 
              (band_table[cur_config.band].jt65_upper_limit - band_table[cur_config.band].jt65_lower_limit) / 2 - 10;
          }
          cur_config.base_freq = band_table[cur_config.band].jt65_lower_limit + random_tx_guard_band +
            random(band_table[cur_config.band].jt65_upper_limit - band_table[cur_config.band].jt65_lower_limit -
              2 * random_tx_guard_band);
          break;
          
        case Mode::JT9:
          if(random_tx_guard_band > 
            (band_table[cur_config.band].jt9_upper_limit - band_table[cur_config.band].jt9_lower_limit) / 2)
          {
            random_tx_guard_band = 
              (band_table[cur_config.band].jt9_upper_limit - band_table[cur_config.band].jt9_lower_limit) / 2 - 10;
          }
          cur_config.base_freq = band_table[cur_config.band].jt9_lower_limit + random_tx_guard_band +
            random(band_table[cur_config.band].jt9_upper_limit - band_table[cur_config.band].jt9_lower_limit -
              2 * random_tx_guard_band);
          break;
          
        case Mode::JT4:
          break;
    
        case Mode::FT8:
          if(random_tx_guard_band > 
            (band_table[cur_config.band].ft8_upper_limit - band_table[cur_config.band].ft8_lower_limit) / 2)
          {
            random_tx_guard_band = 
              (band_table[cur_config.band].ft8_upper_limit - band_table[cur_config.band].ft8_lower_limit) / 2 - 10;
          }
          cur_config.base_freq = band_table[cur_config.band].ft8_lower_limit + random_tx_guard_band +
            random(band_table[cur_config.band].ft8_upper_limit - band_table[cur_config.band].ft8_lower_limit -
              2 * random_tx_guard_band);
          break;
        }
    }
    setTxState(next_state);
    next_tx = UINT32_MAX;
  }

  yield();
}
