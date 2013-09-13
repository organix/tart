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

#define INTEGER_ACTOR_DECL(beh)     { (beh), number_eqv_method }
INTEGER small_integers[2*N_SMALL + N_SMALL + 1] = {
    { INTEGER_ACTOR_DECL(beh_integer), -128 },
    { INTEGER_ACTOR_DECL(beh_integer), -127 },
    { INTEGER_ACTOR_DECL(beh_integer), -126 },
    { INTEGER_ACTOR_DECL(beh_integer), -125 },
    { INTEGER_ACTOR_DECL(beh_integer), -124 },
    { INTEGER_ACTOR_DECL(beh_integer), -123 },
    { INTEGER_ACTOR_DECL(beh_integer), -122 },
    { INTEGER_ACTOR_DECL(beh_integer), -121 },
    { INTEGER_ACTOR_DECL(beh_integer), -120 },
    { INTEGER_ACTOR_DECL(beh_integer), -119 },
    { INTEGER_ACTOR_DECL(beh_integer), -118 },
    { INTEGER_ACTOR_DECL(beh_integer), -117 },
    { INTEGER_ACTOR_DECL(beh_integer), -116 },
    { INTEGER_ACTOR_DECL(beh_integer), -115 },
    { INTEGER_ACTOR_DECL(beh_integer), -114 },
    { INTEGER_ACTOR_DECL(beh_integer), -113 },
    { INTEGER_ACTOR_DECL(beh_integer), -112 },
    { INTEGER_ACTOR_DECL(beh_integer), -111 },
    { INTEGER_ACTOR_DECL(beh_integer), -110 },
    { INTEGER_ACTOR_DECL(beh_integer), -109 },
    { INTEGER_ACTOR_DECL(beh_integer), -108 },
    { INTEGER_ACTOR_DECL(beh_integer), -107 },
    { INTEGER_ACTOR_DECL(beh_integer), -106 },
    { INTEGER_ACTOR_DECL(beh_integer), -105 },
    { INTEGER_ACTOR_DECL(beh_integer), -104 },
    { INTEGER_ACTOR_DECL(beh_integer), -103 },
    { INTEGER_ACTOR_DECL(beh_integer), -102 },
    { INTEGER_ACTOR_DECL(beh_integer), -101 },
    { INTEGER_ACTOR_DECL(beh_integer), -100 },
    { INTEGER_ACTOR_DECL(beh_integer), -99 },
    { INTEGER_ACTOR_DECL(beh_integer), -98 },
    { INTEGER_ACTOR_DECL(beh_integer), -97 },
    { INTEGER_ACTOR_DECL(beh_integer), -96 },
    { INTEGER_ACTOR_DECL(beh_integer), -95 },
    { INTEGER_ACTOR_DECL(beh_integer), -94 },
    { INTEGER_ACTOR_DECL(beh_integer), -93 },
    { INTEGER_ACTOR_DECL(beh_integer), -92 },
    { INTEGER_ACTOR_DECL(beh_integer), -91 },
    { INTEGER_ACTOR_DECL(beh_integer), -90 },
    { INTEGER_ACTOR_DECL(beh_integer), -89 },
    { INTEGER_ACTOR_DECL(beh_integer), -88 },
    { INTEGER_ACTOR_DECL(beh_integer), -87 },
    { INTEGER_ACTOR_DECL(beh_integer), -86 },
    { INTEGER_ACTOR_DECL(beh_integer), -85 },
    { INTEGER_ACTOR_DECL(beh_integer), -84 },
    { INTEGER_ACTOR_DECL(beh_integer), -83 },
    { INTEGER_ACTOR_DECL(beh_integer), -82 },
    { INTEGER_ACTOR_DECL(beh_integer), -81 },
    { INTEGER_ACTOR_DECL(beh_integer), -80 },
    { INTEGER_ACTOR_DECL(beh_integer), -79 },
    { INTEGER_ACTOR_DECL(beh_integer), -78 },
    { INTEGER_ACTOR_DECL(beh_integer), -77 },
    { INTEGER_ACTOR_DECL(beh_integer), -76 },
    { INTEGER_ACTOR_DECL(beh_integer), -75 },
    { INTEGER_ACTOR_DECL(beh_integer), -74 },
    { INTEGER_ACTOR_DECL(beh_integer), -73 },
    { INTEGER_ACTOR_DECL(beh_integer), -72 },
    { INTEGER_ACTOR_DECL(beh_integer), -71 },
    { INTEGER_ACTOR_DECL(beh_integer), -70 },
    { INTEGER_ACTOR_DECL(beh_integer), -69 },
    { INTEGER_ACTOR_DECL(beh_integer), -68 },
    { INTEGER_ACTOR_DECL(beh_integer), -67 },
    { INTEGER_ACTOR_DECL(beh_integer), -66 },
    { INTEGER_ACTOR_DECL(beh_integer), -65 },
    { INTEGER_ACTOR_DECL(beh_integer), -64 },
    { INTEGER_ACTOR_DECL(beh_integer), -63 },
    { INTEGER_ACTOR_DECL(beh_integer), -62 },
    { INTEGER_ACTOR_DECL(beh_integer), -61 },
    { INTEGER_ACTOR_DECL(beh_integer), -60 },
    { INTEGER_ACTOR_DECL(beh_integer), -59 },
    { INTEGER_ACTOR_DECL(beh_integer), -58 },
    { INTEGER_ACTOR_DECL(beh_integer), -57 },
    { INTEGER_ACTOR_DECL(beh_integer), -56 },
    { INTEGER_ACTOR_DECL(beh_integer), -55 },
    { INTEGER_ACTOR_DECL(beh_integer), -54 },
    { INTEGER_ACTOR_DECL(beh_integer), -53 },
    { INTEGER_ACTOR_DECL(beh_integer), -52 },
    { INTEGER_ACTOR_DECL(beh_integer), -51 },
    { INTEGER_ACTOR_DECL(beh_integer), -50 },
    { INTEGER_ACTOR_DECL(beh_integer), -49 },
    { INTEGER_ACTOR_DECL(beh_integer), -48 },
    { INTEGER_ACTOR_DECL(beh_integer), -47 },
    { INTEGER_ACTOR_DECL(beh_integer), -46 },
    { INTEGER_ACTOR_DECL(beh_integer), -45 },
    { INTEGER_ACTOR_DECL(beh_integer), -44 },
    { INTEGER_ACTOR_DECL(beh_integer), -43 },
    { INTEGER_ACTOR_DECL(beh_integer), -42 },
    { INTEGER_ACTOR_DECL(beh_integer), -41 },
    { INTEGER_ACTOR_DECL(beh_integer), -40 },
    { INTEGER_ACTOR_DECL(beh_integer), -39 },
    { INTEGER_ACTOR_DECL(beh_integer), -38 },
    { INTEGER_ACTOR_DECL(beh_integer), -37 },
    { INTEGER_ACTOR_DECL(beh_integer), -36 },
    { INTEGER_ACTOR_DECL(beh_integer), -35 },
    { INTEGER_ACTOR_DECL(beh_integer), -34 },
    { INTEGER_ACTOR_DECL(beh_integer), -33 },
    { INTEGER_ACTOR_DECL(beh_integer), -32 },
    { INTEGER_ACTOR_DECL(beh_integer), -31 },
    { INTEGER_ACTOR_DECL(beh_integer), -30 },
    { INTEGER_ACTOR_DECL(beh_integer), -29 },
    { INTEGER_ACTOR_DECL(beh_integer), -28 },
    { INTEGER_ACTOR_DECL(beh_integer), -27 },
    { INTEGER_ACTOR_DECL(beh_integer), -26 },
    { INTEGER_ACTOR_DECL(beh_integer), -25 },
    { INTEGER_ACTOR_DECL(beh_integer), -24 },
    { INTEGER_ACTOR_DECL(beh_integer), -23 },
    { INTEGER_ACTOR_DECL(beh_integer), -22 },
    { INTEGER_ACTOR_DECL(beh_integer), -21 },
    { INTEGER_ACTOR_DECL(beh_integer), -20 },
    { INTEGER_ACTOR_DECL(beh_integer), -19 },
    { INTEGER_ACTOR_DECL(beh_integer), -18 },
    { INTEGER_ACTOR_DECL(beh_integer), -17 },
    { INTEGER_ACTOR_DECL(beh_integer), -16 },
    { INTEGER_ACTOR_DECL(beh_integer), -15 },
    { INTEGER_ACTOR_DECL(beh_integer), -14 },
    { INTEGER_ACTOR_DECL(beh_integer), -13 },
    { INTEGER_ACTOR_DECL(beh_integer), -12 },
    { INTEGER_ACTOR_DECL(beh_integer), -11 },
    { INTEGER_ACTOR_DECL(beh_integer), -10 },
    { INTEGER_ACTOR_DECL(beh_integer), -9 },
    { INTEGER_ACTOR_DECL(beh_integer), -8 },
    { INTEGER_ACTOR_DECL(beh_integer), -7 },
    { INTEGER_ACTOR_DECL(beh_integer), -6 },
    { INTEGER_ACTOR_DECL(beh_integer), -5 },
    { INTEGER_ACTOR_DECL(beh_integer), -4 },
    { INTEGER_ACTOR_DECL(beh_integer), -3 },
    { INTEGER_ACTOR_DECL(beh_integer), -2 },
    { INTEGER_ACTOR_DECL(beh_integer), -1 },
    { INTEGER_ACTOR_DECL(beh_integer), 0 },
    { INTEGER_ACTOR_DECL(beh_integer), 1 },
    { INTEGER_ACTOR_DECL(beh_integer), 2 },
    { INTEGER_ACTOR_DECL(beh_integer), 3 },
    { INTEGER_ACTOR_DECL(beh_integer), 4 },
    { INTEGER_ACTOR_DECL(beh_integer), 5 },
    { INTEGER_ACTOR_DECL(beh_integer), 6 },
    { INTEGER_ACTOR_DECL(beh_integer), 7 },
    { INTEGER_ACTOR_DECL(beh_integer), 8 },
    { INTEGER_ACTOR_DECL(beh_integer), 9 },
    { INTEGER_ACTOR_DECL(beh_integer), 10 },
    { INTEGER_ACTOR_DECL(beh_integer), 11 },
    { INTEGER_ACTOR_DECL(beh_integer), 12 },
    { INTEGER_ACTOR_DECL(beh_integer), 13 },
    { INTEGER_ACTOR_DECL(beh_integer), 14 },
    { INTEGER_ACTOR_DECL(beh_integer), 15 },
    { INTEGER_ACTOR_DECL(beh_integer), 16 },
    { INTEGER_ACTOR_DECL(beh_integer), 17 },
    { INTEGER_ACTOR_DECL(beh_integer), 18 },
    { INTEGER_ACTOR_DECL(beh_integer), 19 },
    { INTEGER_ACTOR_DECL(beh_integer), 20 },
    { INTEGER_ACTOR_DECL(beh_integer), 21 },
    { INTEGER_ACTOR_DECL(beh_integer), 22 },
    { INTEGER_ACTOR_DECL(beh_integer), 23 },
    { INTEGER_ACTOR_DECL(beh_integer), 24 },
    { INTEGER_ACTOR_DECL(beh_integer), 25 },
    { INTEGER_ACTOR_DECL(beh_integer), 26 },
    { INTEGER_ACTOR_DECL(beh_integer), 27 },
    { INTEGER_ACTOR_DECL(beh_integer), 28 },
    { INTEGER_ACTOR_DECL(beh_integer), 29 },
    { INTEGER_ACTOR_DECL(beh_integer), 30 },
    { INTEGER_ACTOR_DECL(beh_integer), 31 },
    { INTEGER_ACTOR_DECL(beh_integer), 32 },
    { INTEGER_ACTOR_DECL(beh_integer), 33 },
    { INTEGER_ACTOR_DECL(beh_integer), 34 },
    { INTEGER_ACTOR_DECL(beh_integer), 35 },
    { INTEGER_ACTOR_DECL(beh_integer), 36 },
    { INTEGER_ACTOR_DECL(beh_integer), 37 },
    { INTEGER_ACTOR_DECL(beh_integer), 38 },
    { INTEGER_ACTOR_DECL(beh_integer), 39 },
    { INTEGER_ACTOR_DECL(beh_integer), 40 },
    { INTEGER_ACTOR_DECL(beh_integer), 41 },
    { INTEGER_ACTOR_DECL(beh_integer), 42 },
    { INTEGER_ACTOR_DECL(beh_integer), 43 },
    { INTEGER_ACTOR_DECL(beh_integer), 44 },
    { INTEGER_ACTOR_DECL(beh_integer), 45 },
    { INTEGER_ACTOR_DECL(beh_integer), 46 },
    { INTEGER_ACTOR_DECL(beh_integer), 47 },
    { INTEGER_ACTOR_DECL(beh_integer), 48 },
    { INTEGER_ACTOR_DECL(beh_integer), 49 },
    { INTEGER_ACTOR_DECL(beh_integer), 50 },
    { INTEGER_ACTOR_DECL(beh_integer), 51 },
    { INTEGER_ACTOR_DECL(beh_integer), 52 },
    { INTEGER_ACTOR_DECL(beh_integer), 53 },
    { INTEGER_ACTOR_DECL(beh_integer), 54 },
    { INTEGER_ACTOR_DECL(beh_integer), 55 },
    { INTEGER_ACTOR_DECL(beh_integer), 56 },
    { INTEGER_ACTOR_DECL(beh_integer), 57 },
    { INTEGER_ACTOR_DECL(beh_integer), 58 },
    { INTEGER_ACTOR_DECL(beh_integer), 59 },
    { INTEGER_ACTOR_DECL(beh_integer), 60 },
    { INTEGER_ACTOR_DECL(beh_integer), 61 },
    { INTEGER_ACTOR_DECL(beh_integer), 62 },
    { INTEGER_ACTOR_DECL(beh_integer), 63 },
    { INTEGER_ACTOR_DECL(beh_integer), 64 },
    { INTEGER_ACTOR_DECL(beh_integer), 65 },
    { INTEGER_ACTOR_DECL(beh_integer), 66 },
    { INTEGER_ACTOR_DECL(beh_integer), 67 },
    { INTEGER_ACTOR_DECL(beh_integer), 68 },
    { INTEGER_ACTOR_DECL(beh_integer), 69 },
    { INTEGER_ACTOR_DECL(beh_integer), 70 },
    { INTEGER_ACTOR_DECL(beh_integer), 71 },
    { INTEGER_ACTOR_DECL(beh_integer), 72 },
    { INTEGER_ACTOR_DECL(beh_integer), 73 },
    { INTEGER_ACTOR_DECL(beh_integer), 74 },
    { INTEGER_ACTOR_DECL(beh_integer), 75 },
    { INTEGER_ACTOR_DECL(beh_integer), 76 },
    { INTEGER_ACTOR_DECL(beh_integer), 77 },
    { INTEGER_ACTOR_DECL(beh_integer), 78 },
    { INTEGER_ACTOR_DECL(beh_integer), 79 },
    { INTEGER_ACTOR_DECL(beh_integer), 80 },
    { INTEGER_ACTOR_DECL(beh_integer), 81 },
    { INTEGER_ACTOR_DECL(beh_integer), 82 },
    { INTEGER_ACTOR_DECL(beh_integer), 83 },
    { INTEGER_ACTOR_DECL(beh_integer), 84 },
    { INTEGER_ACTOR_DECL(beh_integer), 85 },
    { INTEGER_ACTOR_DECL(beh_integer), 86 },
    { INTEGER_ACTOR_DECL(beh_integer), 87 },
    { INTEGER_ACTOR_DECL(beh_integer), 88 },
    { INTEGER_ACTOR_DECL(beh_integer), 89 },
    { INTEGER_ACTOR_DECL(beh_integer), 90 },
    { INTEGER_ACTOR_DECL(beh_integer), 91 },
    { INTEGER_ACTOR_DECL(beh_integer), 92 },
    { INTEGER_ACTOR_DECL(beh_integer), 93 },
    { INTEGER_ACTOR_DECL(beh_integer), 94 },
    { INTEGER_ACTOR_DECL(beh_integer), 95 },
    { INTEGER_ACTOR_DECL(beh_integer), 96 },
    { INTEGER_ACTOR_DECL(beh_integer), 97 },
    { INTEGER_ACTOR_DECL(beh_integer), 98 },
    { INTEGER_ACTOR_DECL(beh_integer), 99 },
    { INTEGER_ACTOR_DECL(beh_integer), 100 },
    { INTEGER_ACTOR_DECL(beh_integer), 101 },
    { INTEGER_ACTOR_DECL(beh_integer), 102 },
    { INTEGER_ACTOR_DECL(beh_integer), 103 },
    { INTEGER_ACTOR_DECL(beh_integer), 104 },
    { INTEGER_ACTOR_DECL(beh_integer), 105 },
    { INTEGER_ACTOR_DECL(beh_integer), 106 },
    { INTEGER_ACTOR_DECL(beh_integer), 107 },
    { INTEGER_ACTOR_DECL(beh_integer), 108 },
    { INTEGER_ACTOR_DECL(beh_integer), 109 },
    { INTEGER_ACTOR_DECL(beh_integer), 110 },
    { INTEGER_ACTOR_DECL(beh_integer), 111 },
    { INTEGER_ACTOR_DECL(beh_integer), 112 },
    { INTEGER_ACTOR_DECL(beh_integer), 113 },
    { INTEGER_ACTOR_DECL(beh_integer), 114 },
    { INTEGER_ACTOR_DECL(beh_integer), 115 },
    { INTEGER_ACTOR_DECL(beh_integer), 116 },
    { INTEGER_ACTOR_DECL(beh_integer), 117 },
    { INTEGER_ACTOR_DECL(beh_integer), 118 },
    { INTEGER_ACTOR_DECL(beh_integer), 119 },
    { INTEGER_ACTOR_DECL(beh_integer), 120 },
    { INTEGER_ACTOR_DECL(beh_integer), 121 },
    { INTEGER_ACTOR_DECL(beh_integer), 122 },
    { INTEGER_ACTOR_DECL(beh_integer), 123 },
    { INTEGER_ACTOR_DECL(beh_integer), 124 },
    { INTEGER_ACTOR_DECL(beh_integer), 125 },
    { INTEGER_ACTOR_DECL(beh_integer), 126 },
    { INTEGER_ACTOR_DECL(beh_integer), 127 },
    { INTEGER_ACTOR_DECL(beh_integer), 128 },
    { INTEGER_ACTOR_DECL(beh_integer), 129 },
    { INTEGER_ACTOR_DECL(beh_integer), 130 },
    { INTEGER_ACTOR_DECL(beh_integer), 131 },
    { INTEGER_ACTOR_DECL(beh_integer), 132 },
    { INTEGER_ACTOR_DECL(beh_integer), 133 },
    { INTEGER_ACTOR_DECL(beh_integer), 134 },
    { INTEGER_ACTOR_DECL(beh_integer), 135 },
    { INTEGER_ACTOR_DECL(beh_integer), 136 },
    { INTEGER_ACTOR_DECL(beh_integer), 137 },
    { INTEGER_ACTOR_DECL(beh_integer), 138 },
    { INTEGER_ACTOR_DECL(beh_integer), 139 },
    { INTEGER_ACTOR_DECL(beh_integer), 140 },
    { INTEGER_ACTOR_DECL(beh_integer), 141 },
    { INTEGER_ACTOR_DECL(beh_integer), 142 },
    { INTEGER_ACTOR_DECL(beh_integer), 143 },
    { INTEGER_ACTOR_DECL(beh_integer), 144 },
    { INTEGER_ACTOR_DECL(beh_integer), 145 },
    { INTEGER_ACTOR_DECL(beh_integer), 146 },
    { INTEGER_ACTOR_DECL(beh_integer), 147 },
    { INTEGER_ACTOR_DECL(beh_integer), 148 },
    { INTEGER_ACTOR_DECL(beh_integer), 149 },
    { INTEGER_ACTOR_DECL(beh_integer), 150 },
    { INTEGER_ACTOR_DECL(beh_integer), 151 },
    { INTEGER_ACTOR_DECL(beh_integer), 152 },
    { INTEGER_ACTOR_DECL(beh_integer), 153 },
    { INTEGER_ACTOR_DECL(beh_integer), 154 },
    { INTEGER_ACTOR_DECL(beh_integer), 155 },
    { INTEGER_ACTOR_DECL(beh_integer), 156 },
    { INTEGER_ACTOR_DECL(beh_integer), 157 },
    { INTEGER_ACTOR_DECL(beh_integer), 158 },
    { INTEGER_ACTOR_DECL(beh_integer), 159 },
    { INTEGER_ACTOR_DECL(beh_integer), 160 },
    { INTEGER_ACTOR_DECL(beh_integer), 161 },
    { INTEGER_ACTOR_DECL(beh_integer), 162 },
    { INTEGER_ACTOR_DECL(beh_integer), 163 },
    { INTEGER_ACTOR_DECL(beh_integer), 164 },
    { INTEGER_ACTOR_DECL(beh_integer), 165 },
    { INTEGER_ACTOR_DECL(beh_integer), 166 },
    { INTEGER_ACTOR_DECL(beh_integer), 167 },
    { INTEGER_ACTOR_DECL(beh_integer), 168 },
    { INTEGER_ACTOR_DECL(beh_integer), 169 },
    { INTEGER_ACTOR_DECL(beh_integer), 170 },
    { INTEGER_ACTOR_DECL(beh_integer), 171 },
    { INTEGER_ACTOR_DECL(beh_integer), 172 },
    { INTEGER_ACTOR_DECL(beh_integer), 173 },
    { INTEGER_ACTOR_DECL(beh_integer), 174 },
    { INTEGER_ACTOR_DECL(beh_integer), 175 },
    { INTEGER_ACTOR_DECL(beh_integer), 176 },
    { INTEGER_ACTOR_DECL(beh_integer), 177 },
    { INTEGER_ACTOR_DECL(beh_integer), 178 },
    { INTEGER_ACTOR_DECL(beh_integer), 179 },
    { INTEGER_ACTOR_DECL(beh_integer), 180 },
    { INTEGER_ACTOR_DECL(beh_integer), 181 },
    { INTEGER_ACTOR_DECL(beh_integer), 182 },
    { INTEGER_ACTOR_DECL(beh_integer), 183 },
    { INTEGER_ACTOR_DECL(beh_integer), 184 },
    { INTEGER_ACTOR_DECL(beh_integer), 185 },
    { INTEGER_ACTOR_DECL(beh_integer), 186 },
    { INTEGER_ACTOR_DECL(beh_integer), 187 },
    { INTEGER_ACTOR_DECL(beh_integer), 188 },
    { INTEGER_ACTOR_DECL(beh_integer), 189 },
    { INTEGER_ACTOR_DECL(beh_integer), 190 },
    { INTEGER_ACTOR_DECL(beh_integer), 191 },
    { INTEGER_ACTOR_DECL(beh_integer), 192 },
    { INTEGER_ACTOR_DECL(beh_integer), 193 },
    { INTEGER_ACTOR_DECL(beh_integer), 194 },
    { INTEGER_ACTOR_DECL(beh_integer), 195 },
    { INTEGER_ACTOR_DECL(beh_integer), 196 },
    { INTEGER_ACTOR_DECL(beh_integer), 197 },
    { INTEGER_ACTOR_DECL(beh_integer), 198 },
    { INTEGER_ACTOR_DECL(beh_integer), 199 },
    { INTEGER_ACTOR_DECL(beh_integer), 200 },
    { INTEGER_ACTOR_DECL(beh_integer), 201 },
    { INTEGER_ACTOR_DECL(beh_integer), 202 },
    { INTEGER_ACTOR_DECL(beh_integer), 203 },
    { INTEGER_ACTOR_DECL(beh_integer), 204 },
    { INTEGER_ACTOR_DECL(beh_integer), 205 },
    { INTEGER_ACTOR_DECL(beh_integer), 206 },
    { INTEGER_ACTOR_DECL(beh_integer), 207 },
    { INTEGER_ACTOR_DECL(beh_integer), 208 },
    { INTEGER_ACTOR_DECL(beh_integer), 209 },
    { INTEGER_ACTOR_DECL(beh_integer), 210 },
    { INTEGER_ACTOR_DECL(beh_integer), 211 },
    { INTEGER_ACTOR_DECL(beh_integer), 212 },
    { INTEGER_ACTOR_DECL(beh_integer), 213 },
    { INTEGER_ACTOR_DECL(beh_integer), 214 },
    { INTEGER_ACTOR_DECL(beh_integer), 215 },
    { INTEGER_ACTOR_DECL(beh_integer), 216 },
    { INTEGER_ACTOR_DECL(beh_integer), 217 },
    { INTEGER_ACTOR_DECL(beh_integer), 218 },
    { INTEGER_ACTOR_DECL(beh_integer), 219 },
    { INTEGER_ACTOR_DECL(beh_integer), 220 },
    { INTEGER_ACTOR_DECL(beh_integer), 221 },
    { INTEGER_ACTOR_DECL(beh_integer), 222 },
    { INTEGER_ACTOR_DECL(beh_integer), 223 },
    { INTEGER_ACTOR_DECL(beh_integer), 224 },
    { INTEGER_ACTOR_DECL(beh_integer), 225 },
    { INTEGER_ACTOR_DECL(beh_integer), 226 },
    { INTEGER_ACTOR_DECL(beh_integer), 227 },
    { INTEGER_ACTOR_DECL(beh_integer), 228 },
    { INTEGER_ACTOR_DECL(beh_integer), 229 },
    { INTEGER_ACTOR_DECL(beh_integer), 230 },
    { INTEGER_ACTOR_DECL(beh_integer), 231 },
    { INTEGER_ACTOR_DECL(beh_integer), 232 },
    { INTEGER_ACTOR_DECL(beh_integer), 233 },
    { INTEGER_ACTOR_DECL(beh_integer), 234 },
    { INTEGER_ACTOR_DECL(beh_integer), 235 },
    { INTEGER_ACTOR_DECL(beh_integer), 236 },
    { INTEGER_ACTOR_DECL(beh_integer), 237 },
    { INTEGER_ACTOR_DECL(beh_integer), 238 },
    { INTEGER_ACTOR_DECL(beh_integer), 239 },
    { INTEGER_ACTOR_DECL(beh_integer), 240 },
    { INTEGER_ACTOR_DECL(beh_integer), 241 },
    { INTEGER_ACTOR_DECL(beh_integer), 242 },
    { INTEGER_ACTOR_DECL(beh_integer), 243 },
    { INTEGER_ACTOR_DECL(beh_integer), 244 },
    { INTEGER_ACTOR_DECL(beh_integer), 245 },
    { INTEGER_ACTOR_DECL(beh_integer), 246 },
    { INTEGER_ACTOR_DECL(beh_integer), 247 },
    { INTEGER_ACTOR_DECL(beh_integer), 248 },
    { INTEGER_ACTOR_DECL(beh_integer), 249 },
    { INTEGER_ACTOR_DECL(beh_integer), 250 },
    { INTEGER_ACTOR_DECL(beh_integer), 251 },
    { INTEGER_ACTOR_DECL(beh_integer), 252 },
    { INTEGER_ACTOR_DECL(beh_integer), 253 },
    { INTEGER_ACTOR_DECL(beh_integer), 254 },
    { INTEGER_ACTOR_DECL(beh_integer), 255 },
    { INTEGER_ACTOR_DECL(beh_integer), 256 }
};

inline Actor
integer_new(Config cfg, int i)
{
    if ((i <= 2*N_SMALL) && (i >= -N_SMALL)) {  // check for cached objects
        return ((Actor)(&small_integers[N_SMALL + i]));
    }
    Integer n = (Integer)config_create(cfg, sizeof(INTEGER), beh_integer);
    n->_act.eqv = number_eqv_method;  // override eqv procedure
    n->i = i;
    return (Actor)n;
}

Actor
number_eqv_method(Config cfg, Actor this, Actor that)
{
    if (this == that) {
        return a_true;
    }
    if (beh_integer != BEH(this)) { config_fail(cfg, e_inval); }  // number required
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
    if (beh_integer != BEH(this)) { config_fail(cfg, e_inval); }  // number required
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
    if (beh_integer != BEH(this)) { config_fail(cfg, e_inval); }  // number required
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
    if (beh_integer != BEH(this)) { config_fail(cfg, e_inval); }  // number required
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
    beh_halt(e);
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
    Config cfg = quota_config_new(a_root_config, 1000);
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    TRACE(fprintf(stderr, "a_zero = %p\n", a_zero));
    a = integer_new(cfg, 0);
    if (a_zero != a) { halt("expected a_zero == a"); }
    if (number_eqv_method(cfg, a, a_zero) != a_true) { halt("expected number_eqv_method(a, a_zero) == a_true"); }
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
    if (number_eqv_method(cfg, a, b) != a_true) { halt("expected number_eqv_method(a, b) == a_true"); }
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

    quota_config_report(cfg);  // report on resource usage
}
