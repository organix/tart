/*

pair.c -- Tiny Actor Run-Time

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

#include "pair.h"

inline Pair
pair_new(Any h, Any t)
{
    Pair q = NEW(PAIR);
    q->h = h;
    q->t = t;
    return q;
}

inline Pair
list_new()
{
    return NIL;
}

inline int
list_empty_p(Pair list)
{
    return (list == NIL);
}

inline Pair
list_pop(Pair list)
{
    return list;
}

inline Pair
list_push(Pair list, Any item)
{
    return PR(item, list);
}

inline Pair
queue_new()
{
    return PR(NIL, NIL);
}

inline int
queue_empty_p(Pair q)
{
    return (q->h == NIL);
}

inline void
queue_give(Pair q, Any item)
{
    Pair p = PR(item, NIL);
    if (queue_empty_p(q)) {
        q->h = p;
    } else {
        Pair t = q->t;
        t->t = p;
    }
    q->t = p;
}

inline Any
queue_take(Pair q)
{
    // if (queue_empty_p(q)) ERROR("can't take from empty queue");
    Pair p = q->h;
    Any item = p->h;
    q->h = p->t;
    p = FREE(p);
    return item;
}

