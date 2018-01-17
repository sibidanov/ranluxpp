/*************************************************************************
 * Copyright (C) 2018,  Alexei Sibidanov                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * This program shows usage of the RANLUX++ random number generator and  *
 * also benchmarks it.                                                   *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU Lesser General Public License as        *
 * published by the Free Software Foundation, either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * This program is distributed in the hope that it will be useful, but   *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
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

// print 9*64 bit number
void print(uint64_t *x){
  // for(int i=0;i<9;i++) printf("%016lx",x[8-i]); printf("\n");
  printf("%016lx%016lx ... %016lx%016lx\n",x[8],x[7],x[1],x[0]);
}

// print 24*24 bit number
void print(uint32_t *x, bool k){
  // for(int i=0;i<24;i++) printf("%06x ",x[23-i]); printf("k=%d\n",k);
  printf("%06x %06x %06x ... %06x %06x %06x k=%d\n",x[23],x[22],x[21],x[2],x[1],x[0],k);
}

void compare_ranlux_1(){
  int stride = 17;
  ranluxI_scalar g0(100, stride);
  int p = 24*stride;
  ranluxpp g1(0, p);
  printf("Multiplier A = a^%d = ",p); print(g1.getmultiplier());

  uint32_t y[24], k, y2[24], k2;
  g0.getstate(y, k);
  uint64_t x[9];
  getlcgstate(x, y, k);
  for(int i=0;i<9;i++) g1.getstate()[i] = x[i];

  int i0 = 1, N = 1;
  do {
    g0.nextstate(stride);
    g0.getstate(y, k);
    g1.nextstate();
    k2 = getranluxseq(y2, g1.getstate());
    if(k != k2){
      printf("Test failed at step %d. RANLUX carry bit = %d, LCG carry bit = %d\n",N,k,k2);
      return;
    }
    for(int j=0;j<24;j++)
      if(y[j] != y2[j]){
	printf("Test failed at step %d. RANLUX number y[%d]=0x%x, LCG number y[%d]=0x%x\n",N,j,y[j],j,y2[j]);
	return;
      }
    if(i0==N){
      i0 <<= 1;
      printf("RANLUX: y_%d = ",N); print(y,k);
      printf("   LCG: y_%d = ",N); print(y2,k2);
    }
  } while(++N<1000*1000*100);
  printf("Test successfully passed.\n");
  printf("The transformed LCG state and the RANLUX sequence is identical for %d steps.\n",N);
}

void compare_ranlux_0(){
  int stride = 17;
  ranluxI_scalar g0(100, stride);
  int p = 24*stride;
  ranluxpp g1(0, p);
  printf("Multiplier A = a^%d = ",p); print(g1.getmultiplier());

  uint32_t y[24], k;
  g0.getstate(y, k);
  uint64_t x[9];
  getlcgstate(x, y, k);
  printf("RANLUX: x_%d = ",0); print(x);
  for(int i=0;i<9;i++) g1.getstate()[i] = x[i];
  printf("   LCG: x_%d = ",0); print(g1.getstate());

  int i0 = 1, N = 1;
  do {
    g0.nextstate(stride);
    g0.getstate(y,k); getlcgstate(x, y, k);
    g1.nextstate();

    uint64_t *z = g1.getstate();
    for(int j=0;j<9;j++)
      if(x[j] != z[j]){
	printf("Test failed at step %d.\n",N);
	printf("RANLUX: x_%d = ",0); print(x);
	printf("   LCG: x_%d = ",0); print(g1.getstate());
	return;
      }
    if(i0==N){
      i0 <<= 1;
      printf("RANLUX: y_%d = ",N); print(x);
      printf("   LCG: y_%d = ",N); print(z);
    }
  } while(++N<1000*1000*100);
  printf("Test successfully passed.\n");
  printf("The transformed LCG state and the RANLUX sequence is identical for %d steps.\n",N);
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
  printf("              (the RANLUX generator sequence is transformed to LCG state and compared)\n");
  printf("         1 -- perform self consistency test\n");
  printf("              (the LCG state is transformed to RANLUX generator sequence and compared)\n");
  printf("         2 -- sum of 10^9 float random numbers\n");
  printf("         3 -- sum of 10^9 double random numbers\n");
  printf("         4 -- sum of 10^9 float random numbers (array)\n");
  printf("         5 -- sum of 10^9 double random numbers (array)\n");
}

int main(int argc, char **argv){
  if(argc==1||argc>2) { usage(argc,argv); return 0;}

  int ntest = atoi(argv[1]);
  printf("Selected code path is optimized for the %s CPU architecture.\n",getarch());
  if ( ntest == 0 ){
    compare_ranlux_0();
  } else if(ntest == 1){
    compare_ranlux_1();
  } else if(ntest == 2){
    speedtest<float>();
  } else if(ntest == 3){
    speedtest<double>();
  } else if(ntest == 4){
    speedtest_array<float>();
  } else if(ntest == 5){
    speedtest_array<double>();
  } else {
    usage(argc,argv);
  }
  return 0;
}
