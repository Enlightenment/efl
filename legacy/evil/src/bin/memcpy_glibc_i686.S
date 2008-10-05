/* Copy memory block and return pointer to beginning of destination block
   For Intel 80x86, x>=6.
   This file is part of the GNU C Library.
   Copyright (C) 1999, 2000, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#   define CHECK_BOUNDS_BOTH_WIDE(VAL_REG, BP_MEM, LENGTH)	\
	CHECK_BOUNDS_LOW(VAL_REG, BP_MEM);			\
	addl LENGTH, VAL_REG;					\
    	cmpl 8+BP_MEM, VAL_REG;					\
	jbe 0f; /* continue if value <= high */			\
	BOUNDS_VIOLATED;					\
    0:	subl LENGTH, VAL_REG /* restore value */

#   define RETURN_BOUNDED_POINTER(BP_MEM)	\
	movl RTN(%esp), %edx;			\
	movl %eax, 0(%edx);			\
	movl 4+BP_MEM, %eax;			\
	movl %eax, 4(%edx);			\
	movl 8+BP_MEM, %eax;			\
	movl %eax, 8(%edx)

#define PTR_SIZE 12
#define RTN_SIZE 4
#define LINKAGE 8

#define PARMS	LINKAGE		/* no space for saved regs */
#define RTN	PARMS
#define DEST	RTN+RTN_SIZE
#define SRC	DEST+PTR_SIZE
#define LEN	SRC+PTR_SIZE

	.text

	.align
        .global memcpy_glibc
        .func   memcpy_glibc
memcpy_glibc:

	pushl %ebp
        movl %esp, %ebp

	movl	LEN(%esp), %ecx
	movl	%edi, %eax
	movl	DEST(%esp), %edi
	movl	%esi, %edx
	movl	SRC(%esp), %esi

	cld
	shrl	$1, %ecx
	jnc	1f
	movsb
1:	shrl	$1, %ecx
	jnc	2f
	movsw
2:	rep
	movsl
	movl	%eax, %edi
	movl	%edx, %esi
	movl	DEST(%esp), %eax
	RETURN_BOUNDED_POINTER (DEST(%esp))

	movl %ebp, %esp
        popl %ebp

.endfunc
