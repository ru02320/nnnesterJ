/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/*
 * This file contains a C and MMX implentation of the Scale2x effect.
 *
 * You can found an high level description of the effect at :
 *
 * http://scale2x.sourceforge.net/scale2x.html
 *
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 * - derivative works of the program are allowed.
 */

#ifndef __SCALE2X_H
#define __SCALE2X_H

#include <assert.h>

/***************************************************************************/
/* basic types */

typedef unsigned char scale2x_uint8;
typedef unsigned short scale2x_uint16;
typedef unsigned scale2x_uint32;

/***************************************************************************/

#if defined(__GNUC__) && defined(__i386__)

/*
 * Apply the Scale2x effect at a single row.
 * This function must be called only by the other scale2x functions.
 *
 * Considering the pixel map :
 *
 *      ABC (src0)
 *      DEF (src1)
 *      GHI (src2)
 *
 * this functions compute 2 new pixels in substitution of the source pixel E
 * like this map :
 *
 *      ab (dst)
 *
 * with these variables :
 *
 *      &current -> E
 *      &current_left -> D
 *      &current_right -> F
 *      &current_upper -> B
 *      &current_lower -> H
 *
 *      %0 -> current_upper
 *      %1 -> current
 *      %2 -> current_lower
 *      %3 -> dst
 *      %4 -> counter
 *
 *      %mm0 -> *current_left
 *      %mm1 -> *current_next
 *      %mm2 -> tmp0
 *      %mm3 -> tmp1
 *      %mm4 -> tmp2
 *      %mm5 -> tmp3
 *      %mm6 -> *current_upper
 *      %mm7 -> *current
 */
static __inline__ void scale2x_8_mmx_single(scale2x_uint8* dst, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	assert(count >= 16);
	assert(count % 8 == 0);

	/* always do the first and last run */
	count -= 2*8;

	__asm__ __volatile__(
/* first run */
		/* set the current, current_pre, current_next registers */
		"movq 0(%1),%%mm0\n"
		"movq 0(%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psllq $56,%%mm0\n"
		"psllq $56,%%mm1\n"
		"psrlq $56,%%mm0\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $8,%%mm2\n"
		"psrlq $8,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqb %%mm6,%%mm2\n"
		"pcmpeqb %%mm6,%%mm4\n"
		"pcmpeqb (%2),%%mm3\n"
		"pcmpeqb (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqb %%mm1,%%mm2\n"
		"pcmpeqb %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpcklbw %%mm4,%%mm2\n"
		"punpckhbw %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

/* central runs */
		"shrl $3,%4\n"
		"jz 1f\n"

		"0:\n"

		/* set the current, current_pre, current_next registers */
		"movq -8(%1),%%mm0\n"
		"movq (%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psrlq $56,%%mm0\n"
		"psllq $56,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $8,%%mm2\n"
		"psrlq $8,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqb %%mm6,%%mm2\n"
		"pcmpeqb %%mm6,%%mm4\n"
		"pcmpeqb (%2),%%mm3\n"
		"pcmpeqb (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqb %%mm1,%%mm2\n"
		"pcmpeqb %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpcklbw %%mm4,%%mm2\n"
		"punpckhbw %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

		"decl %4\n"
		"jnz 0b\n"
		"1:\n"

/* final run */
		/* set the current, current_pre, current_next registers */
		"movq (%1),%%mm1\n"
		"movq (%1),%%mm7\n"
		"movq -8(%1),%%mm0\n"
		"psrlq $56,%%mm1\n"
		"psrlq $56,%%mm0\n"
		"psllq $56,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $8,%%mm2\n"
		"psrlq $8,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqb %%mm6,%%mm2\n"
		"pcmpeqb %%mm6,%%mm4\n"
		"pcmpeqb (%2),%%mm3\n"
		"pcmpeqb (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqb %%mm1,%%mm2\n"
		"pcmpeqb %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpcklbw %%mm4,%%mm2\n"
		"punpckhbw %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		: "+r" (src0), "+r" (src1), "+r" (src2), "+r" (dst), "+r" (count)
		:
		: "cc"
	);
}

static __inline__ void scale2x_16_mmx_single(scale2x_uint16* dst, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	assert(count >= 8);
	assert(count % 4 == 0);

	/* always do the first and last run */
	count -= 2*4;

	__asm__ __volatile__(
/* first run */
		/* set the current, current_pre, current_next registers */
		"movq 0(%1),%%mm0\n"
		"movq 0(%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psllq $48,%%mm0\n"
		"psllq $48,%%mm1\n"
		"psrlq $48,%%mm0\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $16,%%mm2\n"
		"psrlq $16,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqw %%mm6,%%mm2\n"
		"pcmpeqw %%mm6,%%mm4\n"
		"pcmpeqw (%2),%%mm3\n"
		"pcmpeqw (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqw %%mm1,%%mm2\n"
		"pcmpeqw %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpcklwd %%mm4,%%mm2\n"
		"punpckhwd %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

/* central runs */
		"shrl $2,%4\n"
		"jz 1f\n"

		"0:\n"

		/* set the current, current_pre, current_next registers */
		"movq -8(%1),%%mm0\n"
		"movq (%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psrlq $48,%%mm0\n"
		"psllq $48,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $16,%%mm2\n"
		"psrlq $16,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqw %%mm6,%%mm2\n"
		"pcmpeqw %%mm6,%%mm4\n"
		"pcmpeqw (%2),%%mm3\n"
		"pcmpeqw (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqw %%mm1,%%mm2\n"
		"pcmpeqw %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpcklwd %%mm4,%%mm2\n"
		"punpckhwd %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

		"decl %4\n"
		"jnz 0b\n"
		"1:\n"

/* final run */
		/* set the current, current_pre, current_next registers */
		"movq (%1),%%mm1\n"
		"movq (%1),%%mm7\n"
		"movq -8(%1),%%mm0\n"
		"psrlq $48,%%mm1\n"
		"psrlq $48,%%mm0\n"
		"psllq $48,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $16,%%mm2\n"
		"psrlq $16,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqw %%mm6,%%mm2\n"
		"pcmpeqw %%mm6,%%mm4\n"
		"pcmpeqw (%2),%%mm3\n"
		"pcmpeqw (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqw %%mm1,%%mm2\n"
		"pcmpeqw %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpcklwd %%mm4,%%mm2\n"
		"punpckhwd %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		: "+r" (src0), "+r" (src1), "+r" (src2), "+r" (dst), "+r" (count)
		:
		: "cc"
	);
}

static __inline__ void scale2x_32_mmx_single(scale2x_uint32* dst, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	assert(count >= 4);
	assert(count % 2 == 0);

	/* always do the first and last run */
	count -= 2*2;

	__asm__ __volatile__(
/* first run */
		/* set the current, current_pre, current_next registers */
		"movq 0(%1),%%mm0\n"
		"movq 0(%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psllq $32,%%mm0\n"
		"psllq $32,%%mm1\n"
		"psrlq $32,%%mm0\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $32,%%mm2\n"
		"psrlq $32,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqd %%mm6,%%mm2\n"
		"pcmpeqd %%mm6,%%mm4\n"
		"pcmpeqd (%2),%%mm3\n"
		"pcmpeqd (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqd %%mm1,%%mm2\n"
		"pcmpeqd %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpckldq %%mm4,%%mm2\n"
		"punpckhdq %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

/* central runs */
		"shrl $1,%4\n"
		"jz 1f\n"

		"0:\n"

		/* set the current, current_pre, current_next registers */
		"movq -8(%1),%%mm0\n"
		"movq (%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psrlq $32,%%mm0\n"
		"psllq $32,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $32,%%mm2\n"
		"psrlq $32,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqd %%mm6,%%mm2\n"
		"pcmpeqd %%mm6,%%mm4\n"
		"pcmpeqd (%2),%%mm3\n"
		"pcmpeqd (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqd %%mm1,%%mm2\n"
		"pcmpeqd %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpckldq %%mm4,%%mm2\n"
		"punpckhdq %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

		"decl %4\n"
		"jnz 0b\n"
		"1:\n"

/* final run */
		/* set the current, current_pre, current_next registers */
		"movq (%1),%%mm1\n"
		"movq (%1),%%mm7\n"
		"movq -8(%1),%%mm0\n"
		"psrlq $32,%%mm1\n"
		"psrlq $32,%%mm0\n"
		"psllq $32,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $32,%%mm2\n"
		"psrlq $32,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst on %%mm2 */
		/* compute the upper-right pixel for dst on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqd %%mm6,%%mm2\n"
		"pcmpeqd %%mm6,%%mm4\n"
		"pcmpeqd (%2),%%mm3\n"
		"pcmpeqd (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqd %%mm1,%%mm2\n"
		"pcmpeqd %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst */
		"movq %%mm2,%%mm3\n"
		"punpckldq %%mm4,%%mm2\n"
		"punpckhdq %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		: "+r" (src0), "+r" (src1), "+r" (src2), "+r" (dst), "+r" (count)
		:
		: "cc"
	);
}

/**
 * Scale by a factor of 2 a row of pixels of 8 bits.
 * This is a very fast MMX implementation.
 * The implementation uses a combination of cmp/and/not operations to
 * completly remove the need of conditional jumps. This trick give the
 * major speed improvement.
 * Also, using the 8 bytes MMX registers more than one pixel are computed
 * at the same time.
 * Before calling this function you must ensure that the currenct CPU supports
 * the MMX instruction set. After calling it you must be sure to call the EMMS
 * instruction before any floating-point operation.
 * The pixels over the left and right borders are assumed of the same color of
 * the pixels on the border.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 16 and a multiple of 8.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_8_mmx(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count)
{
	assert(count >= 16);
	assert(count % 8 == 0);

	scale2x_8_mmx_single(dst0, src0, src1, src2, count);
	scale2x_8_mmx_single(dst1, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2 a row of pixels of 16 bits.
 * This function operates like scale2x_8_mmx() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 8 and a multiple of 4.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_16_mmx(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count)
{
	assert(count >= 8);
	assert(count % 4 == 0);

	scale2x_16_mmx_single(dst0, src0, src1, src2, count);
	scale2x_16_mmx_single(dst1, src2, src1, src0, count);
}

/**
 * Scale by a factor of 2 a row of pixels of 32 bits.
 * This function operates like scale2x_8_mmx() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 4 and a multiple of 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
void scale2x_32_mmx(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count)
{
	assert(count >= 4);
	assert(count % 2 == 0);

	scale2x_32_mmx_single(dst0, src0, src1, src2, count);
	scale2x_32_mmx_single(dst1, src2, src1, src0, count);
}

#endif


#endif