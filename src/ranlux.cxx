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

#include "ranlux.h"
#include <stdio.h>

#ifdef ASMSKIP
extern "C" {
  // scalar asm optimized skipping procedure
  // state -- state vector
  // carry -- carry bit
  // nskip -- how many states to skip 
  unsigned char _skipstates(uint32_t *state, unsigned char carry, uint64_t nskip);
};
#endif

// Service generator for seeding
// The initialisation is carried out using a Multiplicative
// Congruential generator using formula constants of L'Ecuyer
// as described in "A review of pseudorandom number generators"
// (Fred James) published in Computer Physics Communications 60 (1990)
// pages 329-344
class LEcuyer {
  int64_t _seed;
public:
  LEcuyer(int64_t seed):_seed(seed){}
  int64_t operator ()(){
    const int a = 0xd1a4, b = 0x9c4e, c = 0x2fb3, d = 0x7fffffab;
    int64_t k = _seed / a;
    _seed = b * (_seed - k * a) - k * c ;
    if(_seed < 0) _seed += d;
    return _seed;
  }
};

// Service generator for seeding
// Algorithm A (Additive number generator).  D. E. Knuth, Semi-Numerical
// Algorithms, in: The Art of Computer Programming, vol. 2, 2nd
// ed. (Addison-Wesley, Reading MA, 1981) p. 27
// fetching _nbits at once
template<int _nbits, int _l, int _k>
class ANGen{
  uint64_t _state;
  int _pos;
public:
  ANGen(uint64_t state):_state(state),_pos(0){}
  uint64_t operator ()() __attribute__((noinline)){
    uint64_t res = 0, state = _state;
    int i = _pos, j = i + (_k - _l);
    j = (j>=_k)? j - _k : j;
    for (int l=0; l<_nbits; l++){
      res = (res<<1)|((state>>i)&1);
      state ^= ((state>>j)&1)<<i;
      ++i; i &= (i - _k)>>31; // wrap around
      ++j; j &= (j - _k)>>31; // wrap around
    }
    _pos = i;
    _state = state;
    return res;
  }
};

ranluxI_scalar::ranluxI_scalar(int seed, int p):_p(p), _pos(24) {
  _c = 0x0;
  init(seed);
#ifdef ASMSKIP
  printf("Scalar ranlux skipping (asm version): wasting %d states (p=%d)\n", _p-1, _p*24);
#else
  printf("Scalar ranlux skipping: wasting %d states (p=%d)\n", _p-1, _p*24);
#endif
}

void ranluxI_scalar::init(int iseed) {
  ANGen<24,13,31> s(iseed);
  for (int k=0; k<24; k++) _x[k] = s();
}

void ranluxI_scalar::nextstate(int nstates){
#ifdef ASMSKIP
  _c = _skipstates(_x, _c, nstates);
#else
  auto step = [this](int i, int j, int c) -> int32_t {
    uint32_t d = _x[j] - _x[i] - c;
    _x[i] = d & 0xffffff;
    return d>>31;
  };
  int32_t c = _c;
  while(nstates-- > 0){
    for(int i=23;i>13;i--) c = step(i,i-14,c);
    for(int i=13;i>=0;i--) c = step(i,i+10,c);
  }
  _c = c;
#endif
}

ranluxI_SSE::ranluxI_SSE(int seed, int p):_p(p),_pos(4*24) {
  _c = _mm_set1_epi32(0x0);
  init(seed);
  printf("SSE2 ranlux skipping (4 generators in parallel): wasting %d states (p=%d)\n", _p-1, _p*24);
}

void ranluxI_SSE::init(int iseed, bool sameseed) {
  ANGen<24,13,31> s(iseed);
  if(!sameseed){
    for (int k=0; k<24; k++) _x[k] = _mm_set_epi32(s(),s(),s(),s());
  }else{
    for (int k=0; k<24; k++) _x[k] = _mm_set1_epi32(s());
  }
}

void ranluxI_SSE::nextstate(int nstates){
  auto step = [this](int i, int j, __m128i c) {
    const __m128i m = _mm_set1_epi32(0xffffff);
    __m128i d = _mm_sub_epi32(_mm_sub_epi32(_x[j], _x[i]), c);
    _x[i] = _mm_and_si128(d, m);
    return _mm_srli_epi32(d, 31);
  };

  __m128i c = _c;
  while(nstates-- > 0){
    for(int i=23;i>13;i--) c = step(i,i-14,c);
    for(int i=13;i>=0;i--) c = step(i,i+10,c);
  }
  _c = c;
}

#ifdef __AVX2__
ranluxI_AVX::ranluxI_AVX(int seed, int p):_p(p),_pos(8*24) {
  _c = _mm256_set1_epi32(0x0);
  init(seed);
  printf("AVX2 ranlux skipping (8 generators in parallel): wasting %d states (p=%d)\n", _p-1, _p*24);
}

void ranluxI_AVX::init(int iseed, bool sameseed) {
  ANGen<24,13,31> s(iseed);
  if(!sameseed){
    for (int k=0; k<24; k++) _x[k] = _mm256_set_epi32(s(),s(),s(),s(),s(),s(),s(),s());
  } else {
    for (int k=0; k<24; k++) _x[k] = _mm256_set1_epi32(s());
  }
}

void ranluxI_AVX::nextstate(int nstates){
  auto step = [this](int i, int j, __m256i c) {
    const __m256i m = _mm256_set1_epi32(0xffffff);
    __m256i d = _mm256_sub_epi32(_mm256_sub_epi32(_x[j], _x[i]), c);
    _x[i] = _mm256_and_si256(d, m);
    return _mm256_srli_epi32(d, 31);
  };
  
  __m256i c = _c;
  while(nstates-- > 0){
    for(int i=23;i>13;i--) c = step(i,i-14,c);
    for(int i=13;i>=0;i--) c = step(i,i+10,c);
  }
  _c = c;
}
#endif

ranluxI_James::ranluxI_James(unsigned int seed, int lux){
  rluxgo(lux, seed, 0, 0);
}

void ranluxI_James::skip(){
// every 24th number generation, several random numbers are generated
// and wasted depending upon the luxury level.
  int nstates = (_nskip + 24 - _i)/24;
  _i += nstates*24 - _nskip;
  _in24 = 0;
  _kount += nstates*24;
  nextstate(nstates);
}

float ranluxI_James::tofloat(int i){
  int x = _x[i];
  if(unlikely(x < (1<<12))){
    int j = (i<14)?i+10:i-14;
    x = (x<<12) + (_x[j]>>12);
    if(unlikely(!x)) return (1.0f/0x1p48f);
    return x * (1.0f/0x1p36f);
  }
  return x * (1.0f/0x1p24f);
}

int ranluxI_James::nextpos(){
  if(unlikely(_in24 >= 24)) skip();
  if(unlikely(_i <= 0)){nextstate(1); _i = 24;}
  _in24++;
  _i--;
  return _i;
}

void ranluxI_James::ranlux(float *v, int n) {
  for(int i=0;i<n;i++) v[i] = tofloat(nextpos());
  _kount += n;
}

void ranluxI_James::setlux(int lux){
  // number of additional random numbers that need to be 'thrown away'
  // every 24 numbers is set using _luxury level variable.
  const int lux_levels[5] = {0,24,73,199,365};
  if(lux<0){
    _luxury = 3;
  } else if(lux<=4){
    _luxury = lux;
  } else if(lux<24 || lux>2000){
    _luxury = 4;
  } else {
    _luxury = lux;
    for(int i=0;i<4;i++) if(lux==lux_levels[i]) _luxury = i;
  }
  if(_luxury<=4){
    _nskip = lux_levels[_luxury];
    printf(" RANLUX LUXURY LEVEL SET BY RLUXGO :%2d     P=%4d\n",_luxury,_nskip+24);
  } else {
    _nskip = _luxury - 24;
    printf(" RANLUX P-VALUE SET BY RLUXGO :%5d\n",_luxury);
  }
}

void ranluxI_James::rluxgo(int lux, int seed, int k1, int k2) {
  setlux(lux);
  if(seed<0)
    printf(" Illegal initialization by RLUXGO, negative input seed\n");
  if(seed>0){
    _seed = seed;
    printf(" RANLUX INITIALIZED BY RLUXGO FROM SEEDS%12d%12d%12d\n",_seed,k1,k2);
  } else {
    _seed = 314159265;
    printf(" RANLUX INITIALIZED BY RLUXGO FROM DEFAULT SEED\n");
  }
  
  LEcuyer ns(_seed);
  for(int i = 0; i < 24; i++) _x[i] = ns() & 0xffffff;
  _c = !_x[23];
  
  _i    = 0;
  _in24 = 0;

  _kount = k1 + k2*(1000*1000*1000);
  uint64_t nstates = (_kount + 24 - _i)/24;
  _i   += nstates*24 - _kount;
  if(nstates) nextstate(nstates);
}

void ranluxI_James::rluxin(int state[25]){
  printf("FULL INITIALIZATION OF RANLUX WITH 25 INTEGERS:\n");
  for(int i=0;i<25;i++) {
    printf("%12d", state[i]); if(!((i+1)%5)) printf("\n");
  }
  for(int i=0;i<24;i++) _x[i] = state[i];
  int isd = state[24];
  _c = isd<0;
  isd = abs(isd);

  _i = isd%100;
  isd /= 100;
  isd /= 100;
  _in24 = isd%100;
  isd /= 100;
  setlux(isd);
  _seed = -1;
}

void ranluxI_James::rluxut(int state[25]){
  // Entry to ouput seeds as integers
  for(int i=0;i<24;i++) state[i] = _x[i];
  state[24] = _i + 100*100*_in24 + 100*100*100*_luxury;
  if(_c) state[24] = -state[24];
}

void ranluxI_James::rluxat(int &lout, int &inout, int &k1, int &k2){
  lout  = _luxury;
  inout = _seed;
  k1    = _kount%(1000*1000*1000);
  k2    = _kount/(1000*1000*1000);
}

ranluxpp_James::ranluxpp_James(unsigned int seed, int lux):ranluxpp(0,1){
  rluxgo(lux, seed, 0, 0);
}

void ranluxpp_James::skip(){
  nextstate();
  _c = getranluxseq(_y, _x);
}

float ranluxpp_James::tofloat(int i){
  int x = _y[i];
  if(unlikely(x < (1<<12))){
    int j = (i<14)?i+10:i-14;
    x = (x<<12) + (_y[j]>>12);
    if(unlikely(!x)) return (1.0f/0x1p48f);
    return x * (1.0f/0x1p36f);
  }
  return x * (1.0f/0x1p24f);
}

int ranluxpp_James::nextpos(){
  if(unlikely(_i <= 0)){ _i = 24; skip();}
  return --_i;
}

void ranluxpp_James::ranlux(float *v, int n) {
  for(int i=0;i<n;i++) v[i] = tofloat(nextpos());
  _kount += n;
}

void ranluxpp_James::setlux(int lux){
  // number of additional random numbers that need to be 'thrown away'
  // every 24 numbers is set using _luxury level variable.
  const int lux_levels[5] = {0,24,73,199,365};
  if(lux<0){
    _luxury = 3;
  } else if(lux<=4){
    _luxury = lux;
  } else if(lux<24 || lux>2000){
    _luxury = 4;
  } else {
    _luxury = lux;
    for(int i=0;i<4;i++) if(lux==lux_levels[i]) _luxury = i;
  }
  if(_luxury<=4){
    _nskip = lux_levels[_luxury];
    printf(" RANLUX LUXURY LEVEL SET BY RLUXGO :%2d     P=%4d\n",_luxury,_nskip+24);
  } else {
    _nskip = _luxury - 24;
    printf(" RANLUX P-VALUE SET BY RLUXGO :%5d\n",_luxury);
  }
  setskip(_nskip+24);
}

void ranluxpp_James::rluxgo(int lux, int seed, int k1, int k2) {
  setlux(lux);
  if(seed<0)
    printf(" Illegal initialization by RLUXGO, negative input seed\n");
  if(seed>0){
    _seed = seed;
    printf(" RANLUX INITIALIZED BY RLUXGO FROM SEEDS%12d%12d%12d\n",_seed,k1,k2);
  } else {
    _seed = 314159265;
    printf(" RANLUX INITIALIZED BY RLUXGO FROM DEFAULT SEED\n");
  }

  LEcuyer ns(_seed);
  for(int i = 0; i < 24; i++) _y[i] = ns() & 0xffffff;
  _c = !_y[23];
  getlcgstate(_x, _y, _c);

  jump(24);
  _c = getranluxseq(_y, _x);

  _i = 0;

  _kount = k1 + k2*(1000*1000*1000);
  uint64_t nstates = (_kount + 24 - _i)/24;
  _i += nstates*24 - _kount;
  if(_kount) jump(_kount);
}

void ranluxpp_James::rluxin(int state[25]){
  printf("FULL INITIALIZATION OF RANLUX WITH 25 INTEGERS:\n");
  for(int i=0;i<25;i++) {
    printf("%12d", state[i]); if(!((i+1)%5)) printf("\n");
  }
  for(int i=0;i<24;i++) _y[i] = state[i];
  int isd = state[24];
  _c = isd<0;

  getlcgstate(_x, _y, _c);

  isd = abs(isd);

  _i = isd%100;
  isd /= 100;
  isd /= 100;
  isd /= 100;
  setlux(isd);
  _seed = -1;
}

void ranluxpp_James::rluxut(int state[25]){
  // Entry to ouput seeds as integers
  bool c = getranluxseq((uint32_t*)state, _x);
  state[24] = _i + 100*100*100*_luxury;
  if(c) state[24] = -state[24];
}

void ranluxpp_James::rluxat(int &lout, int &inout, int &k1, int &k2){
  lout  = _luxury;
  inout = _seed;
  k1    = _kount%(1000*1000*1000);
  k2    = _kount/(1000*1000*1000);
}
