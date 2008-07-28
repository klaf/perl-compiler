/* -*- buffer-read-only: t -*-
 *
 *      Copyright (c) 1996-1999 Malcolm Beattie
 *      Copyright (c) 2007 Yann Nicolas Dauphin (clisp: jit.d)
 *      Copyright (c) 2008 Reini Urban
 *
 *      You may distribute under the terms of either the GNU General Public
 *      License or the Artistic License, as specified in the README file.
 *
 */
/*
 * This file is autogenerated from jitcompiler.pl. Changes made here will be lost.
 */

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#define NO_XSLOCKS
#include "XSUB.h"

#ifndef PL_tokenbuf /* Change 31252: move PL_tokenbuf into the PL_parser struct */
#define PL_tokenbuf		(PL_parser->tokenbuf)
#endif

#include "byterun.h"
#include "bytecode.h"
#include "jitrun.h"

typedef int (*pifi)(int);    /* Pointer to Int Function of Int */

static const int optype_size[] = {
    sizeof(OP),
    sizeof(UNOP),
    sizeof(BINOP),
    sizeof(LOGOP),
    sizeof(LISTOP),
    sizeof(PMOP),
    sizeof(SVOP),
    sizeof(PADOP),
    sizeof(PVOP),
    sizeof(LOOP),
    sizeof(COP)
};

void *
jit_obj_store(pTHX_ struct byteloader_state *bstate, void *obj, I32 ix)
{
    if (ix > bstate->bs_obj_list_fill) {
	Renew(bstate->bs_obj_list, ix + 32, void*);
	bstate->bs_obj_list_fill = ix + 31;
    }
    bstate->bs_obj_list[ix] = obj;
    return obj;
}

int
jitrun(pTHX_ struct byteloader_state *bstate)
{
    // register int insn;
    pifi insn;
    U32 isjit = 0;
    U32 ix;
    SV *specialsv_list[6];

    BYTECODE_HEADER_CHECK;	/* croak if incorrect platform, */
    if (!isjit) {		/* set isjit if PLJC magic header */
      Perl_croak(aTHX_ "No perl jitcode header PLJC\n");
      return 0;
    }
#if 0
    Newx(bstate->bs_obj_list, 32, void*); /* set op objlist */
    bstate->bs_obj_list_fill = 31;
    bstate->bs_obj_list[0] = NULL; /* first is always Null */
    bstate->bs_ix = 1;

    specialsv_list[0] = Nullsv;
    specialsv_list[1] = &PL_sv_undef;
    specialsv_list[2] = &PL_sv_yes;
    specialsv_list[3] = &PL_sv_no;
    specialsv_list[4] = (SV*)pWARN_ALL;
    specialsv_list[5] = (SV*)pWARN_NONE;
    specialsv_list[6] = (SV*)pWARN_STD;
#endif

    int byteptr_max = 1000; /* size of DATA */

    /* codebuffer: contains the JITed code (Temp allocation scheme) */
    jit_insn *codeBuffer = malloc(sizeof(jit_insn)*byteptr_max*JIT_AVG_BCSIZE);
    /* bcIndex: Address of the beginning of each BC in codeBuffer */
    /* Only needed by (JMPHASH) and the unwind protect BCs */
    jit_insn **bcIndex = calloc(byteptr_max+1,sizeof(jit_insn*));

    /* TODO: setup the bcIndex jumps and copy codeBuffer */

    jit_func bc_func = (jit_func) (jit_set_ip(codeBuffer).iptr); /* Function ptr */
#ifdef DEBUGGING
    disassemble(stderr, codeBuffer, jit_get_ip().ptr);
#endif
    jit_flush_code(codeBuffer, jit_get_ip().ptr);

    //Perl_croak(aTHX_ "TODO! jit_run the code\n"); /* jit_run the codebuffer */
    /* Call the JITed function */
    bc_func(codeBuffer, 0);

    free(codeBuffer);
    free(bcIndex);
    return;
}
/* ex: set ro: */
