/*

universe.h -- Tiny Actor Run-Time

"MIT License"

Copyright (c) 2013 Dale Schumacher, Tristan Slominski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#ifndef _UNIVERSE_H_
#define _UNIVERSE_H_

#include "tart.h"
#include "pair.h"
#include "actor.h"

#define a_fail (&fail_actor)
#define a_empty_env (&empty_env_actor)
#define a_skip_ptrn (&skip_ptrn_actor)
#define a_empty (&empty_actor)
#define a_empty_ptrn (&empty_ptrn_actor)
#define a_appl_eval (&appl_eval_actor)

extern ACTOR fail_actor;
extern ACTOR empty_env_actor;
extern ACTOR skip_ptrn_actor;
extern ACTOR empty_ptrn_actor;

extern Actor b_true;
extern Actor b_false;

extern Actor s_eval;
extern Actor s_match;
extern Actor s_lookup;
extern Actor s_bind;
extern Actor s_comb;

extern void     beh_value(Event e);
extern void     act_scope(Event e);  // (dict, parent) -- SERIALIZED
extern void     val_bind_ptrn(Event e);  // (name)
extern void     beh_name(Event e);
extern void     val_comb(Event e);  // (oper, opnd)
extern void     val_appl(Event e);  // (comb)
extern void     val_oper(Event e);  // (env_s, form, evar, body)
extern void     val_vau(Event e);  // (form, evar, body)
extern void     val_lambda(Event e);  // (form, body)
extern void     val_eq_ptrn(Event e);  // (value)
extern void     val_pair(Event e);  // (head, tail)
extern void     val_pair_ptrn(Event e);  // (h_ptrn, t_ptrn)
extern void     val_seq_expr(Event e);  // (head_expr, tail_expr)
extern void     val_par_expr(Event e);  // (head_expr, tail_expr)

extern void     test_universe();  // unit-test method

#endif /* _PAIR_H_ */
