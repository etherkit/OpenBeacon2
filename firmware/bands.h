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
    uint32_t wspr_freq;
};

constexpr BandData band_table[] =
{
  {0, "Empty", 0UL, 0UL, 0U, 149U, 0UL},
  {1, "2200 m", 135700UL, 137800UL, 150U, 299U, 136000UL},
  {2, "630 m", 472000UL, 479000UL, 300U, 449U, 474200UL},
  {3, "160 m", 1800000UL, 2000000UL, 450U, 599U,1838100UL},
  {4, "80 m", 3500000UL, 4000000UL, 600U, 749U, 3594100UL},
  {5, "60 m", 5330500UL, 5406400UL, 750U, 899U, 5358000UL},
  {6, "40 m", 7000000UL, 7300000UL, 900U, 1049U, 7040100UL},
  {7, "30 m", 10100000UL, 10150000UL, 1050U, 1199U, 10140200UL},
  {8, "20 m", 14000000UL, 14350000UL, 1200U, 1349U, 14097100UL},
  {9, "17 m", 18068000UL, 18168000UL, 1350U, 1499U, 18106100UL},
  {10, "15 m", 21000000UL, 21450000UL, 1500U, 1649U, 21096100UL},
  {11, "12 m", 24890000UL, 24990000UL, 1650U, 1799U, 24926100UL},
  {12, "10 m", 28000000UL, 29700000UL, 1800U, 1949U, 28126100UL},
  {13, "6 m", 50000000UL, 54000000UL, 1950U, 2099U, 50294500UL},
  {15, "2 m", 144000000UL, 148000000UL, 2250U, 2399U, 144490500UL}
};
#endif /* BANDS_H_ */
