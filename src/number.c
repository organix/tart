/*

number.c -- Tiny Actor Run-Time

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

#include "number.h"

inline Actor
integer_new(int i)
{
    switch (i) {  // check for cached objects
        case -1 : return a_minus_one;
        case 0 : return a_zero;
        case 1 : return a_one;
        case 2 : return a_two;
    }
    Integer n = NEW(INTEGER);
    BEH(n) = beh_integer;
    n->i = i;
    return (Actor)n;
}

inline Boolean
number_match_method(Actor this, Actor that)
{
    if (this == that) {
        return a_true;
    }
    if (beh_integer != BEH(this)) { halt("number_match_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        if (n->i == m->i) {
            return a_true;
        }
    }
    return a_false;
}

inline Actor
number_diff_method(Actor this, Actor that)
{
    if (beh_integer != BEH(this)) { halt("number_match_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        return integer_new(n->i - m->i);
    }
    return this;
}

inline Actor
number_plus_method(Actor this, Actor that)
{
    if (beh_integer != BEH(this)) { halt("number_match_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        return integer_new(n->i + m->i);
    }
    return this;
}

inline Actor
number_times_method(Actor this, Actor that)
{
    if (beh_integer != BEH(this)) { halt("number_match_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        return integer_new(n->i * m->i);
    }
    return this;
}

void
beh_integer(Event e)
{
    TRACE(fprintf(stderr, "beh_integer{event=%p}\n", e));
    halt("HALT!");
}
INTEGER the_number_minus_one = { { beh_integer }, -1 };
INTEGER the_number_zero = { { beh_integer }, 0 };
INTEGER the_number_one = { { beh_integer }, 1 };
INTEGER the_number_two = { { beh_integer }, 2 };

void
test_number()
{
    Actor a, b;
    Integer n, m;

    TRACE(fprintf(stderr, "---- test_number ----\n"));
    TRACE(fprintf(stderr, "a_zero = %p\n", a_zero));
    a = integer_new(0);
    if (a_zero != a) { halt("expected a_zero == a"); }
    a = number_plus_method(a, a_one);
    if (a_one != a) { halt("expected a_one == a"); }
    a = number_diff_method(a, a_two);
    if (a_minus_one != a) { halt("expected a_minus_one == a"); }
/*
*/
    a = number_times_method(a_two, a_two);
    TRACE(fprintf(stderr, "a = %p\n", a));
    if (beh_integer != BEH(a)) { halt("beh_integer == BEH(a)"); }
    n = (Integer)a;
    TRACE(fprintf(stderr, "n->i = %d\n", n->i));
    b = integer_new(4);
    TRACE(fprintf(stderr, "b = %p\n", b));
    if (a == b) { halt("expected a != b"); }
    if (beh_integer != BEH(b)) { halt("beh_integer == BEH(b)"); }
    m = (Integer)b;
    TRACE(fprintf(stderr, "m->i = %d\n", m->i));
    if (n->i != m->i) { halt("expected n->i == m->i"); }
}
