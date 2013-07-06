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

PAIR the_nil_pair = {
    &the_nil_pair,
    &the_nil_pair
};

inline Pair
pair_new(Any h, Any t)
{
    Pair p = NEW(PAIR);
    p->h = h;
    p->t = t;
    return p;
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
deque_new()
{
    return PR(NIL, NIL);
}
inline int
deque_empty_p(Pair q)
{
    return (q->h == NIL);
}
inline void
deque_give(Pair q, Any item)
{
    Pair p = PR(item, NIL);
    if (deque_empty_p(q)) {
        q->h = p;
    } else {
        Pair t = q->t;
        t->t = p;
    }
    q->t = p;
}
inline Any
deque_take(Pair q)
{
    if (deque_empty_p(q)) {
        halt("deque_take from empty!");
    }
    Pair p = q->h;
    Any item = p->h;
    q->h = p->t;
    p = FREE(p);
    return item;
}
inline void
deque_return(Pair q, Any item)
{
    Pair p = PR(item, q->h);
    if (deque_empty_p(q)) {
        q->t = p;
    }
    q->h = p;
}
inline Any
deque_lookup(Pair q, int i)
{
    Pair p = q->h;
    while (p != NIL) {
        if (i <= 0) {
            return p->h;
        }
        --i;
        p = p->t;
    }
    return NULL;  // not found
}
inline void
deque_bind(Pair q, int i, Any item)
{
    Pair p = q->h;
    while (p != NIL) {
        if (i <= 0) {
            p->h = item;
        }
        --i;
        p = p->t;
    }
    // not found
}

inline Pair
dict_new()
{
    return NIL;
}
inline int
dict_empty_p(Pair dict)
{
    return (dict == NIL);
}
inline Any
dict_lookup(Pair dict, Any key)
{
    while (!dict_empty_p(dict)) {
        Pair p = dict->h;
        if (p->h == key) {
            return p->t;
        }
        dict = dict->t;
    }
    return NULL;  // NOT FOUND
}
inline Pair
dict_bind(Pair dict, Any key, Any value)
{
    return PR(PR(key, value), dict);
}
