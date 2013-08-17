/*

string.c -- Tiny Actor Run-Time

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

#include "string.h"

STRING the_empty_string_actor = { { beh_string }, "", 0 };

inline Actor
cstring_new(char * p)
{
    String s = NEW(STRING);
    BEH(s) = beh_string;
    s->p = p;  // must have '\0' terminator
    s->n = -1;  // unknown length
    return (Actor)s;
}

inline Actor
pstring_new(char * p, int n)
{
    String s = NEW(STRING);
    BEH(s) = beh_string;
    s->p = p;  // may, or may not, have '\0' terminator
    s->n = n;  // pre-defined length
    return (Actor)s;
}

inline Actor
string_length_method(Actor this)
{
    if (beh_string != BEH(this)) { halt("string_length_method: string required"); }
    String s = (String)this;
    if (s->n < 0) {  // unknown length
        int n = 0;
        char *p = s->p;
        while (*p++) {
            ++n;
        }
        s->n = n;
    }
    return integer_new(s->n);
}

inline Actor
string_match_method(Actor this, Actor that)
{
    if (this == that) {
        return a_true;
    }
    if (beh_string != BEH(this)) { halt("string_match_method: string required"); }
    String s = (String)this;
    if (beh_string == BEH(that)) {
        String t = (String)that;
        if ((s->p == t->p) && (s->n == t->n)) {
            return a_true;
        }
        Actor n = string_length_method(this);
        Actor m = string_length_method(that);
        if (number_match_method(n, m) != a_true) {
            return a_false;
        }
        int i = ((Integer)(n))->i;
        char* p = s->p;
        char* q = t->p;
        while (i-- > 0) {
            if (*p++ != *q++) {
                return a_false;
            }
        }
        return a_true;
    }
    return a_false;
}

void
beh_string(Event e)
{
    TRACE(fprintf(stderr, "beh_string{event=%p}\n", e));
    halt("HALT!");
}

void
test_string()
{
    Actor a, b;
    Integer n, m;
    String s, t;

    TRACE(fprintf(stderr, "---- test_string ----\n"));
    TRACE(fprintf(stderr, "a_empty_string = %p\n", a_empty_string));
    a = string_length_method(a_empty_string);
    if (a_zero != a) { halt("expected a_zero == a"); }
    if (number_match_method(a, a_zero) != a_true) { halt("expected number_match_method(a, a_zero) == a_true"); }
    n = (Integer)a;
    if (n->i != 0) { halt("expected n->i == 0"); }
    s = (String)a_empty_string;
    TRACE(fprintf(stderr, "s->n = %d\n", s->n));
    TRACE(fprintf(stderr, "s->p = \"%*s\"\n", s->n, s->p));
/*
*/
    char* p = "foobarfoo";
    char* q = p + 6;
    a = pstring_new(p, 3);
    s = (String)a;
    TRACE(fprintf(stderr, "s = %d\"%*s\"\n", s->n, s->n, s->p));
    b = cstring_new(q);
    t = (String)b;
    TRACE(fprintf(stderr, "t = %d\"%*s\"\n", t->n, t->n, t->p));
    if (string_match_method(a, b) != a_true) { halt("expected string_match_method(a, b) == a_true"); }
    TRACE(fprintf(stderr, "t = %d\"%*s\"\n", t->n, t->n, t->p));
}
