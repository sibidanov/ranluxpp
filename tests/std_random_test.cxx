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

#include <random>
#include <stdio.h>
#include <typeinfo>
#include <cxxabi.h>

// wrapper for C++ standard generators
template<typename gen, typename T, int dm>
class stdrandom {
  gen _gen;
public:
  stdrandom() __attribute__((noinline)) {}

  T operator()() __attribute__((noinline)) {
    return std::generate_canonical<T,dm>(_gen); // integer to floating point conversion
  }
};

// test the performance by summing up 10^9 float numbers
// note the sum for float type is always 2^24=16777216 due to rounding errors
template<typename gen, typename T, int dm>
void speedtest(const char *name) {
  stdrandom<gen, T, dm> dis;
  int N = 1000*1000*1000;
  T sum = 0;
  int status;
  printf("Generator %s for %s type with %d random bits in the mantissa.\n",
	 name, abi::__cxa_demangle(typeid(T).name(), 0, 0, &status), dm);
  printf("Summing up %d random numbers...\n",N);
  for (int i = 0; i < N; i++) sum += dis();
  printf("sum=%lf\n",(double)(sum));
}

void usage(int argc, char **argv){
  printf("Standard C++11 generator performance test: sum up 10^9 random numbers in a loop.\n");
  printf("usage: %s ntest\n",argv[0]);
  printf("  ntest: 0 -- std::minstd_rand to produce floats with 24 random bits in mantissa.\n");
  printf("         1 -- std::minstd_rand to produce doubles with 53 random bits in mantissa.\n");
  printf("         2 -- std::mt19937_64 to produce floats with 24 random bits in mantissa.\n");
  printf("         3 -- std::mt19937_64 to produce doubles with 53 random bits in mantissa.\n");
  printf("         4 -- std::mt19937 to produce floats with 24 random bits in mantissa.\n");
  printf("         5 -- std::mt19937 to produce doubles with 53 random bits in mantissa.\n");
  printf("         6 -- std::ranlux24_base to produce floats with 24 random bits in mantissa.\n");
  printf("         7 -- std::ranlux24_base to produce doubles with 48 random bits in mantissa.\n");
  printf("         8 -- std::ranlux48_base to produce floats with 24 random bits in mantissa.\n");
  printf("         9 -- std::ranlux48_base to produce doubles with 48 random bits in mantissa.\n");
  printf("         10 -- std::ranlux24 (p=223) to produce floats with 24 random bits in mantissa.\n");
  printf("         11 -- std::ranlux24 (p=223) to produce doubles with 48 random bits in mantissa.\n");
  printf("         12 -- std::ranlux48 (p=389) to produce floats with 24 random bits in mantissa.\n");
  printf("         13 -- std::ranlux48 (p=389) to produce doubles with 48 random bits in mantissa.\n");
}

int main(int argc, char **argv){
  if ( argc==1 || argc>2 ) { usage(argc,argv); return 0;}
  int itest = -1, lux = 0;
  if(argc>1) itest = atoi(argv[1]);
  switch(itest){
  case 0: speedtest<std::minstd_rand,float,24>("std::minstd_rand"); break;
  case 1: speedtest<std::minstd_rand,double,53>("std::minstd_rand"); break;
  case 2: speedtest<std::mt19937_64,float,24>("std::mt19937_64"); break;
  case 3: speedtest<std::mt19937_64,double,53>("std::mt19937_64"); break;
  case 4: speedtest<std::mt19937,float,24>("std::mt19937"); break;
  case 5: speedtest<std::mt19937,double,53>("std::mt19937"); break;
  case 6: speedtest<std::ranlux24_base,float,24>("std::ranlux24_base"); break;
  case 7: speedtest<std::ranlux24_base,double,48>("std::ranlux24_base"); break;
  case 8: speedtest<std::ranlux48_base,float,24>("std::ranlux48_base"); break;
  case 9: speedtest<std::ranlux48_base,double,48>("std::ranlux48_base"); break;
  case 10: speedtest<std::ranlux24,float,24>("std::ranlux24"); break;
  case 11: speedtest<std::ranlux24,double,48>("std::ranlux24"); break;
  case 12: speedtest<std::ranlux48,float,24>("std::ranlux48"); break;
  case 13: speedtest<std::ranlux48,double,48>("std::ranlux48"); break;
  default: usage(argc,argv); break;
  }
}
