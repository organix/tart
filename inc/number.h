/*

number.h -- Tiny Actor Run-Time

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
#ifndef _NUMBER_H_
#define _NUMBER_H_

#include "tart.h"
#include "actor.h"

typedef struct integer INTEGER, *Integer;

#define a_minus_one ((Actor)(&the_number_minus_one))
#define a_zero ((Actor)(&the_number_zero))
#define a_one ((Actor)(&the_number_one))
#define a_two ((Actor)(&the_number_two))

struct integer {
    ACTOR       _act;
    int         i;
};

extern Actor    integer_new(int i);
extern Boolean  number_match_method(Actor this, Actor that);
extern Actor    number_diff_method(Actor this, Actor that);
extern Actor    number_plus_method(Actor this, Actor that);
extern Actor    number_times_method(Actor this, Actor that);

extern void     beh_integer(Event e);

extern void     test_number();  // unit-test method

extern INTEGER the_number_minus_one;
extern INTEGER the_number_zero;
extern INTEGER the_number_one;
extern INTEGER the_number_two;

#endif /* _NUMBER_H_ */
