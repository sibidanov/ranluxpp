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

#include "cpuarch.h"

__attribute__((target ("arch=haswell")))
const char * getarch() {
  return "Haswell";
}

__attribute__((target ("arch=broadwell")))
const char * getarch(){
  return "Broadwell";
}

__attribute__((target ("arch=skylake")))
const char * getarch(){
  return "Skylake";
}

__attribute__ ((target ("default")))
const char * getarch() {
  return "default";
}
