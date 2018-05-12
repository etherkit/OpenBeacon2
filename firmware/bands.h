/*
 * bands.h
 *
 *  Created on: 21 January 2018
 *      Author: Jason Milldrum
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

#ifndef BANDS_H_
#define BANDS_H_

struct BandData
{
  uint8_t index;
  char name[10];
  uint32_t lower_limit;
  uint32_t upper_limit;
  uint16_t lower_v;
  uint16_t upper_v;
  uint32_t cw_freq;
  uint32_t wspr_freq;
  uint32_t jt65_freq;
  uint32_t jt9_freq;
};

constexpr BandData band_table[] =
{
  {0, "Empty", 0UL, 0UL, 0U, 149U, 0UL, 0UL, 0UL},
  {1, "2200 m", 135700UL, 137800UL, 150U, 299U, 136000UL, 136000UL, 0UL, 0UL},
  {2, "630 m", 472000UL, 479000UL, 300U, 449U, 474200UL, 474200UL, 0UL, 0UL},
  {3, "160 m", 1800000UL, 2000000UL, 450U, 599U, 1820000UL, 1838100UL, 1838000UL, 1840000UL},
  {4, "80 m", 3500000UL, 4000000UL, 600U, 749U, 3560000UL, 3594100UL, 3578000UL, 3580000UL},
  {5, "60 m", 5330500UL, 5406400UL, 750U, 899U, 5358000UL, 5358000UL, 5359000UL, 5359000UL},
  {6, "40 m", 7000000UL, 7300000UL, 900U, 1049U, 7040000UL, 7040100UL, 7078000UL, 7080000UL},
  {7, "30 m", 10100000UL, 10150000UL, 1050U, 1199U, 10110000UL, 10140200UL, 10140000UL, 10142000UL},
  {8, "20 m", 14000000UL, 14350000UL, 1200U, 1349U, 14060000UL, 14097100UL, 14078000UL, 14080000UL},
  {9, "17 m", 18068000UL, 18168000UL, 1350U, 1499U, 18080000UL, 18106100UL, 18104000UL, 18106000UL},
  {10, "15 m", 21000000UL, 21450000UL, 1500U, 1649U, 21060000UL, 21096100UL, 21078000UL, 21080000UL},
  {11, "12 m", 24890000UL, 24990000UL, 1650U, 1799U, 24926000UL, 24926100UL, 24919000UL, 24921000UL},
  {12, "10 m", 28000000UL, 29700000UL, 1800U, 1949U, 28060000UL, 28126100UL, 28078000UL, 28080000UL},
  {13, "6 m", 50000000UL, 54000000UL, 1950U, 2099U, 50200000UL, 50294500UL, 50278000UL, 50280000UL},
  {15, "2 m", 144000000UL, 148000000UL, 2250U, 2399U, 144200000UL, 144490500UL, 144120000UL, 144122000UL}
};
#endif /* BANDS_H_ */
