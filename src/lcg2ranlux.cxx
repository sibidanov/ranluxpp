/*************************************************************************
 * Copyright (C) 2017,  Alexei Sibidanov                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or	 *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,	 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License	 *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include <stdint.h>
#include <stdio.h>

extern "C" {
  // the first 18 limbs of the fractional exansion of
  // the LCG state x divided by the modulus m = 2^576-2^240 + 1
  // 0.(y0y1y2 .. first 18 limbs ...y15y16y17) = x/m
  // via multiplication by the reciprocal 2^1152/m = 2^576 + 2^240 - 1 + 2^-96 - 2^-335 + 2^-432
  // the state vector x is assumed has the zero guard limbs
  // i.e. x[10] = x[-1] = 0 to shift by multiple of 8 bits
  // via unaligned memory read
  void _divmult(uint64_t y[18], const uint64_t x[9]);
}

// unpack the fraction expansion x/m = 0.(x0x1x2 ... x15x16x17)
// into 24 24-bit chunks to get ordinary subract-with-borrow sequence
void unpack2ranluxseq(uint32_t y[24], const uint64_t x[18]){
  const int mask = (1<<24)-1;
  y[23] = (x[9]>>0) & mask;
  y[22] = (x[9]>>24) & mask;
  y[21] = (x[9]>>48|x[10]<<16) & mask;
  y[20] = (x[10]>>8) & mask;
  y[19] = (x[10]>>32) & mask;
  y[18] = (x[10]>>56|x[11]<<8) & mask;
  y[17] = (x[11]>>16) & mask;
  y[16] = (x[11]>>40);
  y[15] = (x[12]>>0) & mask;
  y[14] = (x[12]>>24) & mask;
  y[13] = (x[12]>>48|x[13]<<16) & mask;
  y[12] = (x[13]>>8) & mask;
  y[11] = (x[13]>>32) & mask;
  y[10] = (x[13]>>56|x[14]<<8) & mask;
  y[9] = (x[14]>>16) & mask;
  y[8] = (x[14]>>40);
  y[7] = (x[15]>>0) & mask;
  y[6] = (x[15]>>24) & mask;
  y[5] = (x[15]>>48|x[16]<<16) & mask;
  y[4] = (x[16]>>8) & mask;
  y[3] = (x[16]>>32) & mask;
  y[2] = (x[16]>>56|x[17]<<8) & mask;
  y[1] = (x[17]>>16) & mask;
  y[0] = (x[17]>>40);
}

// transform the LCG state to a corresponding subtract-with-borrow sequence
// or RANLUX sequence
// returns carry
bool getranluxseq(uint32_t y[24], const uint64_t x[9]){
  uint64_t b[18], zxz[1+9+1];
  zxz[0] = 0;
  for(int i=0;i<9;i++) zxz[i+1] = x[i];
  zxz[10] = 0;
  
  _divmult(b, zxz+1);
  unpack2ranluxseq(y, b);
  
  const int mask = (1<<24)-1;
  int y24 = b[8]>>(64-24);
  int d = y24 - y[10];
  
  bool c0 = ((d + y[0] + 1)&mask) == 0;
  bool c1 =  d + c0 > 0;

  return c1;
}

// transform the RANLUX sequence (24 24-bit numbers) and the carry to the LCG state
void getlcgstate(uint64_t x[9], const uint32_t y[24], bool k){
  x[0] = (uint64_t)y[23] | (uint64_t)y[22]<<24 | (uint64_t)y[21]<<48;
  x[1] = (uint64_t)y[21]>>16 | (uint64_t)y[20]<<8 | (uint64_t)y[19]<<32 | (uint64_t)y[18]<<56;
  x[2] = (uint64_t)y[18]>>8 | (uint64_t)y[17]<<16 | (uint64_t)y[16]<<40;
  x[3] = (uint64_t)y[15] | (uint64_t)y[14]<<24 | (uint64_t)y[13]<<48;
  x[4] = (uint64_t)y[13]>>16 | (uint64_t)y[12]<<8 | (uint64_t)y[11]<<32 | (uint64_t)y[10]<<56;
  x[5] = (uint64_t)y[10]>>8 | (uint64_t)y[9]<<16 | (uint64_t)y[8]<<40;
  x[6] = (uint64_t)y[7] | (uint64_t)y[6]<<24 | (uint64_t)y[5]<<48;
  x[7] = (uint64_t)y[5]>>16 | (uint64_t)y[4]<<8 | (uint64_t)y[3]<<32 | (uint64_t)y[2]<<56;
  x[8] = (uint64_t)y[2]>>8 | (uint64_t)y[1]<<16 | (uint64_t)y[0]<<40;

  uint64_t xm[4];
  xm[0] = x[6]<<48 | x[5]>>16;
  xm[1] = x[7]<<48 | x[6]>>16;
  xm[2] = x[8]<<48 | x[7]>>16;
  xm[3] = x[8]>>16;

  int i = 0;
  do { // propagate carry
    x[i] += k;
    k = (x[i] < k);
  } while(k && ++i<9);

  // subtract: x = x - xm
  k = 0; // carry bit
  for (i = 0; i < 4; i++){
    uint64_t a = x[i], b = xm[i];
    b += k;
    k = (b < k) | (a < b);
    x[i] = a - b;
  }
  do { // propagate carry
    uint64_t a = x[i];
    x[i] = a - k;
    k = a < k;
  } while(k && ++i<9);
}
