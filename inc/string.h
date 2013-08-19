/*

string.h -- Tiny Actor Run-Time

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
#ifndef _STRING_H_
#define _STRING_H_

#include "tart.h"
#include "actor.h"

typedef struct string STRING, *String;

#define a_empty_string ((Actor)(&the_empty_string_actor))

struct string {
    ACTOR       _act;
    char *      p;  // character data
    Actor       n;  // length (-1 = unknown)
};

extern Actor    cstring_new(char * p);
extern Actor    pstring_new(char * p, int n);
extern Actor    string_length_method(Actor this);
extern Actor    string_intern_method(Actor this);
extern Actor    string_match_method(Actor this, Actor that);
extern Actor    string_diff_method(Actor this, Actor that);

extern void     beh_string(Event e);

extern void     test_string();  // unit-test method

extern STRING the_empty_string_actor;

#endif /* _STRING_H_ */
