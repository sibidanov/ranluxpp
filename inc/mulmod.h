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
#pragma once

__attribute__((target ("arch=haswell")))
void mul9x9mod(uint64_t *b, const uint64_t *a);

__attribute__((target ("arch=broadwell")))
void mul9x9mod(uint64_t *b, const uint64_t *a);

__attribute__((target ("arch=skylake")))
void mul9x9mod(uint64_t *b, const uint64_t *a);

__attribute__((target ("default")))
void mul9x9mod(uint64_t *b, const uint64_t *a);
