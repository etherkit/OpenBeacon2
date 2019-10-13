void processSerialIn()
{
  constexpr uint32_t DEFAULT_TIME = 946684800; // 1 Jan 2000

  uint32_t pctime;

  if(SerialUSB)
  {
    if (SerialUSB.available())
    {
//      char serial_packet[JSON_MAX_SIZE + 6];
//      char payload[JSON_MAX_SIZE];
//      StaticJsonDocument<JSON_MAX_SIZE> json_rx_doc;
//      StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
//      char send_json[JSON_MAX_SIZE + 6];
//      DynamicJsonDocument json_rx_doc(JSON_MAX_SIZE);
//      DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
  
      if (SerialUSB.read() == PACKET_ID)
      {
        char payload[JSON_PACKET_MAX_SIZE];
        
        // Get message type
        uint8_t message_type = SerialUSB.read();
    
        // Determine payload length
        uint16_t payload_len = SerialUSB.read();
        payload_len <<= 8;
        payload_len += SerialUSB.read();
    
        // Get the payload
        if(payload_len > 0)
        {
          SerialUSB.readBytes(payload, payload_len);
        }
    
        // Make sure the packet is terminated correctly
        if(SerialUSB.read() == '\n')
        {
        }
        else return;
    
        // Deserialize the JSON document
        DynamicJsonDocument json_rx_doc(JSON_MAX_SIZE);
//        StaticJsonDocument<JSON_MAX_SIZE> json_rx_doc;
        DeserializationError error = deserializeJson(json_rx_doc, payload);
    
        if(!error)
        {
        }
        else return;
    
        // Handle message
        switch(message_type)
        {
        case 0x01:
          pctime = json_rx_doc["timestamp"];
//          sendSerialPacket(0xFE, "{\"text\":\"hello\"}");
          if (pctime >= DEFAULT_TIME)
          {
            time_sync_expire = pctime + TIME_EXPIRE;
            next_time_sync = pctime + TIME_SYNC_INTERVAL;
            rtc.setEpoch(pctime); // Sync RTC to the time received on the serial port
            if (initial_time_sync == 0)
            {
              initial_time_sync = pctime;
            }
            time_sync_request = false;
          }
          break;
        case 0x02:
          if(json_rx_doc["set"] == true)
          {
            // Only make changes if not transmitting
            if (cur_state == TxState::Idle)
            {
              if(json_rx_doc["config"] == "base_freq")
              {
                uint64_t temp_freq = json_rx_doc["value"];
                if(temp_freq >= lower_freq_limit && temp_freq <= upper_freq_limit)
                {
                  cur_config.base_freq = temp_freq;
                }
              }
              else if(json_rx_doc["config"] == "mode")
              {
                // TODO: bounds checking
                uint8_t temp_mode = json_rx_doc["value"];
                selectMode(temp_mode);
//                serializeConfig();
              }
              else if(json_rx_doc["config"] == "band")
              {
                uint8_t temp_band_index = json_rx_doc["value"];
                
                // Make sure the requested band is available
                if(band_table[temp_band_index].module_index == band_module_index_1 ||
                  band_table[temp_band_index].module_index == band_module_index_2)
                {
                  if(band_table[temp_band_index].module_index == band_module_index_1)
                  {
                    cur_band_module = 0;
                    digitalWrite(BAND_SW, LOW);
                  }
                  else
                  {
                    cur_band_module = 1;
                    digitalWrite(BAND_SW, HIGH);
                  }
                  cur_config.band = temp_band_index;
//                  band_index = cur_config.band;
                  lower_freq_limit = band_table[cur_config.band].lower_limit;
                  upper_freq_limit = band_table[cur_config.band].upper_limit;
                  band_name = band_table[cur_config.band].name;
    
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
                      cur_config.base_freq = band_table[cur_config.band].cw_freq;
                      break;
          
                    case Mode::WSPR:
                      cur_config.base_freq = band_table[cur_config.band].wspr_freq;
                      break;
                    case Mode::JT65:
                      cur_config.base_freq = band_table[cur_config.band].jt65_freq;
                      break;
                    case Mode::JT9:
                    case Mode::JT4:
                    case Mode::FT8: // TODO
                      cur_config.base_freq = band_table[cur_config.band].jt9_freq;
                      break;
                  }
                }
//                serializeConfig();
              }
              else if(json_rx_doc["config"] == "wpm")
              {
                char temp_str[40];
                uint8_t temp_wpm = json_rx_doc["value"];
                cur_config.wpm = temp_wpm;
                sprintf(temp_str, "U%lu", cur_config.wpm);
                settings["wpm"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "tx_intv")
              {
                char temp_str[40];
                uint8_t temp_tx_intv = json_rx_doc["value"];
                cur_config.tx_intv = temp_tx_intv;
                sprintf(temp_str, "U%lu", cur_config.tx_intv);
                settings["tx_intv"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "dfcw_offset")
              {
                char temp_str[40];
                uint8_t temp_dfcw_offset = json_rx_doc["value"];
                cur_config.dfcw_offset = temp_dfcw_offset;
                sprintf(temp_str, "U%lu", cur_config.dfcw_offset);
                settings["dfcw_offset"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "buffer")
              {
                char temp_str[40];
                uint8_t temp_buffer = json_rx_doc["value"];
                selectBuffer(temp_buffer);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "callsign")
              {
                char temp_str[40];
                strcpy(cur_config.callsign, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.callsign);
                settings["callsign"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "grid")
              {
                char temp_str[40];
                strcpy(cur_config.grid, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.grid);
                settings["grid"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "power")
              {
                char temp_str[40];
                uint8_t temp_power = json_rx_doc["value"];
                cur_config.power = temp_power;
                sprintf(temp_str, "I%u", cur_config.power);
                settings["power"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "pa_bias")
              {
                char temp_str[40];
                uint16_t temp_pa_bias = json_rx_doc["value"];
                cur_config.pa_bias = temp_pa_bias;
                sprintf(temp_str, "U%lu", cur_config.pa_bias);
                settings["pa_bias"].second = std::string(temp_str);
                setPABias(cur_config.pa_bias);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "cwid")
              {
                char temp_str[40];
                bool temp_cwid = json_rx_doc["value"];
                cur_config.cwid = temp_cwid;
                if(temp_cwid)
                {
                  sprintf(temp_str, "B1");
                }
                else
                {
                  sprintf(temp_str, "B0");
                }
                settings["cwid"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_1")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_1, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_1);
                settings["msg_buffer_1"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_2")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_2, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_2);
                settings["msg_buffer_2"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_3")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_3, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_3);
                settings["msg_buffer_3"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_4")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_4, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_4);
                settings["msg_buffer_4"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "si5351_int_corr")
              {
                char temp_str[40];
                int32_t temp_si5351_int_corr = json_rx_doc["value"];
                cur_config.si5351_int_corr = temp_si5351_int_corr;
                sprintf(temp_str, "I%li", cur_config.si5351_int_corr);
//                settings["si5351_int_corr"].second = std::string(temp_str);
                serializeConfig();
              }
            }
          }
          else if(json_rx_doc["get"] == true)
          {
//            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_PACKET_MAX_SIZE];
            
            if(json_rx_doc["config"] == "base_freq")
            {
              json_tx_doc["config"] = "base_freq";
              json_tx_doc["value"] = cur_config.base_freq;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "mode")
            {
              json_tx_doc["config"] = "mode";
              json_tx_doc["value"] = static_cast<uint8_t>(cur_config.mode);
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "band")
            {
              json_tx_doc["config"] = "band";
              json_tx_doc["value"] = static_cast<uint8_t>(cur_config.band);
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "wpm")
            {
              json_tx_doc["config"] = "wpm";
              json_tx_doc["value"] = cur_config.wpm;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "tx_intv")
            {
              json_tx_doc["config"] = "tx_intv";
              json_tx_doc["value"] = cur_config.tx_intv;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "dfcw_offset")
            {
              json_tx_doc["config"] = "dfcw_offset";
              json_tx_doc["value"] = cur_config.dfcw_offset;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "buffer")
            {
              json_tx_doc["config"] = "buffer";
              json_tx_doc["value"] = cur_config.buffer;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "callsign")
            {
              json_tx_doc["config"] = "callsign";
              json_tx_doc["value"] = cur_config.callsign;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "grid")
            {
              json_tx_doc["config"] = "grid";
              json_tx_doc["value"] = cur_config.grid;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "power")
            {
              json_tx_doc["config"] = "power";
              json_tx_doc["value"] = cur_config.power;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "pa_bias")
            {
              json_tx_doc["config"] = "pa_bias";
              json_tx_doc["value"] = cur_config.pa_bias;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "cwid")
            {
              json_tx_doc["config"] = "cwid";
              json_tx_doc["value"] = cur_config.cwid;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_1")
            {
              json_tx_doc["config"] = "msg_buffer_1";
              json_tx_doc["value"] = cur_config.msg_buffer_1;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_2")
            {
              json_tx_doc["config"] = "msg_buffer_2";
              json_tx_doc["value"] = cur_config.msg_buffer_2;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_3")
            {
              json_tx_doc["config"] = "msg_buffer_3";
              json_tx_doc["value"] = cur_config.msg_buffer_3;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_4")
            {
              json_tx_doc["config"] = "msg_buffer_4";
              json_tx_doc["value"] = cur_config.msg_buffer_4;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "si5351_int_corr")
            {
              json_tx_doc["config"] = "si5351_int_corr";
              json_tx_doc["value"] = cur_config.si5351_int_corr;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "mem_free")
            {
              json_tx_doc["config"] = "mem_free";
              json_tx_doc["value"] = freeMemory();
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
          }
          break;
        case 0x04:
          if(json_rx_doc["action"] == "tx_enable")
          {
            tx_enable = true;
            setTxState(TxState::Idle);
            // Re-compose the buffers to reflect changes
            selectBuffer(cur_config.buffer);
            composeWSPRBuffer();
            setNextTx(0);
          }
          else if(json_rx_doc["action"] == "tx_disable")
          {
            tx_enable = false;
            next_tx = UINT32_MAX;
          }
          else if(json_rx_doc["action"] == "tx_cancel")
          {
            if(cur_state != TxState::Idle)
            {
              setTxState(TxState::Idle);
              morse.reset();
              setNextTx(0);

//              DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
//              StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
              char send_json[JSON_PACKET_MAX_SIZE];
              sprintf(send_json, "{\"level\":0,\"text\":\"TX End\",\"mode\":\"%s\",\"freq\":\"%lu\"}",
                mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name, cur_config.base_freq);
//              json_tx_doc["level"] = 0;
//              json_tx_doc["text"] = "TX End";
//              json_tx_doc["data"] = cur_config.base_freq;
//              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0xFE, send_json);
            }
          }
          break;
        case 0x06:
          if(json_rx_doc["enum"] == "modes")
          {
            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_PACKET_MAX_SIZE];
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray modes = root.createNestedArray("modes");

            for(auto m : mode_table)
            {
              modes.add(m.mode_name);
            }

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          else if(json_rx_doc["enum"] == "bands")
          {
            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_PACKET_MAX_SIZE];
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray bands = root.createNestedArray("bands");

//            for(auto b : band_table)
//            {
//              bands.add(b.name);
//            }

            for(auto b : band_table)
            {
              JsonObject nested = bands.createNestedObject();
              nested["name"] = b.name.c_str();
              nested["mod"] = b.module_index;
            }

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          else if(json_rx_doc["enum"] == "band_modules")
          {
            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_PACKET_MAX_SIZE];
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray band_modules = root.createNestedArray("band_modules");

            for(auto b : band_module_table)
            {
              band_modules.add(b.name.c_str());
            }

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          else if(json_rx_doc["enum"] == "inst_band_modules")
          {
            DynamicJsonDocument json_tx_doc(JSON_MAX_SIZE);
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_PACKET_MAX_SIZE];
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray band_modules = root.createNestedArray("inst_band_modules");

            band_modules.add(band_module_index_1);
            band_modules.add(band_module_index_2);

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          break;
        default:
          break;
        }
      }
    }
  }
  else
  {
    // No connection detected
  }
  yield();
}

uint16_t sendSerialPacket(uint8_t msg_type, const char * payload)
{
  char serial_packet[JSON_PACKET_MAX_SIZE + 6];
  uint16_t payload_len = strlen(payload);
  if(payload_len > JSON_PACKET_MAX_SIZE)
  {
    return 0;
  }

  // Build packet header
  *serial_packet = PACKET_ID;
  *(serial_packet + 1) = msg_type;
  *(serial_packet + 2) = (payload_len >> 8) & 0xff;
  *(serial_packet + 3) = payload_len & 0xff;

  // Add in payload
  memcpy(serial_packet + 4, payload, payload_len);

  // Append terminator char
  *(serial_packet + 4 + payload_len) = PACKET_TERM;

  // Send it
  uint16_t bytes_written = SerialUSB.write(serial_packet, payload_len + 5);
  return bytes_written;
  yield();
}

void processTimeSync()
{
  // Check to see if we need to sync
  if (rtc.getEpoch() > next_time_sync)
  {
    sendSerialPacket(0, "");
    time_sync_request = true;
    next_time_sync = rtc.getEpoch() + TIME_SYNC_RETRY_RATE;
  }
  yield();

  // Indicate time sync status
  // This LED has inverted logic
  if (isTimeValid())
  {
    digitalWrite(SYNC_LED, LOW);
  }
  else
  {
    digitalWrite(SYNC_LED, HIGH);
  }
  yield();
}
