/*************************************************************************
 * Copyright (C) 2018,  Alexei Sibidanov                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the RANLUX++ random number generator.            *
 *                                                                       *
 * RANLUX++ is free software: you can redistribute it and/or modify it   *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation, either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * RANLUX++ is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "mulmod.h"
#include <string.h>

extern "C" {
  // b *= a
  void _mul9x9_mul(uint64_t *b, const uint64_t *a);
  // b %= 2^576 - 2^240 + 1
  void _remainder(uint64_t *b);
  // The input and output memory areas must not overlap
  // out = a*b
  void _mul9x9_mulx(uint64_t *out, const uint64_t *a, const uint64_t *b);
  void _mul9x9_mulxadox(uint64_t *out, const uint64_t *a, const uint64_t *b);
  // out = a*b % (2^576 - 2^240 + 1)
  void _mul9x9mod_mulx(uint64_t *out, const uint64_t *a, const uint64_t *b);
  void _mul9x9mod_mulxadox(uint64_t *out, const uint64_t *a, const uint64_t *b);
};

__attribute__((target ("arch=haswell")))
void mul9x9mod(uint64_t *b, const uint64_t *a) {
  uint64_t buf[9];
  _mul9x9mod_mulx(buf,a,b);
  memcpy(b, buf, sizeof(uint64_t)*9);
}

__attribute__((target ("arch=broadwell")))
void mul9x9mod(uint64_t *b, const uint64_t *a){
  uint64_t buf[9];
  _mul9x9mod_mulxadox(buf,a,b);
  memcpy(b, buf, sizeof(uint64_t)*9);
}

__attribute__((target ("arch=skylake")))
void mul9x9mod(uint64_t *b, const uint64_t *a){
  uint64_t buf[9];
  _mul9x9mod_mulxadox(buf,a,b);
  memcpy(b, buf, sizeof(uint64_t)*9);
}

__attribute__ ((target ("default")))
void mul9x9mod(uint64_t *b, const uint64_t *a) {
  uint64_t buf[18];
  memcpy(buf, b, sizeof(uint64_t)*9); _mul9x9_mul(buf, a);
  _remainder(buf);
  memcpy(b, buf, sizeof(uint64_t)*9);
}
