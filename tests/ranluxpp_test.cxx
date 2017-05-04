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

#include "ranluxpp.h"
#include "ranlux.h"
#include "cpuarch.h"
#include <stdio.h>
#include <typeinfo>
#include <cxxabi.h>

// test the performance by summing up 10^9 float numbers
// note the sum is always 2^24=16777216 for floats due to rounding errors
template<typename T>
void speedtest(){
  ranluxpp g1(3124);
  int N = 1000*1000*1000;
  T sum = 0;
  int status;
  printf("Summing up %d %s type random numbers...\n",
	 N, abi::__cxa_demangle(typeid(T).name(), 0, 0, &status));
  for(int i=0;i<N;i++) sum += g1(sum);
  printf("sum=%lf\n", (double)(sum));
}

// test the performance by summing up 10^9 float numbers
// note the sum is always 2^24=16777216 for floats due to rounding errors
template<typename T>
void speedtest_array(){
  ranluxpp g1(3124);
  int N = 1000*1000*10, M = 100;
  T xs[M], sum = 0;
  int status;
  printf("Summing up %d %s type random numbers from the array of size %d...\n",
	 N*M, abi::__cxa_demangle(typeid(T).name(), 0, 0, &status), M);
  for (int i = 0; i < N; i++){
    g1.getarray(M, xs);
    for(int i=0;i<M;i++) sum += xs[i];
  }
  printf("sum=%lf\n", (double)(sum));
}

// propagate carry bit through 9*64 bit number
void add_carry(uint64_t *va, uint64_t k){
  int i = 0;
  do{
    va[i] += k;
    k = (va[i] < k);
  } while(k && ++i<9);
}

// subtract two 9*64 bit numbers
// d = a - b
void sub(uint64_t *vd, const uint64_t *va, const uint64_t *vb){
  uint64_t k = 0; // carry bit
  for (int i = 0; i < 9; i++){
    uint64_t a = va[i], b = vb[i];
    b += k;
    k = (b < k) | (a < b);
    vd[i] = a - b;
  }
}

// insert 24 bit number at certain position
// x   LCG state
// pos position to insert from beginning of the array
// y   24 bit number to insert
void pack24(uint64_t *x, int pos, uint32_t y){
  int n = pos >> 6, l = pos & 0x3f;
  x[n] |= (uint64_t)y<<l;
  if(64<24+l) x[n+1] |= y>>(64-l);
}

// pack the sequence of the RANLUX generator into the LCG state according to Eq. 3
// x LCG state
// y RANLUX generator sequence
// k carry bit
void pack(uint64_t *x, const uint32_t *y, uint64_t k){
  for(int i=0;i<9;i++) x[i] = 0;
  for(int i=0;i<24;i++) pack24(x, i*24, y[23-i]);
  uint64_t b[9] = {0,0,0,0,0,0,0,0,0};
  for(int i=0;i<10;i++) pack24(b, i*24, y[9-i]);
  sub(x,x,b);
  add_carry(x,k);  
}

// print 9*64 bit number
void print(uint64_t *x){
  // for(int i=0;i<9;i++) printf("%016lx",x[8-i]); printf("\n");
  printf("%016lx%016lx ... %016lx%016lx\n",x[8],x[7],x[1],x[0]);
}

void compare_ranlux(){
  int stride = 17;
  ranluxI_scalar g0(100, stride);
  int p = 24*stride;
  ranluxpp g1(0, p);
  printf("Multiplier A = a^%d = ",p); print(g1.getmultiplier());

  uint32_t y[24], k;
  g0.getstate(y, k);
  uint64_t x[9];
  pack(x, y, k);
  printf("SKIPPING: x_%d = ",0); print(x);
  for(int i=0;i<9;i++) g1.getstate()[i] = x[i];
  printf("     LCG: x_%d = ",0); print(g1.getstate());

  int i0 = 0, i1 = 0;
  int N = 1;
  do {
    for(;i0<N;i0++) g0.nextstate(stride);
    g0.getstate(y,k); pack(x, y, k);
    printf("SKIPPING: x_%d = ",N); print(x);
    
    for(;i1<N;i1++) g1.nextstate();
    printf("     LCG: x_%d = ",N); print(g1.getstate());

    uint64_t *z = g1.getstate();
    for(int j=0;j<9;j++)
      if(x[j] != z[j]){
	printf("Test failed at step %d.\n",N);
	return;
      }
  } while((N<<=3)<1000*1000*1000);
  printf("Test successfully passed.\n");
  printf("The sequence obtained by the RANLUX algorithm is identical to one obtained by the modular multiplication in %d steps.\n",i0);
}

void usage(int argc, char **argv){
  printf("Program to test the performance of the Linear Congruential Generator with long integer modular multiplication.\n");
  printf("The generator produces the recurrent sequence:\n");
  printf("  x_{i+1} = A * x_{i} %% m\n");
  printf("    m = 2^576 - 2^240 + 1\n");
  printf("    A = a^p %% m\n");
  printf("    a = m - (m - 1)/2^24\n");
  printf("    p = 2048 (default value)\n");
  printf("Generator parameters are derived from the RANLUX program.\n\n");
  printf("Usage: %s ntest\n",argv[0]);
  printf("  ntest: 0 -- perform self consistency test\n");
  printf("              (comparing sequencies with the RANLUX generator)\n");
  printf("         1 -- sum of 10^9 float random numbers\n");
  printf("         2 -- sum of 10^9 double random numbers\n");
  printf("         3 -- sum of 10^9 float random numbers (array)\n");
  printf("         4 -- sum of 10^9 double random numbers (array)\n");
}

int main(int argc, char **argv){
  if(argc==1||argc>2) { usage(argc,argv); return 0;}

  int ntest = atoi(argv[1]);
  printf("Selected code path is optimized for the %s CPU architecture.\n",getarch());
  if ( ntest == 0 ){
    compare_ranlux();
  } else if(ntest == 1){
    speedtest<float>();
  } else if(ntest == 2){
    speedtest<double>();
  } else if(ntest == 3){
    speedtest_array<float>();
  } else if(ntest == 4){
    speedtest_array<double>();
  } else {
    usage(argc,argv);
  }
  return 0;
}
