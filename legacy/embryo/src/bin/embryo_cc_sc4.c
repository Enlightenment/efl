/*  Small compiler - code generation (unoptimized "assembler" code)
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Version: $Id$
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>		/* for _MAX_PATH */
#include <string.h>

#include "embryo_cc_sc.h"

/* When a subroutine returns to address 0, the AMX must halt. In earlier
 * releases, the RET and RETN opcodes checked for the special case 0 address.
 * Today, the compiler simply generates a HALT instruction at address 0. So
 * a subroutine can savely return to 0, and then encounter a HALT.
 */
void
writeleader(void)
{
   assert(code_idx == 0);
   stgwrite(";program exit point\n");
   stgwrite("\thalt 0\n");
   /* calculate code length */
   code_idx += opcodes(1) + opargs(1);
}

/*  writetrailer
 *  Not much left of this once important function.
 *
 *  Global references: sc_stksize       (referred to only)
 *                     sc_dataalign     (referred to only)
 *                     code_idx         (altered)
 *                     glb_declared     (altered)
 */
void
writetrailer(void)
{
   assert(sc_dataalign % opcodes(1) == 0);	/* alignment must be a multiple of
						 * the opcode size */
   assert(sc_dataalign != 0);

   /* pad code to align data segment */
   if ((code_idx % sc_dataalign) != 0)
     {
	begcseg();
	while ((code_idx % sc_dataalign) != 0)
	   nooperation();
     }				/* if */

   /* pad data segment to align the stack and the heap */
   assert(litidx == 0);		/* literal queue should have been emptied */
   assert(sc_dataalign % sizeof(cell) == 0);
   if (((glb_declared * sizeof(cell)) % sc_dataalign) != 0)
     {
	begdseg();
	defstorage();
	while (((glb_declared * sizeof(cell)) % sc_dataalign) != 0)
	  {
	     stgwrite("0 ");
	     glb_declared++;
	  }			/* while */
     }				/* if */

   stgwrite("\nSTKSIZE ");	/* write stack size (align stack top) */
   outval(sc_stksize - (sc_stksize % sc_dataalign), TRUE);
}

/*
 *  Start (or restart) the CODE segment.
 *
 *  In fact, the code and data segment specifiers are purely informational;
 *  the "DUMP" instruction itself already specifies that the following values
 *  should go to the data segment. All otherinstructions go to the code
 *  segment.
 *
 *  Global references: curseg
 */
void
begcseg(void)
{
   if (curseg != sIN_CSEG)
     {
	stgwrite("\n");
	stgwrite("CODE\t; ");
	outval(code_idx, TRUE);
	curseg = sIN_CSEG;
     }				/* endif */
}

/*
 *  Start (or restart) the DATA segment.
 *
 *  Global references: curseg
 */
void
begdseg(void)
{
   if (curseg != sIN_DSEG)
     {
	stgwrite("\n");
	stgwrite("DATA\t; ");
	outval(glb_declared - litidx, TRUE);
	curseg = sIN_DSEG;
     }				/* if */
}

void
setactivefile(int fnumber)
{
   stgwrite("curfile ");
   outval(fnumber, TRUE);
}

cell
nameincells(char *name)
{
   cell                clen =
      (strlen(name) + sizeof(cell)) & ~(sizeof(cell) - 1);
   return clen;
}

void
setfile(char *name, int fileno)
{
   if ((sc_debug & sSYMBOLIC) != 0)
     {
	begcseg();
	stgwrite("file ");
	outval(fileno, FALSE);
	stgwrite(" ");
	stgwrite(name);
	stgwrite("\n");
	/* calculate code length */
	code_idx += opcodes(1) + opargs(2) + nameincells(name);
     }				/* if */
}

void
setline(int line, int fileno)
{
   if ((sc_debug & (sSYMBOLIC | sCHKBOUNDS)) != 0)
     {
	stgwrite("line ");
	outval(line, FALSE);
	stgwrite(" ");
	outval(fileno, FALSE);
	stgwrite("\t; ");
	outval(code_idx, TRUE);
	code_idx += opcodes(1) + opargs(2);
     }				/* if */
}

/*  setlabel
 *
 *  Post a code label (specified as a number), on a new line.
 */
void
setlabel(int number)
{
   assert(number >= 0);
   stgwrite("l.");
   stgwrite((char *)itoh(number));
   /* To assist verification of the assembled code, put the address of the
    * label as a comment. However, labels that occur inside an expression
    * may move (through optimization or through re-ordering). So write the
    * address only if it is known to accurate.
    */
   if (!staging)
     {
	stgwrite("\t\t; ");
	outval(code_idx, FALSE);
     }				/* if */
   stgwrite("\n");
}

/* Write a token that signifies the end of an expression, or the end of a
 * function parameter. This allows several simple optimizations by the peephole
 * optimizer.
 */
void
endexpr(int fullexpr)
{
   if (fullexpr)
      stgwrite("\t;$exp\n");
   else
      stgwrite("\t;$par\n");
}

/*  startfunc   - declare a CODE entry point (function start)
 *
 *  Global references: funcstatus  (referred to only)
 */
void
startfunc(char *fname __UNUSED__)
{
   stgwrite("\tproc");
   stgwrite("\n");
   code_idx += opcodes(1);
}

/*  endfunc
 *
 *  Declare a CODE ending point (function end)
 */
void
endfunc(void)
{
   stgwrite("\n");		/* skip a line */
}

/*  alignframe
 *
 *  Aligns the frame (and the stack) of the current function to a multiple
 *  of the specified byte count. Two caveats: the alignment ("numbytes") should
 *  be a power of 2, and this alignment must be done right after the frame
 *  is set up (before the first variable is declared)
 */
void
alignframe(int numbytes)
{
#if !defined NDEBUG
   /* "numbytes" should be a power of 2 for this code to work */
   int                 i, count = 0;

   for (i = 0; i < sizeof numbytes * 8; i++)
      if (numbytes & (1 << i))
	 count++;
   assert(count == 1);
#endif

   stgwrite("\tlctrl 4\n");	/* get STK in PRI */
   stgwrite("\tconst.alt ");	/* get ~(numbytes-1) in ALT */
   outval(~(numbytes - 1), TRUE);
   stgwrite("\tand\n");		/* PRI = STK "and" ~(numbytes-1) */
   stgwrite("\tsctrl 4\n");	/* set the new value of STK ... */
   stgwrite("\tsctrl 5\n");	/* ... and FRM */
   code_idx += opcodes(5) + opargs(4);
}

/*  Define a variable or function
 */
void
defsymbol(char *name, int ident, int vclass, cell offset, int tag)
{
   if ((sc_debug & sSYMBOLIC) != 0)
     {
	begcseg();		/* symbol definition in code segment */
	stgwrite("symbol ");

	stgwrite(name);
	stgwrite(" ");

	outval(offset, FALSE);
	stgwrite(" ");

	outval(vclass, FALSE);
	stgwrite(" ");

	outval(ident, TRUE);

	code_idx += opcodes(1) + opargs(3) + nameincells(name);	/* class and ident encoded in "flags" */

	/* also write the optional tag */
	if (tag != 0)
	  {
	     assert((tag & TAGMASK) != 0);
	     stgwrite("symtag ");
	     outval(tag & TAGMASK, TRUE);
	     code_idx += opcodes(1) + opargs(1);
	  }			/* if */
     }				/* if */
}

void
symbolrange(int level, cell size)
{
   if ((sc_debug & sSYMBOLIC) != 0)
     {
	begcseg();		/* symbol definition in code segment */
	stgwrite("srange ");
	outval(level, FALSE);
	stgwrite(" ");
	outval(size, TRUE);
	code_idx += opcodes(1) + opargs(2);
     }				/* if */
}

/*  rvalue
 *
 *  Generate code to get the value of a symbol into "primary".
 */
void
rvalue(value * lval)
{
   symbol             *sym;

   sym = lval->sym;
   if (lval->ident == iARRAYCELL)
     {
	/* indirect fetch, address already in PRI */
	stgwrite("\tload.i\n");
	code_idx += opcodes(1);
     }
   else if (lval->ident == iARRAYCHAR)
     {
	/* indirect fetch of a character from a pack, address already in PRI */
	stgwrite("\tlodb.i ");
	outval(charbits / 8, TRUE);	/* read one or two bytes */
	code_idx += opcodes(1) + opargs(1);
     }
   else if (lval->ident == iREFERENCE)
     {
	/* indirect fetch, but address not yet in PRI */
	assert(sym != NULL);
	assert(sym->vclass == sLOCAL);	/* global references don't exist in Small */
	if (sym->vclass == sLOCAL)
	   stgwrite("\tlref.s.pri ");
	else
	   stgwrite("\tlref.pri ");
	outval(sym->addr, TRUE);
	markusage(sym, uREAD);
	code_idx += opcodes(1) + opargs(1);
     }
   else
     {
	/* direct or stack relative fetch */
	assert(sym != NULL);
	if (sym->vclass == sLOCAL)
	   stgwrite("\tload.s.pri ");
	else
	   stgwrite("\tload.pri ");
	outval(sym->addr, TRUE);
	markusage(sym, uREAD);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*
 *  Get the address of a symbol into the primary register (used for arrays,
 *  and for passing arguments by reference).
 */
void
address(symbol * sym)
{
   assert(sym != NULL);
   /* the symbol can be a local array, a global array, or an array
    * that is passed by reference.
    */
   if (sym->ident == iREFARRAY || sym->ident == iREFERENCE)
     {
	/* reference to a variable or to an array; currently this is
	 * always a local variable */
	stgwrite("\tload.s.pri ");
     }
   else
     {
	/* a local array or local variable */
	if (sym->vclass == sLOCAL)
	   stgwrite("\taddr.pri ");
	else
	   stgwrite("\tconst.pri ");
     }				/* if */
   outval(sym->addr, TRUE);
   markusage(sym, uREAD);
   code_idx += opcodes(1) + opargs(1);
}

/*  store
 *
 *  Saves the contents of "primary" into a memory cell, either directly
 *  or indirectly (at the address given in the alternate register).
 */
void
store(value * lval)
{
   symbol             *sym;

   sym = lval->sym;
   if (lval->ident == iARRAYCELL)
     {
	/* store at address in ALT */
	stgwrite("\tstor.i\n");
	code_idx += opcodes(1);
     }
   else if (lval->ident == iARRAYCHAR)
     {
	/* store at address in ALT */
	stgwrite("\tstrb.i ");
	outval(charbits / 8, TRUE);	/* write one or two bytes */
	code_idx += opcodes(1) + opargs(1);
     }
   else if (lval->ident == iREFERENCE)
     {
	assert(sym != NULL);
	if (sym->vclass == sLOCAL)
	   stgwrite("\tsref.s.pri ");
	else
	   stgwrite("\tsref.pri ");
	outval(sym->addr, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }
   else
     {
	assert(sym != NULL);
	markusage(sym, uWRITTEN);
	if (sym->vclass == sLOCAL)
	   stgwrite("\tstor.s.pri ");
	else
	   stgwrite("\tstor.pri ");
	outval(sym->addr, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/* source must in PRI, destination address in ALT. The "size"
 * parameter is in bytes, not cells.
 */
void
memcopy(cell size)
{
   stgwrite("\tmovs ");
   outval(size, TRUE);

   code_idx += opcodes(1) + opargs(1);
}

/* Address of the source must already have been loaded in PRI
 * "size" is the size in bytes (not cells).
 */
void
copyarray(symbol * sym, cell size)
{
   assert(sym != NULL);
   /* the symbol can be a local array, a global array, or an array
    * that is passed by reference.
    */
   if (sym->ident == iREFARRAY)
     {
	/* reference to an array; currently this is always a local variable */
	assert(sym->vclass == sLOCAL);	/* symbol must be stack relative */
	stgwrite("\tload.s.alt ");
     }
   else
     {
	/* a local or global array */
	if (sym->vclass == sLOCAL)
	   stgwrite("\taddr.alt ");
	else
	   stgwrite("\tconst.alt ");
     }				/* if */
   outval(sym->addr, TRUE);
   markusage(sym, uWRITTEN);

   code_idx += opcodes(1) + opargs(1);
   memcopy(size);
}

void
fillarray(symbol * sym, cell size, cell value)
{
   const1(value);		/* load value in PRI */

   assert(sym != NULL);
   /* the symbol can be a local array, a global array, or an array
    * that is passed by reference.
    */
   if (sym->ident == iREFARRAY)
     {
	/* reference to an array; currently this is always a local variable */
	assert(sym->vclass == sLOCAL);	/* symbol must be stack relative */
	stgwrite("\tload.s.alt ");
     }
   else
     {
	/* a local or global array */
	if (sym->vclass == sLOCAL)
	   stgwrite("\taddr.alt ");
	else
	   stgwrite("\tconst.alt ");
     }				/* if */
   outval(sym->addr, TRUE);
   markusage(sym, uWRITTEN);

   stgwrite("\tfill ");
   outval(size, TRUE);

   code_idx += opcodes(2) + opargs(2);
}

/*
 *  Instruction to get an immediate value into the primary register
 */
void
const1(cell val)
{
   if (val == 0)
     {
	stgwrite("\tzero.pri\n");
	code_idx += opcodes(1);
     }
   else
     {
	stgwrite("\tconst.pri ");
	outval(val, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*
 *  Instruction to get an immediate value into the secondary register
 */
void
const2(cell val)
{
   if (val == 0)
     {
	stgwrite("\tzero.alt\n");
	code_idx += opcodes(1);
     }
   else
     {
	stgwrite("\tconst.alt ");
	outval(val, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/* Copy value in secondary register to the primary register */
void
moveto1(void)
{
   stgwrite("\tmove.pri\n");
   code_idx += opcodes(1) + opargs(0);
}

/*
 *  Push primary register onto the stack
 */
void
push1(void)
{
   stgwrite("\tpush.pri\n");
   code_idx += opcodes(1);
}

/*
 *  Push alternate register onto the stack
 */
void
push2(void)
{
   stgwrite("\tpush.alt\n");
   code_idx += opcodes(1);
}

/*
 *  Push a constant value onto the stack
 */
void
pushval(cell val)
{
   stgwrite("\tpush.c ");
   outval(val, TRUE);
   code_idx += opcodes(1) + opargs(1);
}

/*
 *  pop stack to the primary register
 */
void
pop1(void)
{
   stgwrite("\tpop.pri\n");
   code_idx += opcodes(1);
}

/*
 *  pop stack to the secondary register
 */
void
pop2(void)
{
   stgwrite("\tpop.alt\n");
   code_idx += opcodes(1);
}

/*
 *  swap the top-of-stack with the value in primary register
 */
void
swap1(void)
{
   stgwrite("\tswap.pri\n");
   code_idx += opcodes(1);
}

/* Switch statements
 * The "switch" statement generates a "case" table using the "CASE" opcode.
 * The case table contains a list of records, each record holds a comparison
 * value and a label to branch to on a match. The very first record is an
 * exception: it holds the size of the table (excluding the first record) and
 * the label to branch to when none of the values in the case table match.
 * The case table is sorted on the comparison value. This allows more advanced
 * abstract machines to sift the case table with a binary search.
 */
void
ffswitch(int label)
{
   stgwrite("\tswitch ");
   outval(label, TRUE);		/* the label is the address of the case table */
   code_idx += opcodes(1) + opargs(1);
}

void
ffcase(cell value, char *labelname, int newtable)
{
   if (newtable)
     {
	stgwrite("\tcasetbl\n");
	code_idx += opcodes(1);
     }				/* if */
   stgwrite("\tcase ");
   outval(value, FALSE);
   stgwrite(" ");
   stgwrite(labelname);
   stgwrite("\n");
   code_idx += opcodes(0) + opargs(2);
}

/*
 *  Call specified function
 */
void
ffcall(symbol * sym, int numargs)
{
   assert(sym != NULL);
   assert(sym->ident == iFUNCTN);
   if ((sym->usage & uNATIVE) != 0)
     {
	/* reserve a SYSREQ id if called for the first time */
	if (sc_status == statWRITE && (sym->usage & uREAD) == 0
	    && sym->addr >= 0)
	   sym->addr = ntv_funcid++;
	stgwrite("\tsysreq.c ");
	outval(sym->addr, FALSE);
	stgwrite("\n\tstack ");
	outval((numargs + 1) * sizeof(cell), TRUE);
	code_idx += opcodes(2) + opargs(2);
     }
   else
     {
	/* normal function */
	stgwrite("\tcall ");
	stgwrite(sym->name);
	stgwrite("\n");
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*  Return from function
 *
 *  Global references: funcstatus  (referred to only)
 */
void
ffret(void)
{
   stgwrite("\tretn\n");
   code_idx += opcodes(1);
}

void
ffabort(int reason)
{
   stgwrite("\thalt ");
   outval(reason, TRUE);
   code_idx += opcodes(1) + opargs(1);
}

void
ffbounds(cell size)
{
   if ((sc_debug & sCHKBOUNDS) != 0)
     {
	stgwrite("\tbounds ");
	outval(size, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*
 *  Jump to local label number (the number is converted to a name)
 */
void
jumplabel(int number)
{
   stgwrite("\tjump ");
   outval(number, TRUE);
   code_idx += opcodes(1) + opargs(1);
}

/*
 *   Define storage (global and static variables)
 */
void
defstorage(void)
{
   stgwrite("dump ");
}

/*
 *  Inclrement/decrement stack pointer. Note that this routine does
 *  nothing if the delta is zero.
 */
void
modstk(int delta)
{
   if (delta)
     {
	stgwrite("\tstack ");
	outval(delta, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/* set the stack to a hard offset from the frame */
void
setstk(cell value)
{
   stgwrite("\tlctrl 5\n");	/* get FRM */
   assert(value <= 0);		/* STK should always become <= FRM */
   if (value < 0)
     {
	stgwrite("\tadd.c ");
	outval(value, TRUE);	/* add (negative) offset */
	code_idx += opcodes(1) + opargs(1);
	// ??? write zeros in the space between STK and the value in PRI (the new stk)
	//     get value of STK in ALT
	//     zero PRI
	//     need new FILL opcode that takes a variable size
     }				/* if */
   stgwrite("\tsctrl 4\n");	/* store in STK */
   code_idx += opcodes(2) + opargs(2);
}

void
modheap(int delta)
{
   if (delta)
     {
	stgwrite("\theap ");
	outval(delta, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

void
setheap_pri(void)
{
   stgwrite("\theap ");		/* ALT = HEA++ */
   outval(sizeof(cell), TRUE);
   stgwrite("\tstor.i\n");	/* store PRI (default value) at address ALT */
   stgwrite("\tmove.pri\n");	/* move ALT to PRI: PRI contains the address */
   code_idx += opcodes(3) + opargs(1);
}

void
setheap(cell value)
{
   stgwrite("\tconst.pri ");	/* load default value in PRI */
   outval(value, TRUE);
   code_idx += opcodes(1) + opargs(1);
   setheap_pri();
}

/*
 *  Convert a cell number to a "byte" address; i.e. double or quadruple
 *  the primary register.
 */
void
cell2addr(void)
{
#if defined(BIT16)
   stgwrite("\tshl.c.pri 1\n");
#else
   stgwrite("\tshl.c.pri 2\n");
#endif
   code_idx += opcodes(1) + opargs(1);
}

/*
 *  Double or quadruple the alternate register.
 */
void
cell2addr_alt(void)
{
#if defined(BIT16)
   stgwrite("\tshl.c.alt 1\n");
#else
   stgwrite("\tshl.c.alt 2\n");
#endif
   code_idx += opcodes(1) + opargs(1);
}

/*
 *  Convert "distance of addresses" to "number of cells" in between.
 *  Or convert a number of packed characters to the number of cells (with
 *  truncation).
 */
void
addr2cell(void)
{
#if defined(BIT16)
   stgwrite("\tshr.c.pri 1\n");
#else
   stgwrite("\tshr.c.pri 2\n");
#endif
   code_idx += opcodes(1) + opargs(1);
}

/* Convert from character index to byte address. This routine does
 * nothing if a character has the size of a byte.
 */
void
char2addr(void)
{
   if (charbits == 16)
     {
	stgwrite("\tshl.c.pri 1\n");
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/* Align PRI (which should hold a character index) to an address.
 * The first character in a "pack" occupies the highest bits of
 * the cell. This is at the lower memory address on Big Endian
 * computers and on the higher address on Little Endian computers.
 * The ALIGN.pri/alt instructions must solve this machine dependence;
 * that is, on Big Endian computers, ALIGN.pri/alt shuold do nothing
 * and on Little Endian computers they should toggle the address.
 */
void
charalign(void)
{
   stgwrite("\talign.pri ");
   outval(charbits / 8, TRUE);
   code_idx += opcodes(1) + opargs(1);
}

/*
 *  Add a constant to the primary register.
 */
void
addconst(cell value)
{
   if (value != 0)
     {
	stgwrite("\tadd.c ");
	outval(value, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*
 *  signed multiply of primary and secundairy registers (result in primary)
 */
void
os_mult(void)
{
   stgwrite("\tsmul\n");
   code_idx += opcodes(1);
}

/*
 *  signed divide of alternate register by primary register (quotient in
 *  primary; remainder in alternate)
 */
void
os_div(void)
{
   stgwrite("\tsdiv.alt\n");
   code_idx += opcodes(1);
}

/*
 *  modulus of (alternate % primary), result in primary (signed)
 */
void
os_mod(void)
{
   stgwrite("\tsdiv.alt\n");
   stgwrite("\tmove.pri\n");	/* move ALT to PRI */
   code_idx += opcodes(2);
}

/*
 *  Add primary and alternate registers (result in primary).
 */
void
ob_add(void)
{
   stgwrite("\tadd\n");
   code_idx += opcodes(1);
}

/*
 *  subtract primary register from alternate register (result in primary)
 */
void
ob_sub(void)
{
   stgwrite("\tsub.alt\n");
   code_idx += opcodes(1);
}

/*
 *  arithmic shift left alternate register the number of bits
 *  given in the primary register (result in primary).
 *  There is no need for a "logical shift left" routine, since
 *  logical shift left is identical to arithmic shift left.
 */
void
ob_sal(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tshl\n");
   code_idx += opcodes(2);
}

/*
 *  arithmic shift right alternate register the number of bits
 *  given in the primary register (result in primary).
 */
void
os_sar(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tsshr\n");
   code_idx += opcodes(2);
}

/*
 *  logical (unsigned) shift right of the alternate register by the
 *  number of bits given in the primary register (result in primary).
 */
void
ou_sar(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tshr\n");
   code_idx += opcodes(2);
}

/*
 *  inclusive "or" of primary and secondary registers (result in primary)
 */
void
ob_or(void)
{
   stgwrite("\tor\n");
   code_idx += opcodes(1);
}

/*
 *  "exclusive or" of primary and alternate registers (result in primary)
 */
void
ob_xor(void)
{
   stgwrite("\txor\n");
   code_idx += opcodes(1);
}

/*
 *  "and" of primary and secundairy registers (result in primary)
 */
void
ob_and(void)
{
   stgwrite("\tand\n");
   code_idx += opcodes(1);
}

/*
 *  test ALT==PRI; result in primary register (1 or 0).
 */
void
ob_eq(void)
{
   stgwrite("\teq\n");
   code_idx += opcodes(1);
}

/*
 *  test ALT!=PRI
 */
void
ob_ne(void)
{
   stgwrite("\tneq\n");
   code_idx += opcodes(1);
}

/* The abstract machine defines the relational instructions so that PRI is
 * on the left side and ALT on the right side of the operator. For example,
 * SLESS sets PRI to either 1 or 0 depending on whether the expression
 * "PRI < ALT" is true.
 *
 * The compiler generates comparisons with ALT on the left side of the
 * relational operator and PRI on the right side. The XCHG instruction
 * prefixing the relational operators resets this. We leave it to the
 * peephole optimizer to choose more compact instructions where possible.
 */

/* Relational operator prefix for chained relational expressions. The
 * "suffix" code restores the stack.
 * For chained relational operators, the goal is to keep the comparison
 * result "so far" in PRI and the value of the most recent operand in
 * ALT, ready for a next comparison.
 * The "prefix" instruction pushed the comparison result (PRI) onto the
 * stack and moves the value of ALT into PRI. If there is a next comparison,
 * PRI can now serve as the "left" operand of the relational operator.
 */
void
relop_prefix(void)
{
   stgwrite("\tpush.pri\n");
   stgwrite("\tmove.pri\n");
   code_idx += opcodes(2);
}

void
relop_suffix(void)
{
   stgwrite("\tswap.alt\n");
   stgwrite("\tand\n");
   stgwrite("\tpop.alt\n");
   code_idx += opcodes(3);
}

/*
 *  test ALT<PRI (signed)
 */
void
os_lt(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tsless\n");
   code_idx += opcodes(2);
}

/*
 *  test ALT<=PRI (signed)
 */
void
os_le(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tsleq\n");
   code_idx += opcodes(2);
}

/*
 *  test ALT>PRI (signed)
 */
void
os_gt(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tsgrtr\n");
   code_idx += opcodes(2);
}

/*
 *  test ALT>=PRI (signed)
 */
void
os_ge(void)
{
   stgwrite("\txchg\n");
   stgwrite("\tsgeq\n");
   code_idx += opcodes(2);
}

/*
 *  logical negation of primary register
 */
void
lneg(void)
{
   stgwrite("\tnot\n");
   code_idx += opcodes(1);
}

/*
 *  two's complement primary register
 */
void
neg(void)
{
   stgwrite("\tneg\n");
   code_idx += opcodes(1);
}

/*
 *  one's complement of primary register
 */
void
invert(void)
{
   stgwrite("\tinvert\n");
   code_idx += opcodes(1);
}

/*
 *  nop
 */
void
nooperation(void)
{
   stgwrite("\tnop\n");
   code_idx += opcodes(1);
}

/*  increment symbol
 */
void
inc(value * lval)
{
   symbol             *sym;

   sym = lval->sym;
   if (lval->ident == iARRAYCELL)
     {
	/* indirect increment, address already in PRI */
	stgwrite("\tinc.i\n");
	code_idx += opcodes(1);
     }
   else if (lval->ident == iARRAYCHAR)
     {
	/* indirect increment of single character, address already in PRI */
	stgwrite("\tpush.pri\n");
	stgwrite("\tpush.alt\n");
	stgwrite("\tmove.alt\n");	/* copy address */
	stgwrite("\tlodb.i ");	/* read from PRI into PRI */
	outval(charbits / 8, TRUE);	/* read one or two bytes */
	stgwrite("\tinc.pri\n");
	stgwrite("\tstrb.i ");	/* write PRI to ALT */
	outval(charbits / 8, TRUE);	/* write one or two bytes */
	stgwrite("\tpop.alt\n");
	stgwrite("\tpop.pri\n");
	code_idx += opcodes(8) + opargs(2);
     }
   else if (lval->ident == iREFERENCE)
     {
	assert(sym != NULL);
	stgwrite("\tpush.pri\n");
	/* load dereferenced value */
	assert(sym->vclass == sLOCAL);	/* global references don't exist in Small */
	if (sym->vclass == sLOCAL)
	   stgwrite("\tlref.s.pri ");
	else
	   stgwrite("\tlref.pri ");
	outval(sym->addr, TRUE);
	/* increment */
	stgwrite("\tinc.pri\n");
	/* store dereferenced value */
	if (sym->vclass == sLOCAL)
	   stgwrite("\tsref.s.pri ");
	else
	   stgwrite("\tsref.pri ");
	outval(sym->addr, TRUE);
	stgwrite("\tpop.pri\n");
	code_idx += opcodes(5) + opargs(2);
     }
   else
     {
	/* local or global variable */
	assert(sym != NULL);
	if (sym->vclass == sLOCAL)
	   stgwrite("\tinc.s ");
	else
	   stgwrite("\tinc ");
	outval(sym->addr, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*  decrement symbol
 *
 *  in case of an integer pointer, the symbol must be incremented by 2.
 */
void
dec(value * lval)
{
   symbol             *sym;

   sym = lval->sym;
   if (lval->ident == iARRAYCELL)
     {
	/* indirect decrement, address already in PRI */
	stgwrite("\tdec.i\n");
	code_idx += opcodes(1);
     }
   else if (lval->ident == iARRAYCHAR)
     {
	/* indirect decrement of single character, address already in PRI */
	stgwrite("\tpush.pri\n");
	stgwrite("\tpush.alt\n");
	stgwrite("\tmove.alt\n");	/* copy address */
	stgwrite("\tlodb.i ");	/* read from PRI into PRI */
	outval(charbits / 8, TRUE);	/* read one or two bytes */
	stgwrite("\tdec.pri\n");
	stgwrite("\tstrb.i ");	/* write PRI to ALT */
	outval(charbits / 8, TRUE);	/* write one or two bytes */
	stgwrite("\tpop.alt\n");
	stgwrite("\tpop.pri\n");
	code_idx += opcodes(8) + opargs(2);
     }
   else if (lval->ident == iREFERENCE)
     {
	assert(sym != NULL);
	stgwrite("\tpush.pri\n");
	/* load dereferenced value */
	assert(sym->vclass == sLOCAL);	/* global references don't exist in Small */
	if (sym->vclass == sLOCAL)
	   stgwrite("\tlref.s.pri ");
	else
	   stgwrite("\tlref.pri ");
	outval(sym->addr, TRUE);
	/* decrement */
	stgwrite("\tdec.pri\n");
	/* store dereferenced value */
	if (sym->vclass == sLOCAL)
	   stgwrite("\tsref.s.pri ");
	else
	   stgwrite("\tsref.pri ");
	outval(sym->addr, TRUE);
	stgwrite("\tpop.pri\n");
	code_idx += opcodes(5) + opargs(2);
     }
   else
     {
	/* local or global variable */
	assert(sym != NULL);
	if (sym->vclass == sLOCAL)
	   stgwrite("\tdec.s ");
	else
	   stgwrite("\tdec ");
	outval(sym->addr, TRUE);
	code_idx += opcodes(1) + opargs(1);
     }				/* if */
}

/*
 *  Jumps to "label" if PRI != 0
 */
void
jmp_ne0(int number)
{
   stgwrite("\tjnz ");
   outval(number, TRUE);
   code_idx += opcodes(1) + opargs(1);
}

/*
 *  Jumps to "label" if PRI == 0
 */
void
jmp_eq0(int number)
{
   stgwrite("\tjzer ");
   outval(number, TRUE);
   code_idx += opcodes(1) + opargs(1);
}

/* write a value in hexadecimal; optionally adds a newline */
void
outval(cell val, int newline)
{
   stgwrite(itoh(val));
   if (newline)
      stgwrite("\n");
}
