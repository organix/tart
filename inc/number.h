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

#undef USE_U_BYTES     /* maintain raw data pointer in integer structure */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct integer INTEGER, *Integer;
#ifdef USE_U_BYTES
typedef struct u_bytes U_BYTES, *U_bytes;
#endif /* USE_U_BYTES */

#define a_minus_one ((Actor)(the_integer_zero - 1))
#define a_zero ((Actor)(the_integer_zero))
#define a_one ((Actor)(the_integer_zero + 1))
#define a_two ((Actor)(the_integer_zero + 2))

extern Integer the_integer_zero;

struct integer {
    ACTOR       _act;
    int         i;
};

#ifdef USE_U_BYTES
struct u_bytes {
    INTEGER     _int;
    u8 *        p;
};
#endif /* USE_U_BYTES */

extern Actor    integer_new(int i);
extern Boolean  number_match_method(Actor this, Actor that);
extern Actor    number_diff_method(Actor this, Actor that);
extern Actor    number_plus_method(Actor this, Actor that);
extern Actor    number_times_method(Actor this, Actor that);

extern Actor    u16_new(u8* p);
extern Actor    u32_new(u8* p);

extern void     beh_integer(Event e);

extern void     test_number();  // unit-test method

#endif /* _NUMBER_H_ */
