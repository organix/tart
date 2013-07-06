/*

object.h -- Tiny Actor Run-Time

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
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "tart.h"

typedef struct kind KIND, *Kind;
typedef struct object OBJECT, *Object;
struct object {
    Kind        kind;
};
struct kind {
    Object      (*kind_of)(Object this, Kind kind);
    Object      (*equal_to)(Object this, Object that);
};
extern Object   call_kind_of(Object this, Kind kind);
extern Object   call_equal_to(Object this, Object that);

typedef struct null_kind NULL_KIND, *NullKind;
struct null_kind {
    KIND        k;
};
extern Kind k_null;
extern Object o_null;

typedef struct boolean_kind BOOLEAN_KIND, *BooleanKind;
struct boolean_kind {
    KIND        k;
    Object      (*not)(Object this);
};
extern Object   call_not(Object this);
extern Kind k_boolean;
extern Object o_true;
extern Object o_false;

typedef struct number_kind NUMBER_KIND, *NumberKind;
extern Object   number_new(int i);
struct number_kind {
    KIND        k;
    Object      (*diff)(Object this, Object that);
    Object      (*plus)(Object this, Object that);
    Object      (*times)(Object this, Object that);
    int         (*as_int)(Object this);
};
extern Object   call_diff(Object this, Object that);
extern Object   call_plus(Object this, Object that);
extern Object   call_times(Object this, Object that);
extern int      call_as_int(Object this);
extern Kind k_number;
extern Object o_minus_one;
extern Object o_zero;
extern Object o_one;
extern Object o_two;

typedef struct func_kind FUNC_KIND, *FuncKind;  // extensionally-defined function
struct func_kind {
    KIND        k;
    Object      (*lookup)(Object this, Object input);
};
extern Object   call_lookup(Object this, Object input);
extern Kind k_func;
extern Object o_undef_func;

typedef struct string_kind STRING_KIND, *StringKind;
extern Object   string_new(char * s);
extern Object   string_intern(char * s);
struct string_kind {
    FUNC_KIND   f;
    Object      (*length)(Object this);
    Object      (*concat)(Object this, Object that);
};
extern Object   call_length(Object this);
extern Object   call_concat(Object this, Object that);
extern Kind k_string;
extern Object o_empty_string;

typedef struct scope_kind SCOPE_KIND, *ScopeKind;
extern Object   scope_new(Object parent);
struct scope_kind {
    FUNC_KIND   f;
    Object      (*bind)(Object this, Object key, Object value);
};
extern Object   call_bind(Object this, Object key, Object value);
extern Kind k_scope;
extern Object o_empty_scope;

extern Object   array_new();  // a specialized kind of Scope
extern Kind k_array;

extern void test_object();  // unit-test method

#endif /* _OBJECT_H_ */
