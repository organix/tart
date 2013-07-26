/*

pair.h -- Tiny Actor Run-Time

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
#ifndef _PAIR_H_
#define _PAIR_H_

#include "tart.h"

typedef struct pair PAIR, *Pair;

struct pair {
    METHOD      _meth;
    Any         h;
    Any         t;
};

extern void     meth_halt(Any arg);
extern void     meth_pair(Any arg);

#define NIL (&the_nil_pair)
#define PR(h,t) pair_new((h),(t))

extern PAIR the_nil_pair;

extern Pair     pair_new(Any h, Any t);

extern Pair     list_new();
extern int      list_empty_p(Pair list);
extern Pair     list_pop(Pair list);
extern Pair     list_push(Pair list, Any item);

extern Pair     deque_new();
extern int      deque_empty_p(Pair q);
extern void     deque_give(Pair q, Any item);
extern Any      deque_take(Pair q);
extern void     deque_return(Pair q, Any item);
extern Any      deque_lookup(Pair q, int i);
extern void     deque_bind(Pair q, int i, Any item);

extern Pair     dict_new();
extern int      dict_empty_p(Pair dict);
extern Any      dict_lookup(Pair dict, Any key);
extern Pair     dict_bind(Pair dict, Any key, Any value);

#endif /* _PAIR_H_ */
