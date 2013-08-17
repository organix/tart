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

#define N (128)
static INTEGER small_integers[N + 2*N] = {
    { { beh_integer }, -128 },
    { { beh_integer }, -127 },
    { { beh_integer }, -126 },
    { { beh_integer }, -125 },
    { { beh_integer }, -124 },
    { { beh_integer }, -123 },
    { { beh_integer }, -122 },
    { { beh_integer }, -121 },
    { { beh_integer }, -120 },
    { { beh_integer }, -119 },
    { { beh_integer }, -118 },
    { { beh_integer }, -117 },
    { { beh_integer }, -116 },
    { { beh_integer }, -115 },
    { { beh_integer }, -114 },
    { { beh_integer }, -113 },
    { { beh_integer }, -112 },
    { { beh_integer }, -111 },
    { { beh_integer }, -110 },
    { { beh_integer }, -109 },
    { { beh_integer }, -108 },
    { { beh_integer }, -107 },
    { { beh_integer }, -106 },
    { { beh_integer }, -105 },
    { { beh_integer }, -104 },
    { { beh_integer }, -103 },
    { { beh_integer }, -102 },
    { { beh_integer }, -101 },
    { { beh_integer }, -100 },
    { { beh_integer }, -99 },
    { { beh_integer }, -98 },
    { { beh_integer }, -97 },
    { { beh_integer }, -96 },
    { { beh_integer }, -95 },
    { { beh_integer }, -94 },
    { { beh_integer }, -93 },
    { { beh_integer }, -92 },
    { { beh_integer }, -91 },
    { { beh_integer }, -90 },
    { { beh_integer }, -89 },
    { { beh_integer }, -88 },
    { { beh_integer }, -87 },
    { { beh_integer }, -86 },
    { { beh_integer }, -85 },
    { { beh_integer }, -84 },
    { { beh_integer }, -83 },
    { { beh_integer }, -82 },
    { { beh_integer }, -81 },
    { { beh_integer }, -80 },
    { { beh_integer }, -79 },
    { { beh_integer }, -78 },
    { { beh_integer }, -77 },
    { { beh_integer }, -76 },
    { { beh_integer }, -75 },
    { { beh_integer }, -74 },
    { { beh_integer }, -73 },
    { { beh_integer }, -72 },
    { { beh_integer }, -71 },
    { { beh_integer }, -70 },
    { { beh_integer }, -69 },
    { { beh_integer }, -68 },
    { { beh_integer }, -67 },
    { { beh_integer }, -66 },
    { { beh_integer }, -65 },
    { { beh_integer }, -64 },
    { { beh_integer }, -63 },
    { { beh_integer }, -62 },
    { { beh_integer }, -61 },
    { { beh_integer }, -60 },
    { { beh_integer }, -59 },
    { { beh_integer }, -58 },
    { { beh_integer }, -57 },
    { { beh_integer }, -56 },
    { { beh_integer }, -55 },
    { { beh_integer }, -54 },
    { { beh_integer }, -53 },
    { { beh_integer }, -52 },
    { { beh_integer }, -51 },
    { { beh_integer }, -50 },
    { { beh_integer }, -49 },
    { { beh_integer }, -48 },
    { { beh_integer }, -47 },
    { { beh_integer }, -46 },
    { { beh_integer }, -45 },
    { { beh_integer }, -44 },
    { { beh_integer }, -43 },
    { { beh_integer }, -42 },
    { { beh_integer }, -41 },
    { { beh_integer }, -40 },
    { { beh_integer }, -39 },
    { { beh_integer }, -38 },
    { { beh_integer }, -37 },
    { { beh_integer }, -36 },
    { { beh_integer }, -35 },
    { { beh_integer }, -34 },
    { { beh_integer }, -33 },
    { { beh_integer }, -32 },
    { { beh_integer }, -31 },
    { { beh_integer }, -30 },
    { { beh_integer }, -29 },
    { { beh_integer }, -28 },
    { { beh_integer }, -27 },
    { { beh_integer }, -26 },
    { { beh_integer }, -25 },
    { { beh_integer }, -24 },
    { { beh_integer }, -23 },
    { { beh_integer }, -22 },
    { { beh_integer }, -21 },
    { { beh_integer }, -20 },
    { { beh_integer }, -19 },
    { { beh_integer }, -18 },
    { { beh_integer }, -17 },
    { { beh_integer }, -16 },
    { { beh_integer }, -15 },
    { { beh_integer }, -14 },
    { { beh_integer }, -13 },
    { { beh_integer }, -12 },
    { { beh_integer }, -11 },
    { { beh_integer }, -10 },
    { { beh_integer }, -9 },
    { { beh_integer }, -8 },
    { { beh_integer }, -7 },
    { { beh_integer }, -6 },
    { { beh_integer }, -5 },
    { { beh_integer }, -4 },
    { { beh_integer }, -3 },
    { { beh_integer }, -2 },
    { { beh_integer }, -1 },
    { { beh_integer }, 0 },
    { { beh_integer }, 1 },
    { { beh_integer }, 2 },
    { { beh_integer }, 3 },
    { { beh_integer }, 4 },
    { { beh_integer }, 5 },
    { { beh_integer }, 6 },
    { { beh_integer }, 7 },
    { { beh_integer }, 8 },
    { { beh_integer }, 9 },
    { { beh_integer }, 10 },
    { { beh_integer }, 11 },
    { { beh_integer }, 12 },
    { { beh_integer }, 13 },
    { { beh_integer }, 14 },
    { { beh_integer }, 15 },
    { { beh_integer }, 16 },
    { { beh_integer }, 17 },
    { { beh_integer }, 18 },
    { { beh_integer }, 19 },
    { { beh_integer }, 20 },
    { { beh_integer }, 21 },
    { { beh_integer }, 22 },
    { { beh_integer }, 23 },
    { { beh_integer }, 24 },
    { { beh_integer }, 25 },
    { { beh_integer }, 26 },
    { { beh_integer }, 27 },
    { { beh_integer }, 28 },
    { { beh_integer }, 29 },
    { { beh_integer }, 30 },
    { { beh_integer }, 31 },
    { { beh_integer }, 32 },
    { { beh_integer }, 33 },
    { { beh_integer }, 34 },
    { { beh_integer }, 35 },
    { { beh_integer }, 36 },
    { { beh_integer }, 37 },
    { { beh_integer }, 38 },
    { { beh_integer }, 39 },
    { { beh_integer }, 40 },
    { { beh_integer }, 41 },
    { { beh_integer }, 42 },
    { { beh_integer }, 43 },
    { { beh_integer }, 44 },
    { { beh_integer }, 45 },
    { { beh_integer }, 46 },
    { { beh_integer }, 47 },
    { { beh_integer }, 48 },
    { { beh_integer }, 49 },
    { { beh_integer }, 50 },
    { { beh_integer }, 51 },
    { { beh_integer }, 52 },
    { { beh_integer }, 53 },
    { { beh_integer }, 54 },
    { { beh_integer }, 55 },
    { { beh_integer }, 56 },
    { { beh_integer }, 57 },
    { { beh_integer }, 58 },
    { { beh_integer }, 59 },
    { { beh_integer }, 60 },
    { { beh_integer }, 61 },
    { { beh_integer }, 62 },
    { { beh_integer }, 63 },
    { { beh_integer }, 64 },
    { { beh_integer }, 65 },
    { { beh_integer }, 66 },
    { { beh_integer }, 67 },
    { { beh_integer }, 68 },
    { { beh_integer }, 69 },
    { { beh_integer }, 70 },
    { { beh_integer }, 71 },
    { { beh_integer }, 72 },
    { { beh_integer }, 73 },
    { { beh_integer }, 74 },
    { { beh_integer }, 75 },
    { { beh_integer }, 76 },
    { { beh_integer }, 77 },
    { { beh_integer }, 78 },
    { { beh_integer }, 79 },
    { { beh_integer }, 80 },
    { { beh_integer }, 81 },
    { { beh_integer }, 82 },
    { { beh_integer }, 83 },
    { { beh_integer }, 84 },
    { { beh_integer }, 85 },
    { { beh_integer }, 86 },
    { { beh_integer }, 87 },
    { { beh_integer }, 88 },
    { { beh_integer }, 89 },
    { { beh_integer }, 90 },
    { { beh_integer }, 91 },
    { { beh_integer }, 92 },
    { { beh_integer }, 93 },
    { { beh_integer }, 94 },
    { { beh_integer }, 95 },
    { { beh_integer }, 96 },
    { { beh_integer }, 97 },
    { { beh_integer }, 98 },
    { { beh_integer }, 99 },
    { { beh_integer }, 100 },
    { { beh_integer }, 101 },
    { { beh_integer }, 102 },
    { { beh_integer }, 103 },
    { { beh_integer }, 104 },
    { { beh_integer }, 105 },
    { { beh_integer }, 106 },
    { { beh_integer }, 107 },
    { { beh_integer }, 108 },
    { { beh_integer }, 109 },
    { { beh_integer }, 110 },
    { { beh_integer }, 111 },
    { { beh_integer }, 112 },
    { { beh_integer }, 113 },
    { { beh_integer }, 114 },
    { { beh_integer }, 115 },
    { { beh_integer }, 116 },
    { { beh_integer }, 117 },
    { { beh_integer }, 118 },
    { { beh_integer }, 119 },
    { { beh_integer }, 120 },
    { { beh_integer }, 121 },
    { { beh_integer }, 122 },
    { { beh_integer }, 123 },
    { { beh_integer }, 124 },
    { { beh_integer }, 125 },
    { { beh_integer }, 126 },
    { { beh_integer }, 127 },
    { { beh_integer }, 128 },
    { { beh_integer }, 129 },
    { { beh_integer }, 130 },
    { { beh_integer }, 131 },
    { { beh_integer }, 132 },
    { { beh_integer }, 133 },
    { { beh_integer }, 134 },
    { { beh_integer }, 135 },
    { { beh_integer }, 136 },
    { { beh_integer }, 137 },
    { { beh_integer }, 138 },
    { { beh_integer }, 139 },
    { { beh_integer }, 140 },
    { { beh_integer }, 141 },
    { { beh_integer }, 142 },
    { { beh_integer }, 143 },
    { { beh_integer }, 144 },
    { { beh_integer }, 145 },
    { { beh_integer }, 146 },
    { { beh_integer }, 147 },
    { { beh_integer }, 148 },
    { { beh_integer }, 149 },
    { { beh_integer }, 150 },
    { { beh_integer }, 151 },
    { { beh_integer }, 152 },
    { { beh_integer }, 153 },
    { { beh_integer }, 154 },
    { { beh_integer }, 155 },
    { { beh_integer }, 156 },
    { { beh_integer }, 157 },
    { { beh_integer }, 158 },
    { { beh_integer }, 159 },
    { { beh_integer }, 160 },
    { { beh_integer }, 161 },
    { { beh_integer }, 162 },
    { { beh_integer }, 163 },
    { { beh_integer }, 164 },
    { { beh_integer }, 165 },
    { { beh_integer }, 166 },
    { { beh_integer }, 167 },
    { { beh_integer }, 168 },
    { { beh_integer }, 169 },
    { { beh_integer }, 170 },
    { { beh_integer }, 171 },
    { { beh_integer }, 172 },
    { { beh_integer }, 173 },
    { { beh_integer }, 174 },
    { { beh_integer }, 175 },
    { { beh_integer }, 176 },
    { { beh_integer }, 177 },
    { { beh_integer }, 178 },
    { { beh_integer }, 179 },
    { { beh_integer }, 180 },
    { { beh_integer }, 181 },
    { { beh_integer }, 182 },
    { { beh_integer }, 183 },
    { { beh_integer }, 184 },
    { { beh_integer }, 185 },
    { { beh_integer }, 186 },
    { { beh_integer }, 187 },
    { { beh_integer }, 188 },
    { { beh_integer }, 189 },
    { { beh_integer }, 190 },
    { { beh_integer }, 191 },
    { { beh_integer }, 192 },
    { { beh_integer }, 193 },
    { { beh_integer }, 194 },
    { { beh_integer }, 195 },
    { { beh_integer }, 196 },
    { { beh_integer }, 197 },
    { { beh_integer }, 198 },
    { { beh_integer }, 199 },
    { { beh_integer }, 200 },
    { { beh_integer }, 201 },
    { { beh_integer }, 202 },
    { { beh_integer }, 203 },
    { { beh_integer }, 204 },
    { { beh_integer }, 205 },
    { { beh_integer }, 206 },
    { { beh_integer }, 207 },
    { { beh_integer }, 208 },
    { { beh_integer }, 209 },
    { { beh_integer }, 210 },
    { { beh_integer }, 211 },
    { { beh_integer }, 212 },
    { { beh_integer }, 213 },
    { { beh_integer }, 214 },
    { { beh_integer }, 215 },
    { { beh_integer }, 216 },
    { { beh_integer }, 217 },
    { { beh_integer }, 218 },
    { { beh_integer }, 219 },
    { { beh_integer }, 220 },
    { { beh_integer }, 221 },
    { { beh_integer }, 222 },
    { { beh_integer }, 223 },
    { { beh_integer }, 224 },
    { { beh_integer }, 225 },
    { { beh_integer }, 226 },
    { { beh_integer }, 227 },
    { { beh_integer }, 228 },
    { { beh_integer }, 229 },
    { { beh_integer }, 230 },
    { { beh_integer }, 231 },
    { { beh_integer }, 232 },
    { { beh_integer }, 233 },
    { { beh_integer }, 234 },
    { { beh_integer }, 235 },
    { { beh_integer }, 236 },
    { { beh_integer }, 237 },
    { { beh_integer }, 238 },
    { { beh_integer }, 239 },
    { { beh_integer }, 240 },
    { { beh_integer }, 241 },
    { { beh_integer }, 242 },
    { { beh_integer }, 243 },
    { { beh_integer }, 244 },
    { { beh_integer }, 245 },
    { { beh_integer }, 246 },
    { { beh_integer }, 247 },
    { { beh_integer }, 248 },
    { { beh_integer }, 249 },
    { { beh_integer }, 250 },
    { { beh_integer }, 251 },
    { { beh_integer }, 252 },
    { { beh_integer }, 253 },
    { { beh_integer }, 254 },
    { { beh_integer }, 255 }
};
Integer the_integer_zero = &small_integers[N];

inline Actor
integer_new(int i)
{
    if ((i < 2*N) && (i >= -N)) {  // check for cached objects
        return ((Actor)(the_integer_zero + i));
    }
    Integer n = NEW(INTEGER);
    BEH(n) = beh_integer;
    n->i = i;
    return (Actor)n;
}

inline Actor
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

inline uint16_t
u16_load(uint8_t* p)
{
    return ((p[0] << 8) | p[1]);
}

inline void
u16_store(uint8_t* p, uint16_t n)
{
    p[0] = (uint8_t)(n >> 8);
    p[1] = (uint8_t)(n);
}

inline uint32_t
u32_load(uint8_t* p)
{
    return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

inline void
u32_store(uint8_t* p, uint32_t n)
{
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)(n);
}

void
beh_integer(Event e)
{
    TRACE(fprintf(stderr, "beh_integer{event=%p}\n", e));
    halt("HALT!");
}

void
test_number()
{
    Actor a, b;
    Integer n, m;
    int i;

    TRACE(fprintf(stderr, "---- test_number ----\n"));
    TRACE(fprintf(stderr, "a_zero = %p\n", a_zero));
    a = integer_new(0);
    if (a_zero != a) { halt("expected a_zero == a"); }
    if (number_match_method(a, a_zero) != a_true) { halt("expected number_match_method(a, a_zero) == a_true"); }
    a = number_plus_method(a, a_one);
    if (a_one != a) { halt("expected a_one == a"); }
    a = number_diff_method(a, a_two);
    if (a_minus_one != a) { halt("expected a_minus_one == a"); }
/*
*/
    a = integer_new(100);
    a = number_times_method(a, a);
    TRACE(fprintf(stderr, "a = %p\n", a));
    if (beh_integer != BEH(a)) { halt("expected beh_integer == BEH(a)"); }
    n = (Integer)a;
    TRACE(fprintf(stderr, "n->i = %d\n", n->i));
    b = integer_new(100 * 100);
    TRACE(fprintf(stderr, "b = %p\n", b));
    if (a == b) { halt("expected a != b"); }
    if (beh_integer != BEH(b)) { halt("expected beh_integer == BEH(b)"); }
    m = (Integer)b;
    TRACE(fprintf(stderr, "m->i = %d\n", m->i));
    if (n->i != m->i) { halt("expected n->i == m->i"); }
    if (number_match_method(a, b) != a_true) { halt("expected number_match_method(a, b) == a_true"); }
/*
*/
    a = integer_new(N);
    b = number_times_method(a, a_minus_one);
    a = number_times_method(a, a_two);
    a = number_plus_method(a, a_minus_one);
    n = (Integer)b;
    TRACE(fprintf(stderr, "n->i = %d\n", n->i));
    m = (Integer)a;
    TRACE(fprintf(stderr, "m->i = %d\n", m->i));
    if (a != integer_new(2*N - 1)) { halt("expected a == integer_new(2*N - 1)"); }
    if (b != integer_new(-N)) { halt("expected b == integer_new(-N)"); }
/*
*/
    for (i = -N; i < 2*N; ++i) {
        a = integer_new(i);
        n = (Integer)a;
        if (n->i != i) {
            TRACE(fprintf(stderr, "expected:%d was:%d\n", n->i, i));
            halt("expected n->i == i");
        }
    }
}
