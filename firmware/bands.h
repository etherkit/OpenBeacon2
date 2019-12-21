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

#ifdef REV_A
struct BandData
{
  uint8_t index;
  std::string name;
  uint32_t lower_limit;
  uint32_t upper_limit;
  uint16_t lower_v;
  uint16_t upper_v;
  uint32_t cw_freq;
  uint32_t wspr_freq;
  uint32_t jt65_freq;
  uint32_t jt9_freq;
  uint32_t ft8_freq;
};

const BandData band_table[] =
{
  {0, "Empty", 0UL, 0UL, 0U, 149U, 0UL, 0UL, 0UL},
  {1, "2200 m", 135700UL, 137800UL, 150U, 299U, 136000UL, 136000UL, 0UL, 0UL, 0UL},
  {2, "630 m", 472000UL, 479000UL, 300U, 449U, 474200UL, 474200UL, 0UL, 0UL, 0UL},
  {3, "160 m", 1800000UL, 2000000UL, 450U, 599U, 1820000UL, 1838100UL, 1838000UL, 1840000UL, 1850000UL},
  {4, "80 m", 3500000UL, 4000000UL, 600U, 749U, 3560000UL, 3568600UL, 3578000UL, 3580000UL, 3574000UL},
  {5, "60 m", 5330500UL, 5406400UL, 750U, 899U, 5358000UL, 5358000UL, 5359000UL, 5359000UL, 5359000UL},
  {6, "40 m", 7000000UL, 7300000UL, 900U, 1049U, 7039800UL, 7040100UL, 7078000UL, 7080000UL, 7075000UL},
  {7, "30 m", 10100000UL, 10150000UL, 1050U, 1199U, 10139900UL, 10140200UL, 10140000UL, 10142000UL, 10136000UL},
  {8, "20 m", 14000000UL, 14350000UL, 1200U, 1349U, 14096800UL, 14097100UL, 14078000UL, 14080000UL, 14075000UL},
  {9, "17 m", 18068000UL, 18168000UL, 1350U, 1499U, 18080000UL, 18106100UL, 18104000UL, 18106000UL, 18101000UL},
  {10, "15 m", 21000000UL, 21450000UL, 1500U, 1649U, 21060000UL, 21096100UL, 21078000UL, 21080000UL, 21075000UL},
  {11, "12 m", 24890000UL, 24990000UL, 1650U, 1799U, 24926000UL, 24926100UL, 24919000UL, 24921000UL, 24916000UL},
  {12, "10 m", 28000000UL, 29700000UL, 1800U, 1949U, 28060000UL, 28126100UL, 28078000UL, 28080000UL, 28075000UL},
  {13, "6 m", 50000000UL, 54000000UL, 1950U, 2099U, 50200000UL, 50294500UL, 50278000UL, 50280000UL, 50324000UL},
  {15, "2 m", 144000000UL, 148000000UL, 2250U, 2399U, 144200000UL, 144490500UL, 144120000UL, 144122000UL, 144175000UL}
};
#endif
#ifdef REV_B
struct BandModuleData
{
  uint8_t index;
  std::string name;
  uint16_t lower_v;
  uint16_t upper_v;
};

const BandModuleData band_module_table[] =
{
  {0, "Empty", 0, 150},
  {1, "2200m", 151, 300},
  {2, "630m", 301, 450},
  {3, "160m", 451, 600},
  {4, "80m/60m", 601, 750},
  {5, "40m/30m", 751, 900},
  {6, "20m/17m", 901, 1050},
  {7, "15m/12m/10m", 1051, 1200},
  {8, "6m/4m", 1201, 1350},
  {9, "2m/1.25m", 1351, 1500}
};

struct BandData
{
  uint8_t index;
  std::string name;
  uint8_t module_index;
  uint32_t lower_limit;
  uint32_t upper_limit;
  uint32_t qrss_freq;
  uint32_t qrss_lower_limit;
  uint32_t qrss_upper_limit;
  uint32_t wspr_freq;
  uint32_t wspr_lower_limit;
  uint32_t wspr_upper_limit;
  uint32_t jt65_freq;
  uint32_t jt65_lower_limit;
  uint32_t jt65_upper_limit;
  uint32_t jt9_freq;
  uint32_t jt9_lower_limit;
  uint32_t jt9_upper_limit;
  uint32_t ft8_freq;
  uint32_t ft8_lower_limit;
  uint32_t ft8_upper_limit;
};

const BandData band_table[] =
{
  {0, "Empty", 0, 0UL, 0UL, 
  0UL, 0UL, 0UL,                         // QRSS
  0UL, 0UL, 0UL,                         // WSPR
  0UL, 0UL, 0UL,                         // JT65
  0UL, 0UL, 0UL,                         // JT9
  0UL, 0UL, 0UL},                        // FT8
  // ----------
  {1, "2200 m", 1, 135700UL, 137800UL, 
  136000UL, 136000UL, 136000UL,          // QRSS
  137500UL, 137400UL, 137600UL,          // WSPR
  0UL, 0UL, 0UL,                         // JT65
  0UL, 0UL, 0UL,                         // JT9
  0UL, 0UL, 0UL},                        // FT8
  // ----------
  {2, "630 m", 2, 472000UL, 479000UL, 
  476100UL, 476000UL, 476200UL,          // QRSS
  475700UL, 475600UL, 475800UL,          // WSPR
  0UL, 0UL, 0UL,                         // JT65
  0UL, 0UL, 0UL,                         // JT9
  0UL, 0UL, 0UL},                        // FT8
  // ----------
  {3, "160 m", 3, 1800000UL, 2000000UL, 
  1843200UL, 1843100UL, 1843300UL,       // QRSS
  1838100UL, 1838000UL, 1838200UL,       // WSPR
  1838000UL, 1838000UL, 1838000UL,       // JT65
  1840000UL, 1840000UL, 1840000UL,       // JT9
  1850000UL, 1850000UL, 1850000UL},      // FT8
  // ----------
  {4, "80 m", 4, 3500000UL, 4000000UL, 
  3569900UL, 3569800UL, 3570000UL,       // QRSS
  3568600UL, 3568500UL, 3568700UL,       // WSPR
  3578000UL, 3578000UL, 3578000UL,       // JT65
  3580000UL, 3580000UL, 3580000UL,       // JT9
  3574000UL, 3574000UL, 3574000UL},      // FT8
  // ----------
  {5, "60 m", 4, 5330500UL, 5406400UL, 
  5366100UL, 5366000UL, 5366200UL,       // QRSS
  5366200UL, 5366100UL, 5366300UL,       // WSPR
  5359000UL, 5359000UL, 5359000UL,       // JT65
  5359000UL, 5359000UL, 5359000UL,       // JT9
  5359000UL, 5359000UL, 5359000UL},      // FT8
  // ----------
  {6, "40 m", 5, 7000000UL, 7300000UL, 
  7039900UL, 7039800UL, 7040000UL,       // QRSS
  7040100UL, 7040000UL, 7040200UL,       // WSPR
  7078000UL, 7078000UL, 7078000UL,       // JT65
  7080000UL, 7080000UL, 7080000UL,       // JT9
  7075000UL, 7075000UL, 7075000UL},      // FT8
  // ----------
  {7, "30 m", 5, 10100000UL, 10150000UL, 
  10140000UL, 10139900UL, 10140100UL,    // QRSS
  10140200UL, 10140100UL, 10140300UL,    // WSPR
  10140000UL, 10140000UL, 10140000UL,    // JT65
  10142000UL, 10142000UL, 10142000UL,    // JT9
  10136000UL, 10136000UL, 10136000UL},   // FT8
  // ----------
  {8, "20 m", 6, 14000000UL, 14350000UL, 
  14096900UL, 14096800UL, 14097000UL,    // QRSS
  14097100UL, 14097000UL, 14097200UL,    // WSPR
  14078000UL, 14078000UL, 14078000UL,    // JT65
  14080000UL, 14080000UL, 14080000UL,    // JT9
  14075000UL, 14075000UL, 14075000UL},   // FT8
  // ----------
  {9, "17 m", 6, 18068000UL, 18168000UL, 
  18105900UL, 18105800UL, 18106000UL,    // QRSS
  18106100UL, 18106000UL, 18106200UL,    // WSPR
  18104000UL, 18104000UL, 18104000UL,    // JT65
  18106000UL, 18106000UL, 18106000UL,    // JT9
  18101000UL, 18101000UL, 18101000UL},   // FT8
  // ----------
  {10, "15 m", 7, 21000000UL, 21450000UL, 
  21000800UL, 21000700UL, 21000900UL,    // QRSS
  21096100UL, 21096000UL, 21096200UL,    // WSPR
  21078000UL, 21078000UL, 21078000UL,    // JT65
  21080000UL, 21080000UL, 21080000UL,    // JT9
  21075000UL, 21075000UL, 21075000UL},   // FT8
  // ----------
  {11, "12 m", 7, 24890000UL, 24990000UL, 
  24890800UL, 24890700UL, 24890900UL,    // QRSS
  24926100UL, 24926000UL, 24926200UL,    // WSPR
  24919000UL, 24919000UL, 24919000UL,    // JT65
  24921000UL, 24921000UL, 24921000UL,    // JT9
  24916000UL, 24916000UL, 24916000UL},   // FT8
  // ----------
  {12, "10 m", 7, 28000000UL, 29700000UL, 
  28000800UL, 28000700UL, 28000900UL,     // QRSS
  28126100UL, 28126000UL, 28126200UL,     // WSPR
  28078000UL, 28078000UL, 28078000UL,     // JT65
  28080000UL, 28080000UL, 28080000UL,     // JT9
  28075000UL, 28075000UL, 28075000UL},    // FT8
  // ----------
  {13, "6 m", 8, 50000000UL, 54000000UL, 
  50200000UL, 50200000UL, 50200000UL,     // QRSS
  50294500UL, 50294400UL, 50294600UL,     // WSPR
  50278000UL, 50278000UL, 50278000UL,     // JT65
  50280000UL, 50280000UL, 50280000UL,     // JT9
  50324000UL, 50324000UL, 50324000UL},    // FT8
  // ----------
  {15, "2 m", 9, 144000000UL, 148000000UL, 
  144200000UL, 144200000UL, 144200000UL,  // QRSS
  144490500UL, 144490400UL, 144490600UL,  // WSPR
  144120000UL, 144120000UL, 144120000UL,  // JT65
  144122000UL, 144122000UL, 144122000UL,  // JT9
  144175000UL, 144175000UL, 144175000UL}  // FT8
};
#endif
#endif /* BANDS_H_ */
