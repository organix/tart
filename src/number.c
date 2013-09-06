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

INTEGER small_integers[2*N_SMALL + N_SMALL + 1] = {
    { { beh_integer, number_match_method }, -128 },
    { { beh_integer, number_match_method }, -127 },
    { { beh_integer, number_match_method }, -126 },
    { { beh_integer, number_match_method }, -125 },
    { { beh_integer, number_match_method }, -124 },
    { { beh_integer, number_match_method }, -123 },
    { { beh_integer, number_match_method }, -122 },
    { { beh_integer, number_match_method }, -121 },
    { { beh_integer, number_match_method }, -120 },
    { { beh_integer, number_match_method }, -119 },
    { { beh_integer, number_match_method }, -118 },
    { { beh_integer, number_match_method }, -117 },
    { { beh_integer, number_match_method }, -116 },
    { { beh_integer, number_match_method }, -115 },
    { { beh_integer, number_match_method }, -114 },
    { { beh_integer, number_match_method }, -113 },
    { { beh_integer, number_match_method }, -112 },
    { { beh_integer, number_match_method }, -111 },
    { { beh_integer, number_match_method }, -110 },
    { { beh_integer, number_match_method }, -109 },
    { { beh_integer, number_match_method }, -108 },
    { { beh_integer, number_match_method }, -107 },
    { { beh_integer, number_match_method }, -106 },
    { { beh_integer, number_match_method }, -105 },
    { { beh_integer, number_match_method }, -104 },
    { { beh_integer, number_match_method }, -103 },
    { { beh_integer, number_match_method }, -102 },
    { { beh_integer, number_match_method }, -101 },
    { { beh_integer, number_match_method }, -100 },
    { { beh_integer, number_match_method }, -99 },
    { { beh_integer, number_match_method }, -98 },
    { { beh_integer, number_match_method }, -97 },
    { { beh_integer, number_match_method }, -96 },
    { { beh_integer, number_match_method }, -95 },
    { { beh_integer, number_match_method }, -94 },
    { { beh_integer, number_match_method }, -93 },
    { { beh_integer, number_match_method }, -92 },
    { { beh_integer, number_match_method }, -91 },
    { { beh_integer, number_match_method }, -90 },
    { { beh_integer, number_match_method }, -89 },
    { { beh_integer, number_match_method }, -88 },
    { { beh_integer, number_match_method }, -87 },
    { { beh_integer, number_match_method }, -86 },
    { { beh_integer, number_match_method }, -85 },
    { { beh_integer, number_match_method }, -84 },
    { { beh_integer, number_match_method }, -83 },
    { { beh_integer, number_match_method }, -82 },
    { { beh_integer, number_match_method }, -81 },
    { { beh_integer, number_match_method }, -80 },
    { { beh_integer, number_match_method }, -79 },
    { { beh_integer, number_match_method }, -78 },
    { { beh_integer, number_match_method }, -77 },
    { { beh_integer, number_match_method }, -76 },
    { { beh_integer, number_match_method }, -75 },
    { { beh_integer, number_match_method }, -74 },
    { { beh_integer, number_match_method }, -73 },
    { { beh_integer, number_match_method }, -72 },
    { { beh_integer, number_match_method }, -71 },
    { { beh_integer, number_match_method }, -70 },
    { { beh_integer, number_match_method }, -69 },
    { { beh_integer, number_match_method }, -68 },
    { { beh_integer, number_match_method }, -67 },
    { { beh_integer, number_match_method }, -66 },
    { { beh_integer, number_match_method }, -65 },
    { { beh_integer, number_match_method }, -64 },
    { { beh_integer, number_match_method }, -63 },
    { { beh_integer, number_match_method }, -62 },
    { { beh_integer, number_match_method }, -61 },
    { { beh_integer, number_match_method }, -60 },
    { { beh_integer, number_match_method }, -59 },
    { { beh_integer, number_match_method }, -58 },
    { { beh_integer, number_match_method }, -57 },
    { { beh_integer, number_match_method }, -56 },
    { { beh_integer, number_match_method }, -55 },
    { { beh_integer, number_match_method }, -54 },
    { { beh_integer, number_match_method }, -53 },
    { { beh_integer, number_match_method }, -52 },
    { { beh_integer, number_match_method }, -51 },
    { { beh_integer, number_match_method }, -50 },
    { { beh_integer, number_match_method }, -49 },
    { { beh_integer, number_match_method }, -48 },
    { { beh_integer, number_match_method }, -47 },
    { { beh_integer, number_match_method }, -46 },
    { { beh_integer, number_match_method }, -45 },
    { { beh_integer, number_match_method }, -44 },
    { { beh_integer, number_match_method }, -43 },
    { { beh_integer, number_match_method }, -42 },
    { { beh_integer, number_match_method }, -41 },
    { { beh_integer, number_match_method }, -40 },
    { { beh_integer, number_match_method }, -39 },
    { { beh_integer, number_match_method }, -38 },
    { { beh_integer, number_match_method }, -37 },
    { { beh_integer, number_match_method }, -36 },
    { { beh_integer, number_match_method }, -35 },
    { { beh_integer, number_match_method }, -34 },
    { { beh_integer, number_match_method }, -33 },
    { { beh_integer, number_match_method }, -32 },
    { { beh_integer, number_match_method }, -31 },
    { { beh_integer, number_match_method }, -30 },
    { { beh_integer, number_match_method }, -29 },
    { { beh_integer, number_match_method }, -28 },
    { { beh_integer, number_match_method }, -27 },
    { { beh_integer, number_match_method }, -26 },
    { { beh_integer, number_match_method }, -25 },
    { { beh_integer, number_match_method }, -24 },
    { { beh_integer, number_match_method }, -23 },
    { { beh_integer, number_match_method }, -22 },
    { { beh_integer, number_match_method }, -21 },
    { { beh_integer, number_match_method }, -20 },
    { { beh_integer, number_match_method }, -19 },
    { { beh_integer, number_match_method }, -18 },
    { { beh_integer, number_match_method }, -17 },
    { { beh_integer, number_match_method }, -16 },
    { { beh_integer, number_match_method }, -15 },
    { { beh_integer, number_match_method }, -14 },
    { { beh_integer, number_match_method }, -13 },
    { { beh_integer, number_match_method }, -12 },
    { { beh_integer, number_match_method }, -11 },
    { { beh_integer, number_match_method }, -10 },
    { { beh_integer, number_match_method }, -9 },
    { { beh_integer, number_match_method }, -8 },
    { { beh_integer, number_match_method }, -7 },
    { { beh_integer, number_match_method }, -6 },
    { { beh_integer, number_match_method }, -5 },
    { { beh_integer, number_match_method }, -4 },
    { { beh_integer, number_match_method }, -3 },
    { { beh_integer, number_match_method }, -2 },
    { { beh_integer, number_match_method }, -1 },
    { { beh_integer, number_match_method }, 0 },
    { { beh_integer, number_match_method }, 1 },
    { { beh_integer, number_match_method }, 2 },
    { { beh_integer, number_match_method }, 3 },
    { { beh_integer, number_match_method }, 4 },
    { { beh_integer, number_match_method }, 5 },
    { { beh_integer, number_match_method }, 6 },
    { { beh_integer, number_match_method }, 7 },
    { { beh_integer, number_match_method }, 8 },
    { { beh_integer, number_match_method }, 9 },
    { { beh_integer, number_match_method }, 10 },
    { { beh_integer, number_match_method }, 11 },
    { { beh_integer, number_match_method }, 12 },
    { { beh_integer, number_match_method }, 13 },
    { { beh_integer, number_match_method }, 14 },
    { { beh_integer, number_match_method }, 15 },
    { { beh_integer, number_match_method }, 16 },
    { { beh_integer, number_match_method }, 17 },
    { { beh_integer, number_match_method }, 18 },
    { { beh_integer, number_match_method }, 19 },
    { { beh_integer, number_match_method }, 20 },
    { { beh_integer, number_match_method }, 21 },
    { { beh_integer, number_match_method }, 22 },
    { { beh_integer, number_match_method }, 23 },
    { { beh_integer, number_match_method }, 24 },
    { { beh_integer, number_match_method }, 25 },
    { { beh_integer, number_match_method }, 26 },
    { { beh_integer, number_match_method }, 27 },
    { { beh_integer, number_match_method }, 28 },
    { { beh_integer, number_match_method }, 29 },
    { { beh_integer, number_match_method }, 30 },
    { { beh_integer, number_match_method }, 31 },
    { { beh_integer, number_match_method }, 32 },
    { { beh_integer, number_match_method }, 33 },
    { { beh_integer, number_match_method }, 34 },
    { { beh_integer, number_match_method }, 35 },
    { { beh_integer, number_match_method }, 36 },
    { { beh_integer, number_match_method }, 37 },
    { { beh_integer, number_match_method }, 38 },
    { { beh_integer, number_match_method }, 39 },
    { { beh_integer, number_match_method }, 40 },
    { { beh_integer, number_match_method }, 41 },
    { { beh_integer, number_match_method }, 42 },
    { { beh_integer, number_match_method }, 43 },
    { { beh_integer, number_match_method }, 44 },
    { { beh_integer, number_match_method }, 45 },
    { { beh_integer, number_match_method }, 46 },
    { { beh_integer, number_match_method }, 47 },
    { { beh_integer, number_match_method }, 48 },
    { { beh_integer, number_match_method }, 49 },
    { { beh_integer, number_match_method }, 50 },
    { { beh_integer, number_match_method }, 51 },
    { { beh_integer, number_match_method }, 52 },
    { { beh_integer, number_match_method }, 53 },
    { { beh_integer, number_match_method }, 54 },
    { { beh_integer, number_match_method }, 55 },
    { { beh_integer, number_match_method }, 56 },
    { { beh_integer, number_match_method }, 57 },
    { { beh_integer, number_match_method }, 58 },
    { { beh_integer, number_match_method }, 59 },
    { { beh_integer, number_match_method }, 60 },
    { { beh_integer, number_match_method }, 61 },
    { { beh_integer, number_match_method }, 62 },
    { { beh_integer, number_match_method }, 63 },
    { { beh_integer, number_match_method }, 64 },
    { { beh_integer, number_match_method }, 65 },
    { { beh_integer, number_match_method }, 66 },
    { { beh_integer, number_match_method }, 67 },
    { { beh_integer, number_match_method }, 68 },
    { { beh_integer, number_match_method }, 69 },
    { { beh_integer, number_match_method }, 70 },
    { { beh_integer, number_match_method }, 71 },
    { { beh_integer, number_match_method }, 72 },
    { { beh_integer, number_match_method }, 73 },
    { { beh_integer, number_match_method }, 74 },
    { { beh_integer, number_match_method }, 75 },
    { { beh_integer, number_match_method }, 76 },
    { { beh_integer, number_match_method }, 77 },
    { { beh_integer, number_match_method }, 78 },
    { { beh_integer, number_match_method }, 79 },
    { { beh_integer, number_match_method }, 80 },
    { { beh_integer, number_match_method }, 81 },
    { { beh_integer, number_match_method }, 82 },
    { { beh_integer, number_match_method }, 83 },
    { { beh_integer, number_match_method }, 84 },
    { { beh_integer, number_match_method }, 85 },
    { { beh_integer, number_match_method }, 86 },
    { { beh_integer, number_match_method }, 87 },
    { { beh_integer, number_match_method }, 88 },
    { { beh_integer, number_match_method }, 89 },
    { { beh_integer, number_match_method }, 90 },
    { { beh_integer, number_match_method }, 91 },
    { { beh_integer, number_match_method }, 92 },
    { { beh_integer, number_match_method }, 93 },
    { { beh_integer, number_match_method }, 94 },
    { { beh_integer, number_match_method }, 95 },
    { { beh_integer, number_match_method }, 96 },
    { { beh_integer, number_match_method }, 97 },
    { { beh_integer, number_match_method }, 98 },
    { { beh_integer, number_match_method }, 99 },
    { { beh_integer, number_match_method }, 100 },
    { { beh_integer, number_match_method }, 101 },
    { { beh_integer, number_match_method }, 102 },
    { { beh_integer, number_match_method }, 103 },
    { { beh_integer, number_match_method }, 104 },
    { { beh_integer, number_match_method }, 105 },
    { { beh_integer, number_match_method }, 106 },
    { { beh_integer, number_match_method }, 107 },
    { { beh_integer, number_match_method }, 108 },
    { { beh_integer, number_match_method }, 109 },
    { { beh_integer, number_match_method }, 110 },
    { { beh_integer, number_match_method }, 111 },
    { { beh_integer, number_match_method }, 112 },
    { { beh_integer, number_match_method }, 113 },
    { { beh_integer, number_match_method }, 114 },
    { { beh_integer, number_match_method }, 115 },
    { { beh_integer, number_match_method }, 116 },
    { { beh_integer, number_match_method }, 117 },
    { { beh_integer, number_match_method }, 118 },
    { { beh_integer, number_match_method }, 119 },
    { { beh_integer, number_match_method }, 120 },
    { { beh_integer, number_match_method }, 121 },
    { { beh_integer, number_match_method }, 122 },
    { { beh_integer, number_match_method }, 123 },
    { { beh_integer, number_match_method }, 124 },
    { { beh_integer, number_match_method }, 125 },
    { { beh_integer, number_match_method }, 126 },
    { { beh_integer, number_match_method }, 127 },
    { { beh_integer, number_match_method }, 128 },
    { { beh_integer, number_match_method }, 129 },
    { { beh_integer, number_match_method }, 130 },
    { { beh_integer, number_match_method }, 131 },
    { { beh_integer, number_match_method }, 132 },
    { { beh_integer, number_match_method }, 133 },
    { { beh_integer, number_match_method }, 134 },
    { { beh_integer, number_match_method }, 135 },
    { { beh_integer, number_match_method }, 136 },
    { { beh_integer, number_match_method }, 137 },
    { { beh_integer, number_match_method }, 138 },
    { { beh_integer, number_match_method }, 139 },
    { { beh_integer, number_match_method }, 140 },
    { { beh_integer, number_match_method }, 141 },
    { { beh_integer, number_match_method }, 142 },
    { { beh_integer, number_match_method }, 143 },
    { { beh_integer, number_match_method }, 144 },
    { { beh_integer, number_match_method }, 145 },
    { { beh_integer, number_match_method }, 146 },
    { { beh_integer, number_match_method }, 147 },
    { { beh_integer, number_match_method }, 148 },
    { { beh_integer, number_match_method }, 149 },
    { { beh_integer, number_match_method }, 150 },
    { { beh_integer, number_match_method }, 151 },
    { { beh_integer, number_match_method }, 152 },
    { { beh_integer, number_match_method }, 153 },
    { { beh_integer, number_match_method }, 154 },
    { { beh_integer, number_match_method }, 155 },
    { { beh_integer, number_match_method }, 156 },
    { { beh_integer, number_match_method }, 157 },
    { { beh_integer, number_match_method }, 158 },
    { { beh_integer, number_match_method }, 159 },
    { { beh_integer, number_match_method }, 160 },
    { { beh_integer, number_match_method }, 161 },
    { { beh_integer, number_match_method }, 162 },
    { { beh_integer, number_match_method }, 163 },
    { { beh_integer, number_match_method }, 164 },
    { { beh_integer, number_match_method }, 165 },
    { { beh_integer, number_match_method }, 166 },
    { { beh_integer, number_match_method }, 167 },
    { { beh_integer, number_match_method }, 168 },
    { { beh_integer, number_match_method }, 169 },
    { { beh_integer, number_match_method }, 170 },
    { { beh_integer, number_match_method }, 171 },
    { { beh_integer, number_match_method }, 172 },
    { { beh_integer, number_match_method }, 173 },
    { { beh_integer, number_match_method }, 174 },
    { { beh_integer, number_match_method }, 175 },
    { { beh_integer, number_match_method }, 176 },
    { { beh_integer, number_match_method }, 177 },
    { { beh_integer, number_match_method }, 178 },
    { { beh_integer, number_match_method }, 179 },
    { { beh_integer, number_match_method }, 180 },
    { { beh_integer, number_match_method }, 181 },
    { { beh_integer, number_match_method }, 182 },
    { { beh_integer, number_match_method }, 183 },
    { { beh_integer, number_match_method }, 184 },
    { { beh_integer, number_match_method }, 185 },
    { { beh_integer, number_match_method }, 186 },
    { { beh_integer, number_match_method }, 187 },
    { { beh_integer, number_match_method }, 188 },
    { { beh_integer, number_match_method }, 189 },
    { { beh_integer, number_match_method }, 190 },
    { { beh_integer, number_match_method }, 191 },
    { { beh_integer, number_match_method }, 192 },
    { { beh_integer, number_match_method }, 193 },
    { { beh_integer, number_match_method }, 194 },
    { { beh_integer, number_match_method }, 195 },
    { { beh_integer, number_match_method }, 196 },
    { { beh_integer, number_match_method }, 197 },
    { { beh_integer, number_match_method }, 198 },
    { { beh_integer, number_match_method }, 199 },
    { { beh_integer, number_match_method }, 200 },
    { { beh_integer, number_match_method }, 201 },
    { { beh_integer, number_match_method }, 202 },
    { { beh_integer, number_match_method }, 203 },
    { { beh_integer, number_match_method }, 204 },
    { { beh_integer, number_match_method }, 205 },
    { { beh_integer, number_match_method }, 206 },
    { { beh_integer, number_match_method }, 207 },
    { { beh_integer, number_match_method }, 208 },
    { { beh_integer, number_match_method }, 209 },
    { { beh_integer, number_match_method }, 210 },
    { { beh_integer, number_match_method }, 211 },
    { { beh_integer, number_match_method }, 212 },
    { { beh_integer, number_match_method }, 213 },
    { { beh_integer, number_match_method }, 214 },
    { { beh_integer, number_match_method }, 215 },
    { { beh_integer, number_match_method }, 216 },
    { { beh_integer, number_match_method }, 217 },
    { { beh_integer, number_match_method }, 218 },
    { { beh_integer, number_match_method }, 219 },
    { { beh_integer, number_match_method }, 220 },
    { { beh_integer, number_match_method }, 221 },
    { { beh_integer, number_match_method }, 222 },
    { { beh_integer, number_match_method }, 223 },
    { { beh_integer, number_match_method }, 224 },
    { { beh_integer, number_match_method }, 225 },
    { { beh_integer, number_match_method }, 226 },
    { { beh_integer, number_match_method }, 227 },
    { { beh_integer, number_match_method }, 228 },
    { { beh_integer, number_match_method }, 229 },
    { { beh_integer, number_match_method }, 230 },
    { { beh_integer, number_match_method }, 231 },
    { { beh_integer, number_match_method }, 232 },
    { { beh_integer, number_match_method }, 233 },
    { { beh_integer, number_match_method }, 234 },
    { { beh_integer, number_match_method }, 235 },
    { { beh_integer, number_match_method }, 236 },
    { { beh_integer, number_match_method }, 237 },
    { { beh_integer, number_match_method }, 238 },
    { { beh_integer, number_match_method }, 239 },
    { { beh_integer, number_match_method }, 240 },
    { { beh_integer, number_match_method }, 241 },
    { { beh_integer, number_match_method }, 242 },
    { { beh_integer, number_match_method }, 243 },
    { { beh_integer, number_match_method }, 244 },
    { { beh_integer, number_match_method }, 245 },
    { { beh_integer, number_match_method }, 246 },
    { { beh_integer, number_match_method }, 247 },
    { { beh_integer, number_match_method }, 248 },
    { { beh_integer, number_match_method }, 249 },
    { { beh_integer, number_match_method }, 250 },
    { { beh_integer, number_match_method }, 251 },
    { { beh_integer, number_match_method }, 252 },
    { { beh_integer, number_match_method }, 253 },
    { { beh_integer, number_match_method }, 254 },
    { { beh_integer, number_match_method }, 255 },
    { { beh_integer, number_match_method }, 256 }
};

inline Actor
integer_new(Config cfg, int i)
{
    if ((i <= 2*N_SMALL) && (i >= -N_SMALL)) {  // check for cached objects
        return ((Actor)(&small_integers[N_SMALL + i]));
    }
    Integer n = (Integer)config_create(cfg, sizeof(INTEGER), beh_integer);
    n->_act.match = number_match_method;  // override match procedure
    n->i = i;
    return (Actor)n;
}

Actor
number_match_method(Config cfg, Actor this, Actor that)
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
number_diff_method(Config cfg, Actor this, Actor that)
{
    if (beh_integer != BEH(this)) { halt("number_diff_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        return integer_new(cfg, n->i - m->i);
    }
    return this;
}

inline Actor
number_plus_method(Config cfg, Actor this, Actor that)
{
    if (beh_integer != BEH(this)) { halt("number_plus_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        return integer_new(cfg, n->i + m->i);
    }
    return this;
}

inline Actor
number_times_method(Config cfg, Actor this, Actor that)
{
    if (beh_integer != BEH(this)) { halt("number_times_method: number required"); }
    Integer n = (Integer)this;
    if (beh_integer == BEH(that)) {
        Integer m = (Integer)that;
        return integer_new(cfg, n->i * m->i);
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

inline uint32_t
u32_getbits(uint8_t* p, int n, int m)
{
    p += (n >> 3);  // position p at first byte in this bit range
    int lb = n & 0x7;  // number of bits in the left part
    int rb = (8 - lb);  // number of bits in the right part
    int rm = 0xFF >> lb;  // mask for right bits
    uint32_t b = *p++;
    b &= rm;
    if (m <= rb) {
        b >>= (rb - m);
        return b;  // all bits present in first byte
    }
    m -= rb;
    while (m >= 8) {
        b <<= 8;
        b |= *p++;
        m -= 8;
    }
    lb = (8 - m);  // left-over bits
    b <<= m;
    b |= (*p >> lb);
    return b;
}

void
beh_integer(Event e)
{
    TRACE(fprintf(stderr, "beh_integer{event=%p}\n", e));
    halt("HALT!");
}

void
test_getbits(int n, int m, uint32_t expect)
{
    uint8_t buf[] = { 0, 0, 0, 0, 0, 0 };
    uint8_t b = (0x80 >> (n & 0x7));
    buf[n >> 3] |= b;
    int x = n + m - 1;  // last bit position
    b = (0x80 >> (x & 0x7));
    buf[x >> 3] |= b;
    uint32_t actual = u32_getbits(buf, n, m);
    TRACE(fprintf(stderr, "test_getbits{n=%d, m=%d, buf=%02x%02x%02x%02x%02x%02x, expect=%08x, actual=%08x}\n",
        n, m, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], expect, actual));
    if (expect != actual) { halt("test_getbits() failed"); }
}

void
test_number()
{
    Actor a, b;
    Integer n, m;
    int i;

    TRACE(fprintf(stderr, "---- test_number ----\n"));
    Config cfg = config_new(a_root_config);
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    TRACE(fprintf(stderr, "a_zero = %p\n", a_zero));
    a = integer_new(cfg, 0);
    if (a_zero != a) { halt("expected a_zero == a"); }
    if (number_match_method(cfg, a, a_zero) != a_true) { halt("expected number_match_method(a, a_zero) == a_true"); }
    a = number_plus_method(cfg, a, a_one);
    if (a_one != a) { halt("expected a_one == a"); }
    a = number_diff_method(cfg, a, a_two);
    if (a_minus_one != a) { halt("expected a_minus_one == a"); }
/*
*/
    a = integer_new(cfg, 100);
    a = number_times_method(cfg, a, a);
    TRACE(fprintf(stderr, "a = %p\n", a));
    if (beh_integer != BEH(a)) { halt("expected beh_integer == BEH(a)"); }
    n = (Integer)a;
    TRACE(fprintf(stderr, "n->i = %d\n", n->i));
    b = integer_new(cfg, 100 * 100);
    TRACE(fprintf(stderr, "b = %p\n", b));
    if (a == b) { halt("expected a != b"); }
    if (beh_integer != BEH(b)) { halt("expected beh_integer == BEH(b)"); }
    m = (Integer)b;
    TRACE(fprintf(stderr, "m->i = %d\n", m->i));
    if (n->i != m->i) { halt("expected n->i == m->i"); }
    if (number_match_method(cfg, a, b) != a_true) { halt("expected number_match_method(a, b) == a_true"); }
/*
*/
    a = integer_new(cfg, N_SMALL);
    b = number_times_method(cfg, a, a_minus_one);
    n = (Integer)b;
    TRACE(fprintf(stderr, "n->i = %d\n", n->i));
    if (b != integer_new(cfg, -N_SMALL)) { halt("expected b == integer_new(-N_SMALL)"); }
    a = number_times_method(cfg, a, a_two);
    a = number_plus_method(cfg, a, a_minus_one);
    m = (Integer)a;
    TRACE(fprintf(stderr, "m->i = %d\n", m->i));
    if (a != integer_new(cfg, 2*N_SMALL - 1)) { halt("expected a == integer_new(2*N_SMALL - 1)"); }
    a = number_plus_method(cfg, a, a_one);
    m = (Integer)a;
    TRACE(fprintf(stderr, "m->i = %d\n", m->i));
    if (a != integer_new(cfg, 2*N_SMALL)) { halt("expected a == integer_new(2*N_SMALL)"); }
/*
*/
    for (i = -N_SMALL; i <= 2*N_SMALL; ++i) {
        a = integer_new(cfg, i);
        n = (Integer)a;
        if (n->i != i) {
            TRACE(fprintf(stderr, "expected:%d was:%d\n", n->i, i));
            halt("expected n->i == i");
        }
    }
/*
*/
    test_getbits(0, 1, 0x00000001);
    test_getbits(0, 2, 0x00000003);
    test_getbits(0, 3, 0x00000005);
    test_getbits(0, 4, 0x00000009);
    test_getbits(0, 5, 0x00000011);
    test_getbits(0, 6, 0x00000021);
    test_getbits(0, 7, 0x00000041);
    test_getbits(0, 8, 0x00000081);
    test_getbits(1, 7, 0x00000041);
    test_getbits(2, 6, 0x00000021);
    test_getbits(3, 5, 0x00000011);
    test_getbits(4, 4, 0x00000009);
    test_getbits(5, 3, 0x00000005);
    test_getbits(6, 2, 0x00000003);
    test_getbits(7, 1, 0x00000001);

    test_getbits(8, 1, 0x00000001);
    test_getbits(8, 2, 0x00000003);
    test_getbits(8, 3, 0x00000005);
    test_getbits(8, 4, 0x00000009);
    test_getbits(8, 5, 0x00000011);
    test_getbits(8, 6, 0x00000021);
    test_getbits(8, 7, 0x00000041);
    test_getbits(8, 8, 0x00000081);
    test_getbits(8, 9, 0x00000101);
    test_getbits(8, 10, 0x00000201);
    test_getbits(8, 11, 0x00000401);
    test_getbits(8, 12, 0x00000801);
    test_getbits(8, 13, 0x00001001);
    test_getbits(8, 14, 0x00002001);
    test_getbits(8, 15, 0x00004001);
    test_getbits(8, 16, 0x00008001);
    test_getbits(8, 17, 0x00010001);
    test_getbits(8, 18, 0x00020001);
    test_getbits(8, 19, 0x00040001);
    test_getbits(8, 20, 0x00080001);
    test_getbits(8, 21, 0x00100001);
    test_getbits(8, 22, 0x00200001);
    test_getbits(8, 23, 0x00400001);
    test_getbits(8, 24, 0x00800001);
    test_getbits(8, 25, 0x01000001);
    test_getbits(8, 26, 0x02000001);
    test_getbits(8, 27, 0x04000001);
    test_getbits(8, 28, 0x08000001);
    test_getbits(8, 29, 0x10000001);
    test_getbits(8, 30, 0x20000001);
    test_getbits(8, 31, 0x40000001);

    test_getbits(8, 32, 0x80000001);
    test_getbits(9, 32, 0x80000001);
    test_getbits(10, 32, 0x80000001);
    test_getbits(11, 32, 0x80000001);
    test_getbits(12, 32, 0x80000001);
    test_getbits(13, 32, 0x80000001);
    test_getbits(14, 32, 0x80000001);
    test_getbits(15, 32, 0x80000001);
    test_getbits(16, 32, 0x80000001);
}
