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

extern ACTOR fail_actor;
extern ACTOR empty_env_actor;

extern Actor b_true;
extern Actor b_false;

extern Actor s_eval;
extern Actor s_match;
extern Actor s_lookup;
extern Actor s_bind;
extern Actor s_comb;

extern void     act_value(Event e);
extern void     act_scope(Event e);
extern void     act_bind_ptrn(Event e);
extern void     act_name(Event e);
extern void     act_comb(Event e);
extern void     act_appl(Event e);
extern void     act_thunk(Event e);
extern void     act_lambda(Event e);
extern void     act_oper(Event e);

#endif /* _PAIR_H_ */
