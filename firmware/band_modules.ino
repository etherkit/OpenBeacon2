void selectBand()
{
  #ifdef REV_A
  static uint8_t prev_band_module_index[3] = {0, 0, 0};
  static uint32_t new_freq, new_qrss_freq, new_wspr_freq, new_jt65_freq, new_jt9_freq;
  static uint32_t new_lower_freq_limit, new_upper_freq_limit;

  //  constexpr uint8_t ADC0_RING_BUF_SIZE = 4;
  //  static uint16_t adc0_ring_buf[ADC0_RING_BUF_SIZE];
  //  static uint8_t adc0_ring_buf_pos = 0;
  //  uint32_t adc0_ring_buf_total = 0;

  // TODO: handle out of bounds
  //  uint16_t adc0 = analogRead(A0);
  //  yield();
  //  adc0_ring_buf[adc0_ring_buf_pos++] = adc0;
  //  if(adc0_ring_buf_pos >= ADC0_RING_BUF_SIZE)
  //  {
  //    adc0_ring_buf_pos = 0;
  //  }
  //  for(uint8_t i = 0; i <= ADC0_RING_BUF_SIZE; ++i)
  //  {
  //    if(adc0_ring_buf[i] == 0)
  //    {
  //      adc0_ring_buf_total += adc0;
  //    }
  //    else
  //    {
  //      adc0_ring_buf_total += adc0_ring_buf[i];
  //    }
  //    yield();
  //  }
  //  band_id = adc0_ring_buf_total / ADC0_RING_BUF_SIZE;

  prev_band_module_index[2] = prev_band_module_index[1];
  prev_band_module_index[1] = prev_band_module_index[0];
  prev_band_module_index[0] = cur_config.band;

  band_id = analogRead(ADC_BAND_ID);

  yield();
  band_id = (band_id * ANALOG_REF) / 4096UL;
  yield();

  for (auto band : band_table)
  {
    if (band_id < band.upper_v && band_id > band.lower_v)
    {
      if (band.index != cur_config.band)
      {
        prev_band_module_index[0] = band.index;
        //band_index = band.index;
        new_lower_freq_limit = band.lower_limit;
        new_upper_freq_limit = band.upper_limit;
        new_qrss_freq = band.qrss_freq;
        new_wspr_freq = band.wspr_freq;
        new_jt65_freq = band.jt65_freq;
        new_jt9_freq = band.jt9_freq;
        //        if(cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
        //        {
        //          switch(mode)
        //          {
        //          case Mode::DFCW3:
        //          case Mode::DFCW6:
        //          case Mode::DFCW10:
        //          case Mode::DFCW120:
        //          case Mode::QRSS3:
        //          case Mode::QRSS6:
        //          case Mode::QRSS10:
        //          case Mode::QRSS120:
        //          case Mode::CW:
        //          case Mode::HELL:
        //            new_freq = band.qrss_freq;
        //            break;
        //
        //          case Mode::WSPR:
        //            new_freq = band.wspr_freq;
        //            break;
        //          case Mode::JT65:
        //            new_freq = band.jt65_freq;
        //            break;
        //          case Mode::JT9:
        //          case Mode::JT4:
        //            new_freq = band.jt9_freq;
        //            break;
        //          }
        //        }
      }
    }
  }
  yield();

  // Guard against ADC glitches by not changing bands until
  // three consequtive reads of the same band
  if (prev_band_module_index[0] == prev_band_module_index[1] && prev_band_module_index[1] == prev_band_module_index[2])
  {
    // If the band index is changed, change bands only when not transmitting,
    // unless band index is 0, which indicates removal of the band module
    //if(band_index != prev_band_module_index[0] && (cur_state != TxState::Idle || band_index == 0))
    //if(band_index != prev_band_module_index[0] && (!tx_lock || prev_band_module_index[0] == 0))
    if (cur_config.band != prev_band_module_index[0])
    {
      lower_freq_limit = new_lower_freq_limit;
      upper_freq_limit = new_upper_freq_limit;

      if (cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
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
          case Mode::CW:
          case Mode::HELL:
            new_freq = new_qrss_freq;
            break;

          case Mode::WSPR:
            new_freq = new_wspr_freq;
            break;
          case Mode::JT65:
            new_freq = new_jt65_freq;
            break;
          case Mode::JT9:
          case Mode::JT4:
          case Mode::FT8: // TODO
            new_freq = new_jt9_freq;
            break;
        }
      }

      //if(cur_state != TxState::Idle)
      if (tx_lock)
      {
        // Terminate the transmission if module is removed while transmitting
        if (prev_band_module_index[0] == 0)
        {
          setTxState(TxState::Idle);
          tx_enable = false;
          return;
        }
      }
      else
      {
        cur_config.base_freq = new_freq;
      }
    }
    // Then change band index
    cur_config.band = prev_band_module_index[0];

    //    if(band_index == 0)
    //    {
    //      tx_lock = true;
    //      cur_config.base_freq = 0;
    //    }

    //    if(!tx_lock)
    //    {
    //      cur_config.base_freq = new_freq;
    //    }
  }
  else if (prev_band_module_index[2] == 0 && prev_band_module_index[1] != 0 && prev_band_module_index[0] != 0)
  {
    tx_lock = false;
  }
  else
  {
    return;
  }

  //  if(band_index == 0 && prev_band_module_index[1] == 0 && prev_band_module_index[0] == 0)
  //  {
  //    tx_lock = true;
  //    cur_config.base_freq = 0;
  //  }
  //  else
  //  {
  //    if(tx_lock == true && cur_state == TxState::Idle)
  //    {
  //      //tx_lock = false;
  //    }
  //  }


  //  if(prev_band_module_index[1] == 0 && prev_band_module_index[0] != 0 && band_index != 0)
  //  {
  //    tx_lock = false;
  //  }
  //  else if(prev_band_module_index[0] != prev_band_module_index[1] && prev_band_module_index[0] == band_index)
  //  {
  //    return;
  //  }

  //  if(!tx_lock)
  //  {
  //    cur_config.base_freq = new_freq;
  //  }
  //  else
  //  {
  //    if(band_index == 0)
  //    {
  //      cur_config.base_freq = new_freq;
  //    }
  //  }
  yield();
  #endif
  #ifdef REV_B
  static uint8_t prev_band_module_index_1[3] = {0, 0, 0};
  static uint8_t prev_band_module_index_2[3] = {0, 0, 0};
  static uint32_t new_freq, new_qrss_freq, new_wspr_freq, new_jt65_freq, new_jt9_freq;
  static uint32_t new_lower_freq_limit, new_upper_freq_limit;
  static std::string new_band_name;

  if(select_band_reset)
  {
    prev_band_module_index_1[2] = 0;
    prev_band_module_index_1[1] = 0;
    prev_band_module_index_1[0] = 0;
  
    prev_band_module_index_2[2] = 0;
    prev_band_module_index_2[1] = 0;
    prev_band_module_index_2[0] = 0;

    select_band_reset = false;
  }
  else
  {
    prev_band_module_index_1[2] = prev_band_module_index_1[1];
    prev_band_module_index_1[1] = prev_band_module_index_1[0];
    prev_band_module_index_1[0] = band_module_index_1;
  
    prev_band_module_index_2[2] = prev_band_module_index_2[1];
    prev_band_module_index_2[1] = prev_band_module_index_2[0];
    prev_band_module_index_2[0] = band_module_index_2;
  }

  band_id_1 = analogRead(ADC_BAND_ID_1);
  yield();
  band_id_2 = analogRead(ADC_BAND_ID_2);
  yield();
  
//  if(cur_band_module == 0)
//  {
//    band_id = analogRead(ADC_BAND_ID_1);
//  }
//  else if(cur_band_module == 1)
//  {
//    band_id = analogRead(ADC_BAND_ID_2);
//  }

  band_id_1 = (band_id_1 * ANALOG_REF) / 4096UL;
  band_id_2 = (band_id_2 * ANALOG_REF) / 4096UL;
  yield();

//  for (auto band_module : band_module_table)
//  {
//    yield();
//    band_id = cur_band_module == 0 ? band_id_1 : band_id_2;
//    if (band_id < band_module.upper_v && band_id > band_module.lower_v)
//    {
//      // Module change
//      if (band_module.index != (cur_band_module == 0 ? band_module_index_1 : band_module_index_2))
//      {
//        if(cur_band_module == 0)
//        {
//          prev_band_module_index_1[0] = band_module.index;
//        }
//        else
//        {
//          prev_band_module_index_2[0] = band_module.index;
//        }
//        
//        // Set band to first in band module list
//        for(auto band : band_table)
//        {
//          if(band.module_index == band_module.index)
//          {
//            band_index = band.index;
//
//            // Set default frequencies for band
//            new_lower_freq_limit = band.lower_limit;
//            new_upper_freq_limit = band.upper_limit;
//            new_qrss_freq = band.qrss_freq;
//            new_wspr_freq = band.wspr_freq;
//            new_jt65_freq = band.jt65_freq;
//            new_jt9_freq = band.jt9_freq;
//            new_band_name = band.name;
//            break;
//          }
//        }
//      }
//    }
//  }
  for (auto band_module : band_module_table)
  {
    yield();
    if (band_id_1 < band_module.upper_v && band_id_1 > band_module.lower_v)
    {
      // Module change
      if (band_module.index != band_module_index_1)
      {
        prev_band_module_index_1[0] = band_module.index;
        
        // Set band to first in band module list
        if(cur_band_module == 0)
        {
          for(auto band : band_table)
          {
            if(band.module_index == band_module.index)
            {
              cur_config.band = band.index;
  
              // Set default frequencies for band
              new_lower_freq_limit = band.lower_limit;
              new_upper_freq_limit = band.upper_limit;
              new_qrss_freq = band.qrss_freq;
              new_wspr_freq = band.wspr_freq;
              new_jt65_freq = band.jt65_freq;
              new_jt9_freq = band.jt9_freq;
              new_band_name = band.name;
              break;
            }
          }
        }
      }
    }
  }
  yield();
  for (auto band_module : band_module_table)
  {
    yield();
    if (band_id_2 < band_module.upper_v && band_id_2 > band_module.lower_v)
    {
      prev_band_module_index_2[0] = band_module.index;
      
      // Module change
      if (band_module.index != band_module_index_2)
      {
//        prev_band_module_index_2[0] = band_module.index;
        
        // Set band to first in band module list
        if(cur_band_module == 1)
        {
          for(auto band : band_table)
          {
            if(band.module_index == band_module.index)
            {
              cur_config.band = band.index;
  
              // Set default frequencies for band
              new_lower_freq_limit = band.lower_limit;
              new_upper_freq_limit = band.upper_limit;
              new_qrss_freq = band.qrss_freq;
              new_wspr_freq = band.wspr_freq;
              new_jt65_freq = band.jt65_freq;
              new_jt9_freq = band.jt9_freq;
              new_band_name = band.name;
              break;
            }
          }
        }
      }
    }
  }
  yield();

  // Guard against ADC glitches by not changing bands until
  // three consequtive reads of the same band
  if (prev_band_module_index_1[0] == prev_band_module_index_1[1] && prev_band_module_index_1[1] == prev_band_module_index_1[2])
  {
    if(cur_band_module == 0)
    {
      // If the band index is changed, change bands only when not transmitting,
      // unless band index is 0, which indicates removal of the band module
      if (band_module_index_1 != prev_band_module_index_1[0])
      {
        lower_freq_limit = new_lower_freq_limit;
        upper_freq_limit = new_upper_freq_limit;
        band_name = new_band_name;
  
        if (cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
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
            case Mode::CW:
            case Mode::HELL:
              new_freq = new_qrss_freq;
              break;
  
            case Mode::WSPR:
              new_freq = new_wspr_freq;
              break;
            case Mode::JT65:
              new_freq = new_jt65_freq;
              break;
            case Mode::JT9:
            case Mode::JT4:
            case Mode::FT8: // TODO
              new_freq = new_jt9_freq;
              break;
          }

          if (!tx_lock)
          {
            cur_config.base_freq = new_freq;
          }
        }
  
        //if(cur_state != TxState::Idle)
        if (tx_lock)
        {
          // Terminate the transmission if module is removed while transmitting
          if (prev_band_module_index_1[0] == 0)
          {
            setTxState(TxState::Idle);
            tx_enable = false;
            return;
          }
        }
        // else
        // {
        //   cur_config.base_freq = new_freq;
        // }
      }
    }
    // Then change band index
    band_module_index_1 = prev_band_module_index_1[0];
  }
  else if (prev_band_module_index_1[2] == 0 && prev_band_module_index_1[1] != 0 && prev_band_module_index_1[0] != 0)
  {
    tx_lock = false;
  }
  else
  {
    return;
  }
  yield();
  
  if (prev_band_module_index_2[0] == prev_band_module_index_2[1] && prev_band_module_index_2[1] == prev_band_module_index_2[2])
  {
    if(cur_band_module == 1)
    {
      // If the band index is changed, change bands only when not transmitting,
      // unless band index is 0, which indicates removal of the band module
      if (band_module_index_2 != prev_band_module_index_2[0])
      {
        lower_freq_limit = new_lower_freq_limit;
        upper_freq_limit = new_upper_freq_limit;
        band_name = new_band_name;
  
        if (cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
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
            case Mode::CW:
            case Mode::HELL:
              new_freq = new_qrss_freq;
              break;
  
            case Mode::WSPR:
              new_freq = new_wspr_freq;
              break;
            case Mode::JT65:
              new_freq = new_jt65_freq;
              break;
            case Mode::JT9:
            case Mode::JT4:
            case Mode::FT8: // TODO
              new_freq = new_jt9_freq;
              break;
          }

          if (!tx_lock)
          {
            cur_config.base_freq = new_freq;
          }
        }
  
        //if(cur_state != TxState::Idle)
        if (tx_lock)
        {
          // Terminate the transmission if module is removed while transmitting
          if (prev_band_module_index_2[0] == 0)
          {
            setTxState(TxState::Idle);
            tx_enable = false;
            return;
          }
        }
        // else
        // {
        //   cur_config.base_freq = new_freq;
        // }
      }
    }
    // Then change band index
    band_module_index_2 = prev_band_module_index_2[0];
  }
  else if (prev_band_module_index_2[2] == 0 && prev_band_module_index_2[1] != 0 && prev_band_module_index_2[0] != 0)
  {
    tx_lock = false;
  }
  else
  {
    return;
  }
  yield();

  #endif
}
