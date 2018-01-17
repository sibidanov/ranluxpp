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
	
	.globl _mul9x9_mul
_mul9x9_mul:
/*
b array of 18
a array of 9
void _mul9x9_mul(uint64_t *b, const uint64_t *a);

b = b*a by schoolbook method

only mul instruction is used for 64x64 bit multiplication
b=%rdi, a=%rsi
*/
	
.set   b, %rdi
.set out, %rdi
.set   a, %rsi
.set  op, %rbx
.set  t0, %rcx
.set  r0, %rbp
.set  r1, %r8
.set  r2, %r9
.set  r3, %r10
.set  r4, %r11
.set  r5, %r12
.set  r6, %r13
.set  r7, %r14
.set  r8, %r15

	pushregs
	
	mov 0x0(b), op
	mov 0x0(a), %rax
	mul op
	mov %rax, 0x0(b)
	mov %rdx, r0
	
.macro introstep in o0 o1
	mov \in(a), %rax
	mul  op
	add %rax, \o0
	adc $0x0, %rdx
	mov %rdx, \o1
.endm
	
	introstep 0x8  r0 r1
	introstep 0x10 r1 r2
	introstep 0x18 r2 r3
	introstep 0x20 r3 r4
	introstep 0x28 r4 r5
	introstep 0x30 r5 r6
	introstep 0x38 r6 r7
	introstep 0x40 r7 r8

.macro oldsubstep uff R0 R1 TMP
	mov \uff(a), %rax
	mul op
	mov \TMP, \R0
	mov %rdx, \TMP
	add  \R1, \R0
	adc   $0, \TMP
	add %rax, \R0
	adc   $0, \TMP
.endm

.macro oldstep off
	mov \off(b), op

	mov 0x0(a), %rax
	mul op
	add %rax, r0
	adc $0, %rdx
	mov %rdx, t0
	mov r0, \off(out)

	oldsubstep 0x8  r0 r1 t0
	oldsubstep 0x10 r1 r2 t0
	oldsubstep 0x18 r2 r3 t0
	oldsubstep 0x20 r3 r4 t0
	oldsubstep 0x28 r4 r5 t0
	oldsubstep 0x30 r5 r6 t0
	oldsubstep 0x38 r6 r7 t0
	oldsubstep 0x40 r7 r8 t0
	mov t0, r8
.endm

	oldstep 0x8 
	oldstep 0x10
	oldstep 0x18
	oldstep 0x20
	oldstep 0x28
	oldstep 0x30
	oldstep 0x38
	oldstep 0x40

	mov r0, 0x48(out)
	mov r1, 0x50(out)
	mov r2, 0x58(out)
	mov r3, 0x60(out)
	mov r4, 0x68(out)
	mov r5, 0x70(out)
	mov r6, 0x78(out)
	mov r7, 0x80(out)
	mov r8, 0x88(out)

	popregs
        ret
	
	.globl _mul9x9_mulx
_mul9x9_mulx:
/*
out array of 18
b   array of 9
a   array of 9

void _mul9x9mulx(uint64_t *out, const uint64_t *a, const uint64_t *b);

out = a*b by schoolbook method

mulx instruction is used for 64x64 bit multiplication

out=%rdi, a=%rsi, b=%rdx
*/

.set   b, %r8
.set   a, %rsi
.set  t1, %r10
.set  t0, %r11
.set out, %rdi
.set  r0, %rax
.set  r1, %rbx
.set  r2, %rcx
.set  r3, %r15
.set  r4, %rbp
.set  r5, %r14
.set  r6, %r9
.set  r7, %r12
.set  r8, %r13

	pushregs

	mov %rdx, b
	mov   0(b), %rdx

	mulx  0(a), t1, r0
	mov t1, (out)

	mulx  8(a), t1, r1
	add t1, r0

.macro mstep in o0 o1
	mulx  \in(a), t1, \o0
	adc t1, \o1
.endm

	mstep 0x10 r2 r1
	mstep 0x18 r3 r2
	mstep 0x20 r4 r3
	mstep 0x28 r5 r4
	mstep 0x30 r6 r5
	mstep 0x38 r7 r6
	mstep 0x40 r8 r7
	adc $0, r8

.macro substep off reg regn tmp0 tmp1
	mulx  \off(a), \reg, \tmp0
	add   \regn, \reg
	adc   $0, \tmp0
	add   \tmp1, \reg
	adc   $0, \tmp0
.endm

.macro step off
	mov   \off(b), %rdx
	
	mulx  0(a), t0, t1
	add   t0, r0
	adc   $0, t1
	mov   r0, \off(out)

	substep 0x8  r0 r1 t0 t1
	substep 0x10 r1 r2 t1 t0
	substep 0x18 r2 r3 t0 t1
	substep 0x20 r3 r4 t1 t0
	substep 0x28 r4 r5 t0 t1
	substep 0x30 r5 r6 t1 t0
	substep 0x38 r6 r7 t0 t1
	substep 0x40 r7 r8 t1 t0
	mov   t1, r8
.endm

	step 0x8 
	step 0x10
	step 0x18
	step 0x20
	step 0x28
	step 0x30
	step 0x38
	step 0x40
	
	mov r0, 0x48(out)
	mov r1, 0x50(out)
	mov r2, 0x58(out)
	mov r3, 0x60(out)
	mov r4, 0x68(out)
	mov r5, 0x70(out)
	mov r6, 0x78(out)
	mov r7, 0x80(out)
	mov r8, 0x88(out)

	popregs
        ret

/*
out array of 18
b   array of 9
a   array of 9

void _mul9x9_mulxadox(uint64_t *out, const uint64_t *a, const uint64_t *b);

out = a*b by schoolbook method

mulx instruction is used for 64x64 bit multiplication and adcx and adxo for carry propagation

out=%rdi, a=%rsi, b=%rdx
*/
	.globl _mul9x9_mulxadox
_mul9x9_mulxadox:
	pushregs

	mov %rdx, b
	mov   0(b), %rdx

	mulx  0(a), t1, r0
	mov t1, (out)

	mulx  8(a), t1, r1
	add t1, r0

	mstep 0x10 r2 r1
	mstep 0x18 r3 r2
	mstep 0x20 r4 r3
	mstep 0x28 r5 r4
	mstep 0x30 r6 r5
	mstep 0x38 r7 r6
	mstep 0x40 r8 r7
	adc $0, r8

.macro adoxsubstep off reg0 reg1 reg2 tmp
	mulx \off(a), \reg0, \tmp
	adox \reg1, \reg0
	adcx \tmp, \reg2
.endm

.macro adoxstep off
	xor t0, t0
	mov \off(b), %rdx
	
	mulx 0x0(a), t0, t1
	adox t0, r0
	adcx t1, r1
	mov  r0, \off(out)

	adoxsubstep 0x8  r0 r1 r2 t1
	adoxsubstep 0x10 r1 r2 r3 t1
	adoxsubstep 0x18 r2 r3 r4 t1
	adoxsubstep 0x20 r3 r4 r5 t1
	adoxsubstep 0x28 r4 r5 r6 t1
	adoxsubstep 0x30 r5 r6 r7 t1
	adoxsubstep 0x38 r6 r7 r8 t1
	mulx 0x40(a), r7, t1
	adox r8, r7
	seto %r13b
	movzx %r13b, %r13
	adc  t1, r8
.endm
	adoxstep 0x8 
	adoxstep 0x10
	adoxstep 0x18
	adoxstep 0x20
	adoxstep 0x28
	adoxstep 0x30
	adoxstep 0x38
	adoxstep 0x40
	
	mov r0, 0x48(out)
	mov r1, 0x50(out)
	mov r2, 0x58(out)
	mov r3, 0x60(out)
	mov r4, 0x68(out)
	mov r5, 0x70(out)
	mov r6, 0x78(out)
	mov r7, 0x80(out)
	mov r8, 0x88(out)

	popregs
        ret

	.globl _remainder
_remainder:
/*
b  array of 18
void _remainder(uint64_t *b);

b = b % (2^(24*24)-2^(24*10)+1)

b = %rdi 
	
=>0 h[ 0] + h[14]
    h[ 1] + h[15]
    h[ 2] + h[16]
    h[ 3] + h[17]
    h[ 4] + h[18]
    h[ 5] + h[19]
    h[ 6] + h[20]
    h[ 7] + h[21]
=>3 h[ 8] + h[22]
    h[ 9] + h[23]
    h[10] -(h[ 0] + h[14])
    h[11] -(h[ 1] + h[15])
    h[12] -(h[ 2] + h[16])
    h[13] -(h[ 3] + h[17])
    h[14] -(h[ 4] + h[18])
    h[15] -(h[ 5] + h[19])
=>6 h[16] -(h[ 6] + h[20])
    h[17] -(h[ 7] + h[21])
    h[18] -(h[ 8] + h[22])
    h[19] -(h[ 9] + h[23])
    h[20] - h[10]
    h[21] - h[11]
    h[22] - h[12]
    h[23] - h[13]
*/

.set   b, %rdi
.set  t0, %rsi
.set  t1, %rax
.set  t2, %rbx
.set  t3, %rcx
.set  cr, %rdx
.set  r0, %rbp
.set  r1, %r8
.set  r2, %r9
.set  r3, %r10
.set  r4, %r11
.set  r5, %r12
.set  r6, %r13
.set  r7, %r14
.set  r8, %r15

	pushregs
	sub $0x60, %rsp

	mov 0x48+0x00(b), r0
	mov 0x48+0x08(b), r1
	mov 0x48+0x10(b), r2
	mov 0x48+0x18(b), r3
	mov 0x48+0x20(b), r4
	mov 0x48+0x28(b), r5
	mov 0x48+0x30(b), r6
	mov 0x48+0x38(b), r7
	mov 0x48+0x40(b), r8

	mov r5, t0
	shrd $16, r6, t0
	mov t0, 0x00(%rsp)
	
	mov r6, t0
	shrd $16, r7, t0
	mov t0, 0x08(%rsp)
	
	mov r7, t0
	shrd $16, r8, t0
	mov t0, 0x10(%rsp)
	
	mov r8, t0
	shr $16, t0
	mov t0, 0x18(%rsp)
/* t_2 = [rsp,rsp+8,rsp+10,rsp+18]*/
	mov $((1<<16)-1), t0
	mov t0, cr
	mov t0, t1
	not cr
	mov cr, t2
	and r5, t0
	
	add 0x00(%rsp), r0
	adc 0x08(%rsp), r1
	adc 0x10(%rsp), r2
	adc 0x18(%rsp), r3
	adc $0, r4
	adc $0, t0
/* enough space in t0, so carry is (t0>>16) */
/* t_3 + t_2 = [r0,r1,r2,r3,r4,t0] */
	and t0, cr
/* select carry bit */
	and t1, t0
	and t2, r5
	or  t0, r5
	mov cr, t1
	shr $16, cr
/* select carry bit */
	add t1, r5
	adc $0, r6
	adc $0, r7
	adc $0, r8
/* t_1 + t_2 = [r0,r1,r2,r3,r4,r5,r6,r7,r8] */
	sbb $0, cr
	
	mov r0, t2
	shl $48, t2
	mov t2, 0x00(%rsp)

	mov r1, t2
	shld $48, r0, t2
	mov t2, 0x08(%rsp)

	mov r2, t2
	shld $48, r1, t2
	mov t2, 0x10(%rsp)

	mov r3, t2
	shld $48, r2, t2
	mov t2, 0x18(%rsp)

	mov r4, t2
	shld $48, r3, t2
	mov t2, 0x20(%rsp)

	shld $48, r4, t0

/* (t_3 + t_2)*b^10 = [sp,sp+8,sp+10,sp+18,sp+20,t0] */
	
	sub 0x00(%rsp), r3
	sbb 0x08(%rsp), r4
	sbb 0x10(%rsp), r5
	sbb 0x18(%rsp), r6
	sbb 0x20(%rsp), r7
	sbb t0, r8
/* (t_1 + t_2) - (t_3 + t_2)*b^10 = [r0,r1,r2,r3,r4,r5,r6,r7,r8] */
	
	adc $0, cr

	mov 0x00(b), t0
	mov 0x08(b), t1
	mov 0x10(b), t2
	mov 0x18(b), t3

	sub r0, t0
	sbb r1, t1
	sbb r2, t2
	sbb r3, t3

	mov 0x20(b), r0
	mov 0x28(b), r1
	mov 0x30(b), r2
	mov 0x38(b), r3
	
	sbb r4, r0
	sbb r5, r1
	sbb r6, r2
	sbb r7, r3

	mov 0x40(b), r4
	sbb r8, r4
/* t_0 - ((t_1 + t_2) - (t_3 + t_2)*b^10) = [t0,t1,t2,t3,r0,r1,r2,r3,r4] */

	sbb $0, cr
/*
cr       - carry,
t0,t1,t2 - temporary storage
r0 r1 r2 r3 r4 r5 r6 r7 r8 - adjusting number = r - c*m
*/
.macro adjust cr t0 t1 t2 r0 r1 r2 r3 r4 r5 r6 r7 r8
	mov \cr, \t0
	mov \cr, \t1
	sar $63, \t0 
	shl $48, \t1
	mov \t0, \t2
	sub \t1, \t2
	mov \t2, \t1
	sar $63, \t1

	sub \cr, \r0
	sbb \t0, \r1
	sbb \t0, \r2
	sbb \t2, \r3
	sbb \t1, \r4
	sbb \t1, \r5
	sbb \t1, \r6
	sbb \t1, \r7
	sbb \t1, \r8
.endm

	adjust cr   r5 r6 r7    t0 t1 t2 t3 r0 r1 r2 r3 r4

.macro dump a r0 r1 r2 r3 r4 r5 r6 r7 r8
	mov \r0, 0x00(\a)
	mov \r1, 0x08(\a)
	mov \r2, 0x10(\a)
	mov \r3, 0x18(\a)
	mov \r4, 0x20(\a)
	mov \r5, 0x28(\a)
	mov \r6, 0x30(\a)
	mov \r7, 0x38(\a)
	mov \r8, 0x40(\a)
.endm

	dump b t0 t1 t2 t3 r0 r1 r2 r3 r4
	
	add $0x60, %rsp
	popregs
        ret

	.globl _mul9x9mod_mulx
_mul9x9mod_mulx:
/*
out array of 9
b   array of 9
a   array of 9
  
out = a*b mod (2^(24*24)-2^(24*10)+1)

void _mul9x9mod_mulx(uint64_t *out, const uint64_t *a, const uint64_t *b);

use mulx instruction
*/
	
.set out, %rdi
.set   a, %rsi
.set   b, %rcx
.set  t0, %rax
.set  t1, %rbx
.set  r0, %rbp
.set  r1, %r8
.set  r2, %r9
.set  r3, %r10
.set  r4, %r11
.set  r5, %r12
.set  r6, %r13
.set  r7, %r14
.set  r8, %r15
	pushregs

	mov %rdx, b
	mov   0x0(b), %rdx

	mulx  0x0(a), t1, r0
	mov t1, (out)

	mulx  0x8(a), t1, r1
	add t1, r0

	mstep 0x10 r2 r1
	mstep 0x18 r3 r2
	mstep 0x20 r4 r3
	mstep 0x28 r5 r4
	mstep 0x30 r6 r5
	mstep 0x38 r7 r6
	mstep 0x40 r8 r7
	adc $0, r8

	step 0x8 
	step 0x10
	step 0x18
	step 0x20
	step 0x28
	step 0x30
	step 0x38
	step 0x40
	
	sub $0x60, %rsp

.set   b, %rdi
.set  t0, %rsi
.set  t1, %rax
.set  t2, %rbx
.set  t3, %rcx
.set  cr, %rdx

	mov r5, t0
	shrd $16, r6, t0
	mov t0, 0x00(%rsp)
	
	mov r6, t0
	shrd $16, r7, t0
	mov t0, 0x08(%rsp)
	
	mov r7, t0
	shrd $16, r8, t0
	mov t0, 0x10(%rsp)
	
	mov r8, t0
	shr $16, t0
	mov t0, 0x18(%rsp)
/* t_2 = [rsp,rsp+8,rsp+10,rsp+18]*/
	mov $((1<<16)-1), t0
	mov t0, cr
	mov t0, t1
	not cr
	mov cr, t2
	and r5, t0
	
	add 0x00(%rsp), r0
	adc 0x08(%rsp), r1
	adc 0x10(%rsp), r2
	adc 0x18(%rsp), r3
	adc $0, r4
	adc $0, t0
/* enough space in t0, so carry is (t0>>16) */
/* t_3 + t_2 = [r0,r1,r2,r3,r4,t0] */
	and t0, cr
/* select carry bit */
	and t1, t0
	and t2, r5
	or  t0, r5
	mov cr, t1
	shr $16, cr
/* select carry bit */
	add t1, r5
	adc $0, r6
	adc $0, r7
	adc $0, r8
/* t_1 + t_2 = [r0,r1,r2,r3,r4,r5,r6,r7,r8] */
	sbb $0, cr
	
	mov r0, t2
	shl $48, t2
	mov t2, 0x00(%rsp)

	mov r1, t2
	shld $48, r0, t2
	mov t2, 0x08(%rsp)

	mov r2, t2
	shld $48, r1, t2
	mov t2, 0x10(%rsp)

	mov r3, t2
	shld $48, r2, t2
	mov t2, 0x18(%rsp)

	mov r4, t2
	shld $48, r3, t2
	mov t2, 0x20(%rsp)

	shld $48, r4, t0

/* (t_3 + t_2)*b^10 = [sp,sp+8,sp+10,sp+18,sp+20,t0] */

	sub 0x00(%rsp), r3
	sbb 0x08(%rsp), r4
	sbb 0x10(%rsp), r5
	sbb 0x18(%rsp), r6
	sbb 0x20(%rsp), r7
	sbb t0, r8
/* (t_1 + t_2) - (t_3 + t_2)*b^10 = [r0,r1,r2,r3,r4,r5,r6,r7,r8] */
	
	adc $0, cr

	mov 0x00(b), t0
	mov 0x08(b), t1
	mov 0x10(b), t2
	mov 0x18(b), t3

	sub r0, t0
	sbb r1, t1
	sbb r2, t2
	sbb r3, t3

	mov 0x20(b), r0
	mov 0x28(b), r1
	mov 0x30(b), r2
	mov 0x38(b), r3
	
	sbb r4, r0
	sbb r5, r1
	sbb r6, r2
	sbb r7, r3

	mov 0x40(b), r4
	sbb r8, r4
/* t_0 - ((t_1 + t_2) - (t_3 + t_2)*b^10) = [t0,t1,t2,t3,r0,r1,r2,r3,r4] */
	sbb $0, cr

	adjust cr   r5 r6 r7    t0 t1 t2 t3 r0 r1 r2 r3 r4

	dump out t0 t1 t2 t3 r0 r1 r2 r3 r4
	
	add $0x60, %rsp
	
	popregs
        ret

	.globl _mul9x9mod_mulxadox
_mul9x9mod_mulxadox:
/*
out array of 9
b   array of 9
a   array of 9
  
out = a*b mod (2^(24*24)-2^(24*10)+1)

void _mul9x9mod_mulxadox(uint64_t *out, const uint64_t *a, const uint64_t *b);

use mulx and adox instructions 
*/
	
.set out, %rdi
.set   a, %rsi
.set   b, %rcx
.set t0, %rax
.set t1, %rbx
.set  r0, %rbp
.set  r1, %r8
.set  r2, %r9
.set  r3, %r10
.set  r4, %r11
.set  r5, %r12
.set  r6, %r13
.set  r7, %r14
.set  r8, %r15
	pushregs

	mov %rdx, b
	mov   0(b), %rdx

	mulx  0(a), t1, r0
	mov t1, (out)

	mulx  8(a), t1, r1
	add t1, r0

	mstep 0x10 r2 r1
	mstep 0x18 r3 r2
	mstep 0x20 r4 r3
	mstep 0x28 r5 r4
	mstep 0x30 r6 r5
	mstep 0x38 r7 r6
	mstep 0x40 r8 r7
	adc $0, r8

.macro adoxstep2 off
	xor t0, t0
	mov \off(b), %rdx
	
	mulx 0x0(a), t0, t1
	adox t0, r0
	adcx t1, r1
	mov  r0, \off(out)

	adoxsubstep 0x8  r0 r1 r2 t1
	adoxsubstep 0x10 r1 r2 r3 t1
	adoxsubstep 0x18 r2 r3 r4 t1
	adoxsubstep 0x20 r3 r4 r5 t1
	adoxsubstep 0x28 r4 r5 r6 t1
	adoxsubstep 0x30 r5 r6 r7 t1
	adoxsubstep 0x38 r6 r7 r8 t1
	mulx 0x40(a), r7, t1
	adox r8, r7
	seto %r15b
	movzx %r15b, %r15
	adc  t1, r8
.endm
	adoxstep2 0x8 
	adoxstep2 0x10
	adoxstep2 0x18
	adoxstep2 0x20
	adoxstep2 0x28
	adoxstep2 0x30
	adoxstep2 0x38
	adoxstep2 0x40
	
	sub $0x60, %rsp

.set   b, %rdi
.set  t0, %rsi
.set  t1, %rax
.set  t2, %rbx
.set  t3, %rcx
.set  cr, %rdx

	mov r5, t0
	shrd $16, r6, t0
	mov t0, 0x00(%rsp)
	
	mov r6, t0
	shrd $16, r7, t0
	mov t0, 0x08(%rsp)
	
	mov r7, t0
	shrd $16, r8, t0
	mov t0, 0x10(%rsp)
	
	mov r8, t0
	shr $16, t0
	mov t0, 0x18(%rsp)
/* t_2 = [rsp,rsp+8,rsp+10,rsp+18]*/
	mov $((1<<16)-1), t0
	mov t0, cr
	mov t0, t1
	not cr
	mov cr, t2
	and r5, t0
	
	add 0x00(%rsp), r0
	adc 0x08(%rsp), r1
	adc 0x10(%rsp), r2
	adc 0x18(%rsp), r3
	adc $0, r4
	adc $0, t0
/* enough space in t0, so carry is (t0>>16) */
/* t_3 + t_2 = [r0,r1,r2,r3,r4,t0] */
	and t0, cr
/* select carry bit */
	and t1, t0
	and t2, r5
	or  t0, r5
	mov cr, t1
	shr $16, cr
/* select carry bit */
	add t1, r5
	adc $0, r6
	adc $0, r7
	adc $0, r8
/* t_1 + t_2 = [r0,r1,r2,r3,r4,r5,r6,r7,r8] */
	sbb $0, cr
	
	mov r0, t2
	shl $48, t2
	mov t2, 0x00(%rsp)

	mov r1, t2
	shld $48, r0, t2
	mov t2, 0x08(%rsp)

	mov r2, t2
	shld $48, r1, t2
	mov t2, 0x10(%rsp)

	mov r3, t2
	shld $48, r2, t2
	mov t2, 0x18(%rsp)

	mov r4, t2
	shld $48, r3, t2
	mov t2, 0x20(%rsp)

	shld $48, r4, t0

/* (t_3 + t_2)*b^10 = [sp,sp+8,sp+10,sp+18,sp+20,t0] */

	sub 0x00(%rsp), r3
	sbb 0x08(%rsp), r4
	sbb 0x10(%rsp), r5
	sbb 0x18(%rsp), r6
	sbb 0x20(%rsp), r7
	sbb t0, r8
/* (t_1 + t_2) - (t_3 + t_2)*b^10 = [r0,r1,r2,r3,r4,r5,r6,r7,r8] */
	
	adc $0, cr

	mov 0x00(b), t0
	mov 0x08(b), t1
	mov 0x10(b), t2
	mov 0x18(b), t3

	sub r0, t0
	sbb r1, t1
	sbb r2, t2
	sbb r3, t3

	mov 0x20(b), r0
	mov 0x28(b), r1
	mov 0x30(b), r2
	mov 0x38(b), r3
	
	sbb r4, r0
	sbb r5, r1
	sbb r6, r2
	sbb r7, r3

	mov 0x40(b), r4
	sbb r8, r4
/* t_0 - ((t_1 + t_2) - (t_3 + t_2)*b^10) = [t0,t1,t2,t3,r0,r1,r2,r3,r4] */
	sbb $0, cr

	adjust cr   r5 r6 r7    t0 t1 t2 t3 r0 r1 r2 r3 r4

	dump out t0 t1 t2 t3 r0 r1 r2 r3 r4
	
	add $0x60, %rsp
	
	popregs
        ret

