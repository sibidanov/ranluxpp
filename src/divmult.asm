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

.text
	.global _divmult
// void _divmult(uint64_t y[18], const uint64_t x[9]);
// divide the LCG state x by the modulus m = 2^576-2^240+1
// via multiplication by the reciprocal 2^1152/m	
// y = x*(2^576 + 2^240 - 1 + 2^-96 - 2^-335 + 2^-432);
	.set y, %rdi
	.set x, %rsi
	.set t0, %rax
	.set t0d, %eax
	.set t1, %r8
	.set t2, %r9
	.set t3, %rdx
	.set tm, %r11
	.set u, %r10
	.set ud, %r10d
	.align 16
_divmult:
	xor ud, ud
	.set cnt, 0
	.rept 9
	mov $0, t0d
	sbb cnt*8(x), t0 
	mov t0, cnt*8(y)
	.set cnt, cnt+1
	.endr
	
	.set cnt, 0
	.rept 9
	mov cnt*8(x), t0
	sbb $0, t0 
	mov t0, (cnt+9)*8(y)
	.set cnt, cnt + 1
	.endr

	xor ud, ud
	.set cnt, 0
	.rept 10
	mov (cnt*8-6)(x), t0
        mov (cnt+3)*8(y), t1
	adc t0,t1
	mov t1, (cnt+3)*8(y)
	.set cnt, cnt + 1
	.endr
	
	.set cnt, 13
	.rept 5
        mov cnt*8(y), u
	adc $0, u
	mov u, cnt*8(y)
	.if cnt==13
	jae 1f
	.endif
	.set cnt, cnt + 1
	.endr
1:	
	xor ud, ud
	.set cnt, 0
	.rept 8
	mov (cnt*8+96/8)(x), t0
        mov (cnt)*8(y), t1
	adc t0,t1
	mov t1, (cnt)*8(y)
	.set cnt, cnt + 1
	.endr
	
	.rept 10
        mov cnt*8(y), u
	adc $0, u
	mov u, cnt*8(y)
	.if cnt==8
	jae 2f
	.endif
	.set cnt, cnt + 1
	.endr
2:
	xor ud, ud
	.set cnt, 0
	.rept 3
	mov (cnt*8+432/8)(x), t0
        mov (cnt)*8(y), t1
	adc t0,t1
	mov t1, (cnt)*8(y)
	.set cnt, cnt + 1
	.endr
	
	.rept 15
        mov cnt*8(y), u
	adc $0, u
	mov u, cnt*8(y)
	.if cnt==3
	jae 3f
	.endif
	.set cnt, cnt + 1
	.endr
3:	
	mov 5*8(x), tm
	mov 6*8(x), t0
	mov 7*8(x), t1
	mov 8*8(x), t2
	mov t2, t3
	shld $49, t1, t2
	shld $49, t0, t1
	shld $49, tm, t0
	shr $(64-49), t3
	
	xor ud, ud

	mov 0*8(y), u
	sbb t0,u
	mov u, 0*8(y)

	mov 1*8(y), u
	sbb t1,u
	mov u, 1*8(y)

	mov 2*8(y), u
	sbb t2,u
	mov u, 2*8(y)

	mov 3*8(y), u
	sbb t3,u
	mov u, 3*8(y)

	.set cnt, 4
	.rept 15
        mov cnt*8(y), u
	sbb $0, u
	mov u, cnt*8(y)
	.if cnt==4
	jae 4f
	.endif
	.set cnt, cnt + 1
	.endr
4:	
	ret
