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
	
.macro pushregs
	push %rbp
	push %rbx
	push %r12
	push %r13
	push %r14
	push %r15
.endm

.macro popregs
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbx
	pop %rbp
.endm
	
.macro step i j r
	mov 4*\j(x), \r
	sbb 4*\i(x), \r	
.endm

.macro step2 r i
	and m, \r
	mov \r, 4*\i(x)
.endm

.macro step3 i r
	sbb 4*\i(x), \r	
.endm

.set  x, %rdi
.set  c, %sil
.set  n, %rdx
.set  r0, %eax
.set  r1, %ebx
.set  r2, %ecx
.set  r3, %r13d
.set  r4, %r15d
.set  r5, %ebp
.set  r6, %r8d
.set  r7, %r9d
.set  r8, %r10d
.set  r9, %r11d
.set  r10, %r12d
.set  t0, %r13d
.set  m, %r14d

/*
unsigned char _skipstates(uint32_t *state, unsigned char carry, uint64_t nskip);
x=state=%rdi, c=carry=%rsi, n=nskip=%rdx
*/
	.globl _skipstates
_skipstates:
	pushregs

	mov $0xffffff, m

.L0:
	add $0xff, c
	step 23 9 r0
	step 22 8 r1
	step 21 7 r2
	step 20 6 r3
	step 19 5 r4
	step 18 4 r5
	step 17 3 r6
	step 16 2 r7
	step 15 1 r8
	step 14 0 r9
	setc c
	
	step2 r0 23
	step2 r1 22
	step2 r2 21
	step2 r3 20
	step2 r4 19
	step2 r5 18
	step2 r6 17
	step2 r7 16
	step2 r8 15
	step2 r9 14

	add $0xff, c
	step3 13 r0
	step3 12 r1
	step3 11 r2
	step3 10 r3
	step3  9 r4
	step3  8 r5
	step3  7 r6
	step3  6 r7
	step3  5 r8
	step3  4 r9
	setc c

	step2 r0 13
	step2 r1 12
	step2 r2 11
	step2 r3 10
	step2 r4  9
	step2 r5  8
	step2 r6  7
	step2 r7  6
	step2 r8  5
	step2 r9  4
	
	add $0xff, c
	step3 3 r0
	step3 2 r1
	step3 1 r2
	step3 0 r3
	setc c

	step2 r0 3
	step2 r1 2
	step2 r2 1
	step2 r3 0
	dec n
	jnz .L0
	
	mov c, %al

	popregs
        ret
