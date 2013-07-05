/*

object.c -- Tiny Actor Run-Time

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

#include "object.h"

static Object
base_kind_of_method(Object this, Kind kind)
{
    return (this->kind == kind) ? o_true : o_false;
}
static Object
base_equal_to_method(Object this, Object that)
{
    return (this == that) ? o_true : o_false;
}
inline Object
call_kind_of(Object this, Kind kind)
{
    return (((Kind)this->kind)->kind_of)(this, kind);
}
inline Object
call_equal_to(Object this, Object that)
{
    return (((Kind)this->kind)->equal_to)(this, that);
}

static NULL_KIND the_null_kind = {
    { base_kind_of_method,
      base_equal_to_method }
};
Kind k_null = (Kind)&the_null_kind;

static OBJECT the_null_object = {
    (Kind)&the_null_kind
};
Object o_null = &the_null_object;

static Object
boolean_kind_of_method(Object this, Kind kind)
{
    return ((this->kind == kind) || (k_boolean == kind)) ? o_true : o_false;
}
static Object
boolean_not_method(Object this)
{
    return (this == o_true) ? o_false : o_true;  // NOTE: this should never be called
}
inline Object
call_not(Object this)
{
    return (((BooleanKind)this->kind)->not)(this);
}
static BOOLEAN_KIND the_boolean_kind = {
    { boolean_kind_of_method,
      base_equal_to_method },
    boolean_not_method
};
Kind k_boolean = (Kind)&the_boolean_kind;

static Object
true_not_method(Object this)
{
    return o_false;
}
static BOOLEAN_KIND the_true_kind = {
    { boolean_kind_of_method,
      base_equal_to_method },
    true_not_method
};
static OBJECT the_true_object = {
    (Kind)&the_true_kind
};
Object o_true = &the_true_object;

static Object
false_not_method(Object this)
{
    return o_true;
}
static BOOLEAN_KIND the_false_kind = {
    { boolean_kind_of_method,
      base_equal_to_method },
    false_not_method
};
static OBJECT the_false_object = {
    (Kind)&the_false_kind
};
Object o_false = &the_false_object;

typedef struct number NUMBER, *Number;
struct number {
    OBJECT      o;
    int         i;  // native representation
};
inline Object
number_new(int i)
{
    Number p = NEW(NUMBER);
    p->o.kind = k_number;
    p->i = i;
    return (Object)p;
}
static Object
number_equal_to_method(Object this, Object that)
{
    if (this == that) {
        return o_true;
    }
    if (k_number == that->kind) {
        if (((Number)this)->i == ((Number)that)->i) {
            return o_true;
        }
    }
    return o_false;
}
static Object
number_plus_method(Object this, Object that)
{
    if (k_number != that->kind) {
        return this;
    }
    return number_new(((Number)this)->i + ((Number)that)->i);
}
static Object
number_times_method(Object this, Object that)
{
    if (k_number != that->kind) {
        return this;
    }
    return number_new(((Number)this)->i * ((Number)that)->i);
}
static int
number_as_int_method(Object this)
{
    return ((Number)this)->i;
}
inline Object
call_plus(Object this, Object that)
{
    return (((NumberKind)this->kind)->plus)(this, that);
}
inline Object
call_times(Object this, Object that)
{
    return (((NumberKind)this->kind)->times)(this, that);
}
inline int
call_as_int(Object this)
{
    return (((NumberKind)this->kind)->as_int)(this);
}
static NUMBER_KIND the_number_kind = {
    { base_kind_of_method,
      number_equal_to_method },
    number_plus_method,
    number_times_method,
    number_as_int_method
};
Kind k_number = (Kind)&the_number_kind;
static NUMBER the_minus_one_object = {
    { (Kind)&the_number_kind },
    -1
};
Object o_minus_one = (Object)&the_minus_one_object;
static NUMBER the_zero_object = {
    { (Kind)&the_number_kind },
    0
};
Object o_zero = (Object)&the_zero_object;
static NUMBER the_one_object = {
    { (Kind)&the_number_kind },
    1
};
Object o_one = (Object)&the_one_object;
static NUMBER the_two_object = {
    { (Kind)&the_number_kind },
    2
};
Object o_two = (Object)&the_two_object;

/*
typedef struct string_kind STRING_KIND, *StringKind;
extern Object   string_new(char * s);
struct string_kind {
    KIND        k;
    Object      (*length)(Object this);
    Object      (*concat)(Object this, Object that);
    Object      (*lookup)(Object this, Object offset);
};
extern Object   call_length(Object this);
extern Object   call_concat(Object this, Object that);
extern Object   call_lookup(Object this, Object offset);
extern Kind k_string;
extern Object o_empty_string;
*/
typedef struct string STRING, *String;
struct string {
    OBJECT      o;
    int         n;  // length in characters
    char *      s;  // native representation
};
inline Object
string_new(char * s)
{
    String p = NEW(STRING);
    p->o.kind = k_string;
    p->s = s;
    int n = 0;
    while (*s++) {  // count characters in string
        ++n;
    }
    p->n = n;
    return (Object)p;
}
static Object
string_equal_to_method(Object this, Object that)
{
    if (this == that) {
        return o_true;
    }
    if (k_string == that->kind) {
        char * s = ((String)this)->s;
        char * t = ((String)that)->s;
        while (*s == *t) {
            if (*s == '\0') {
                return o_true;  // matched string terminator
            }
            ++s;
            ++t;
        }
    }
    return o_false;
}
static Object
string_length_method(Object this)
{
	return number_new(((String)this)->n);
}
static Object
string_concat_method(Object this, Object that)
{
    if (k_string != that->kind) {
        return this;
    }
    String p = NEW(STRING);
    p->o.kind = k_string;
    int n = ((String)this)->n + ((String)that)->n;  // calculate combined length
    p->n = n;
    char * s = ALLOC(n + 1);  // allocate space for new string (plus terminator)
    p->s = s;
    char * t = ((String)this)->s;
    while (*s++ = *t++)  // copy first string (and terminator)
        ;
    --s;  // over-write first terminator
    t = ((String)that)->s;
    while (*s++ = *t++)  // copy second string (and terminator)
        ;
    return (Object)p;
}
static Object
string_lookup_method(Object this, Object offset)
{
    if (k_number != offset->kind) {
        return NULL;  // invalid offset
    }
    int n = ((String)this)->n;
    int i = ((Number)offset)->i;
    if ((i < 0) || (i > n)) {  // allow lookup of terminator at offset 'n'
        return NULL;  // offset out of bounds
    }
    return number_new(((String)this)->s[i]);
}
inline Object
call_length(Object this)
{
    return (((StringKind)this->kind)->length)(this);
}
inline Object
call_concat(Object this, Object that)
{
    return (((StringKind)this->kind)->concat)(this, that);
}
inline Object
call_lookup(Object this, Object offset)
{
    return (((StringKind)this->kind)->lookup)(this, offset);
}
static STRING_KIND the_string_kind = {
    { base_kind_of_method,
      string_equal_to_method },
    string_length_method,
    string_concat_method,
    string_lookup_method
};
Kind k_string = (Kind)&the_string_kind;
static STRING the_empty_string_object = {
    { (Kind)&the_string_kind },
    0,
    ""
};
Object o_empty_string = (Object)&the_empty_string_object;
