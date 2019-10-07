/*
 * modes.h
 *
 *  Created on: 21 January 2018
 *     Author: Jason Milldrum
 *     Company: Etherkit
 *
 *     Copyright (c) 2018, Jason Milldrum
 *     All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice, this list
 *  of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice, this list
 *  of conditions and the following disclaimer in the documentation and/or other
 *  materials provided with the distribution.
 *
 *  - Neither the name of Etherkit nor the names of its contributors may be
 *  used to endorse or promote products derived from this software without specific
 *  prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 *  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MODES_H_
#define MODES_H_

//#include <JTEncode.h>

enum class Mode {DFCW3, DFCW6, DFCW10, DFCW120, QRSS3, QRSS6, QRSS10, QRSS120,
  CW, HELL, WSPR, JT65, JT9, JT4, FT8};
enum class MetaMode {CW, DFCW, MFSK};

struct ModeData
{
  Mode index;
  char mode_name[10];
  MetaMode meta_mode;
  float WPM;
  uint8_t buffer_size;
  uint16_t tone_spacing; // In Hz / 100
  uint16_t symbol_time;  // In ms
  uint8_t symbol_count;
  uint16_t tx_interval_mult;  // In seconds
};

const ModeData mode_table[] =
{
  {Mode::DFCW3, "DFCW3", MetaMode::DFCW, 0.4, 100, 600, 0, 0, 0},
  {Mode::DFCW6, "DFCW6", MetaMode::DFCW, 0.2, 100, 600, 0, 0, 0},
  {Mode::DFCW10, "DFCW10", MetaMode::DFCW, 0.12, 100, 600, 0, 0, 0},
  {Mode::DFCW120, "DFCW120", MetaMode::DFCW, 0.01, 100, 600, 0, 0, 0},
  {Mode::QRSS3, "QRSS3", MetaMode::CW, 0.4, 100, 0, 0, 0, 0},
  {Mode::QRSS6, "QRSS6", MetaMode::CW, 0.2, 100, 0, 0, 0, 0},
  {Mode::QRSS10, "QRSS10", MetaMode::CW, 0.12, 100, 0, 0, 0, 0},
  {Mode::QRSS120, "QRSS120", MetaMode::CW, 0.01, 100, 0, 0, 0, 0},
  {Mode::CW, "CW", MetaMode::CW, 25, 100, 0, 0, 0, 10},
  {Mode::HELL, "HELL", MetaMode::MFSK, 3.5, 100, 0, 0, 0, 0},
  {Mode::WSPR, "WSPR", MetaMode::MFSK, 1.75, 163, 146, 682, WSPR_SYMBOL_COUNT, 120},
  {Mode::JT65, "JT65", MetaMode::MFSK, 1.75, 200, 269, 371, JT65_SYMBOL_COUNT, 60},
  {Mode::JT9, "JT9", MetaMode::MFSK, 1.75, 200, 173, 576, JT9_SYMBOL_COUNT, 60},
  {Mode::JT4, "JT4", MetaMode::MFSK, 1.75, 200, 437, 229, JT4_SYMBOL_COUNT, 60},
  {Mode::FT8, "FT8", MetaMode::MFSK, 1.75, 200, 625, 159, FT8_SYMBOL_COUNT, 15}
};

//constexpr ModeData mode_table[] =
//{
//  {Mode::DFCW3, "DFCW3", MetaMode::MORSE, 0.4, 100, 0, 0, selectModeDFCW3},
//  {Mode::DFCW6, "DFCW6", MetaMode::MORSE, 0.2, 100, 0, 0, selectModeDFCW6},
//  {Mode::DFCW10, "DFCW10", MetaMode::MORSE, 0.12, 100, 0, 0, selectModeDFCW10},
//  {Mode::DFCW120, "DFCW120", MetaMode::MORSE, 0.01, 100, 0, 0, selectModeDFCW120},
//  {Mode::QRSS3, "QRSS3", MetaMode::MORSE, 0.4, 100, 0, 0, selectModeQRSS3},
//  {Mode::QRSS6, "QRSS6", MetaMode::MORSE, 0.2, 100, 0, 0, selectModeQRSS6},
//  {Mode::QRSS10, "QRSS10", MetaMode::MORSE, 0.12, 100, 0, 0, selectModeQRSS10},
//  {Mode::QRSS120, "QRSS120", MetaMode::MORSE, 0.01, 100, 0, 0, selectModeQRSS120},
//  {Mode::CW, "CW", MetaMode::MORSE, 25, 100, 0, 0, selectModeCW},
//  {Mode::HELL, "Hell", MetaMode::MFSK, 3.5, 100, 0, 0, selectModeHELL},
//  {Mode::WSPR, "WSPR", MetaMode::MFSK, 1.75, 163, 146, 683, selectModeWSPR},
//  {Mode::JT65, "JT65", MetaMode::MFSK, 1.75, 200, 269, 372, selectModeJT65},
//  {Mode::JT9, "JT9", MetaMode::MFSK, 1.75, 200, 174, 576, selectModeJT9},
//  {Mode::JT4, "JT4", MetaMode::MFSK, 1.75, 200, 437, 229, selectModeJT4},
//  {Mode::CAL, "Cal", MetaMode::MORSE, 10, 100, 0, 0, selectModeCAL}
//};

#endif /* MODES_H_ */
