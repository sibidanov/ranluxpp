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

/*************************************************************************
 * Optimized implementation of the subtract-with-borrow generator with   *
 * skipping also known as the RANLUX generator. About an order of        *
 * magnutude speedup compared to other popular implementations is        *
 * achieved for the scalar version. The SIMD extensions additional       *
 * boost are provided.                                                   *
 *************************************************************************/

#include <stdint.h>
#include "immintrin.h"
#include "ranluxpp.h"

#pragma once

#define   likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

class ranluxI_scalar {
protected:
  uint32_t _x[24]; // state vector
  uint32_t _c;     // carry bit
  int _p;          // number of states to skip
  int _pos;        // current position in the state vector
public:
  ranluxI_scalar(){};
  ranluxI_scalar(int seed):ranluxI_scalar(seed,17){};
  ranluxI_scalar(int seed, int lux);
  void init(int seed);
  void nextstate(int nstates);
  float operator()(){
    if(unlikely(_pos>=24)){_pos = 0; nextstate(_p);}
    return ((int32_t*)_x)[_pos++]*(1.0f/0x1p24f);
  }
  void getstate(uint32_t *x, uint32_t &c){
    for(int i=0;i<24;i++) x[i] = _x[i]; c = _c;
  }
};

class ranluxI_SSE {
protected:
  __m128i _x[24]; // state vector
  __m128i _c;     // carry bits
  int _p;         // number of states to skip
  int _pos;       // current position in the state vector
public:
  ranluxI_SSE(int seed):ranluxI_SSE(seed,17){};
  ranluxI_SSE(int seed, int lux);
  void init(int seed, bool sameseed=0);
  void nextstate(int nstates);
  float operator()() {
    if(unlikely(_pos>=4*24)){_pos = 0; nextstate(_p);}
    return ((int32_t*)_x)[_pos++]*(1.0f/0x1p24f);
  }
};

#ifdef __AVX2__
class ranluxI_AVX {
protected:
  __m256i _x[24]; // state vector
  __m256i _c;     // carry bits
  int _p;         // number of states to skip
  int _pos;       // current position in the state vector
public:
  ranluxI_AVX(int seed):ranluxI_AVX(seed,17){};
  ranluxI_AVX(int seed, int lux);
  void init(int seed, bool sameseed=0);
  void nextstate(int nstates);
  float operator()(){
    if(unlikely(_pos>=8*24)){_pos = 0; nextstate(_p);}
    return ((int32_t*)_x)[_pos++]*(1.0f/0x1p24f);
  }
};
#endif

// For testing purpose, full emulation of the original FORTRAN routine
// using the optimized subtract-with-borrow algorithm
// to compare it with the code:
// http://www.cpc.cs.qub.ac.uk/summaries/ACPR_v1_0.html or
// http://www.cpc.cs.qub.ac.uk/summaries/ACPR_v2_0.html
class ranluxI_James: public ranluxI_scalar{
protected:
  int _nskip; // how many numbers generate and skip
  int _luxury; // luxury level
  int _i;      // current position in state vector
  int _in24;   // numbers delivered to a user after the skipping
  int _seed;   // the seed number used to initialize the generator
  uint64_t _kount; // total generated numbers

  float tofloat(int);
  int nextpos(); // next position in the state vector
  void skip(); // skip nskip numbers
  void setlux(int luxury);
public:
  ranluxI_James():ranluxI_James(0){}
  ranluxI_James(unsigned int seed, int luxury = 3);
  void ranlux(float*, int n);
  void rluxgo(int luxury, int seed, int k1, int k2);
  void rluxin(int*);
  void rluxut(int*);
  void rluxat(int &lout, int &inout, int &k1, int &k2);
};

// For testing purpose, full emulation of the original FORTRAN routine
// using LCG as a skipping engine
// to compare it with the code:
// http://www.cpc.cs.qub.ac.uk/summaries/ACPR_v1_0.html or
// http://www.cpc.cs.qub.ac.uk/summaries/ACPR_v2_0.html
class ranluxpp_James: public ranluxpp{
protected:
  uint32_t _y[24]; // unpacked RANLUX sequence
  uint32_t _c;// unpacked carry
  int _nskip; // how many numbers generate and skip
  int _luxury; // luxury level
  int _i;      // current position in state vector
  int _seed;   // the seed number used to initialize the generator
  uint64_t _kount; // total generated numbers

  float tofloat(int);
  int nextpos(); // next position in the state vector
  void skip(); // skip nskip numbers
  void setlux(int luxury);
public:
  ranluxpp_James():ranluxpp_James(0){}
  ranluxpp_James(unsigned int seed, int luxury = 3);
  void ranlux(float*, int n);
  void rluxgo(int luxury, int seed, int k1, int k2);
  void rluxin(int*);
  void rluxut(int*);
  void rluxat(int &lout, int &inout, int &k1, int &k2);
};
