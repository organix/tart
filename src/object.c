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
#include "pair.h"  // Scope is implemented with dictionary

struct cache {
    int         n;  // number of entries in use
    int         m;  // number of entries allocated
    int         (*cmp)(Object entry, Any value);  // comparison function
    Object      (*new)(Any value);  // constructor function
    Object *    base;  // pointer to continguous block of Object memory
};
static Object
cache_intern(struct cache * cache, Any value)
{
    register int a, b, c, d;
    Object p;
    
    int n = cache->n;
    a = 0;
    b = n - 1;
    while (a <= b) {
        c = (a + b) >> 1;  // == ((a + b) / 2)
        p = cache->base[c];
        d = (cache->cmp)(p, value);
        if (d > 0) {
            b = c - 1;
        } else if (d < 0) {
            a = c + 1;
        } else {
            return p;  // FOUND!
        }
    }
    // NOT FOUND (a == insertion point)
    p = (cache->new)(value);
    int m = cache->m;
    if (n < m) {  // space available for entry
        DEBUG(fprintf(stderr, "cache_intern: space available, n=%d, m=%d\n", n, m));
        for (b = n; a < b; --b) {
            cache->base[b] = cache->base[b - 1];
        }
        cache->base[a] = p;
        cache->n = n + 1;
    } else if (cache->base) {  // need to allocate more space
        Object * bp = cache->base;
        m = m << 1;  // == (m * 2)
        DEBUG(fprintf(stderr, "cache_intern: expanded allocation, n=%d, m=%d\n", n, m));
        cache->m = m;
        cache->base = NEWxN(Object, m);
        for (b = n; a < b; --b) {
            cache->base[b] = bp[b - 1];
        }
        cache->base[a] = p;
        for (b = 0; b < a; ++b) {
            cache->base[b] = bp[b];
        }
        cache->n = n + 1;
    } else {  // initial space allocation
        m = 21;
        DEBUG(fprintf(stderr, "cache_intern: initial allocation, m=%d\n", m));
        cache->m = m;
        cache->base = NEWxN(Object, m);
        cache->base[0] = p;
        cache->n = 1;
    }
    return p;
}

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
    halt("boolean_not_method called");
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
    switch (i) {  // check for cached objects
        case -1 : return o_minus_one;
        case 0 : return o_zero;
        case 1 : return o_one;
        case 2 : return o_two;
    }
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
number_diff_method(Object this, Object that)
{
    if (k_number != that->kind) {
        return this;
    }
    return number_new(((Number)this)->i - ((Number)that)->i);
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
call_diff(Object this, Object that)
{
    return (((NumberKind)this->kind)->diff)(this, that);
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
    number_diff_method,
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

static Object
func_kind_of_method(Object this, Kind kind)
{
    return ((this->kind == kind) || (k_func == kind)) ? o_true : o_false;
}
static Object
func_lookup_method(Object this, Object input)
{
    return NULL;  // undefined
}
inline Object
call_lookup(Object this, Object input)
{
    return (((FuncKind)this->kind)->lookup)(this, input);
}
static FUNC_KIND the_func_kind = {
    { func_kind_of_method,
      base_equal_to_method },
    func_lookup_method
};
Kind k_func = (Kind)&the_func_kind;
static OBJECT the_undef_func_object = {
    (Kind)&the_func_kind
};
Object o_undef_func = &the_undef_func_object;

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
string_new_value(Any value)
{
    return string_new((char *)value);
}
static int
string_compare_value(Object this, Any value)  // (this < that) -> <0, (this == that) -> 0, (this > that) -> >0.
{
    char * s = ((String)this)->s;
    char * t = (char *)value;
    while (*s == *t) {
        if (*s == '\0') {
            return 0;  // matched string terminator
        }
        ++s;
        ++t;
    }
    return (*s - *t);
}
static struct cache string_cache = { 0, 0, string_compare_value, string_new_value, NULL };
inline Object
string_intern(char * s)  // return the canonical String instance with this value
{
    cache_intern(&string_cache, s);
}
static Object
string_equal_to_method(Object this, Object that)
{
    if (this == that) {
        return o_true;
    }
    if (k_string != that->kind) {
        return o_false;
    }
    Any value = ((String)that)->s;
    return (string_compare_value(this, value) == 0) ? o_true : o_false;
}
static Object
string_lookup_method(Object this, Object offset)
{
    if (k_number != offset->kind) {
        return NULL;  // invalid offset
    }
    int n = ((String)this)->n;
    int i = ((Number)offset)->i;
    if ((i < 0) || (i >= n)) {
        return NULL;  // offset out of bounds
    }
    return number_new(((String)this)->s[i]);
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
static STRING_KIND the_string_kind = {
    { { func_kind_of_method,
        string_equal_to_method },
      string_lookup_method },
    string_length_method,
    string_concat_method
};
Kind k_string = (Kind)&the_string_kind;
static STRING the_empty_string_object = {
    { (Kind)&the_string_kind },
    0,
    ""
};
Object o_empty_string = (Object)&the_empty_string_object;

typedef struct scope SCOPE, *Scope;
struct scope {
    OBJECT      o;
    Pair        dict;  // dictionary of local bindings
    Object      parent;  // enclosing scope (read-only)
};
inline Object
scope_new(Object parent)
{
    Scope p = NEW(SCOPE);
    p->o.kind = k_scope;
    p->dict = dict_new();
    p->parent = parent;
    return (Object)p;
}
static Object
scope_lookup_method(Object this, Object key)
{
    while (this != o_empty_scope) {
        Pair dict = ((Scope)this)->dict;
        Any value = dict_lookup(dict, key);
        if (value != NULL) {  // found in local scope
            return (Object)value;
        }
        this = ((Scope)this)->parent;  // simulate tail-recursion
    }
    return NULL;
}
static Object
scope_bind_method(Object this, Object key, Object value)
{
    Pair dict = ((Scope)this)->dict;
    ((Scope)this)->dict = dict_bind(dict, key, value);  // bind in local scope
    return this;
}
inline Object
call_bind(Object this, Object key, Object value)
{
    return (((ScopeKind)this->kind)->bind)(this, key, value);
}
static SCOPE_KIND the_scope_kind = {
    { { func_kind_of_method,
        base_equal_to_method },
      scope_lookup_method },
    scope_bind_method
};
Kind k_scope = (Kind)&the_scope_kind;
static SCOPE the_empty_scope_object = {
    { (Kind)&the_scope_kind },
    NIL,
    (Object)&the_empty_scope_object
};
Object o_empty_scope = (Object)&the_empty_scope_object;

typedef struct array ARRAY, *Array;
struct array {
    OBJECT      o;
    Pair        q;  // deque of array items
    int         n;  // number of items
};
inline Object
array_new()
{
    Array p = NEW(ARRAY);
    p->o.kind = k_array;
    p->q = queue_new();
    p->n = 0;
    return (Object)p;
}
static Object
array_kind_of_method(Object this, Kind kind)
{
    return ((this->kind == kind) || (k_func == kind) || (k_scope == kind)) ? o_true : o_false;
}
static Object
array_length_method(Object this)
{
	return number_new(((Array)this)->n);
}
static Object s_length;  // cached symbol
static Object
array_lookup_method(Object this, Object key)
{
    if (!s_length) { s_length = string_intern("length"); }  // lazy-init symbol
    if (s_length == key) {
        return array_length_method(this);
    } else if (k_number != key->kind) {
        return NULL;  // invalid offset
    }
    int n = ((Array)this)->n;
    int i = ((Number)key)->i;
    Pair q = ((Array)this)->q;
    return queue_lookup(q, i);
}
static Object
array_bind_method(Object this, Object key, Object value)
{
    if (k_number != key->kind) {
        return NULL;  // invalid offset
    }
    int n = ((Array)this)->n;
    int i = ((Number)key)->i;
    Pair q = ((Array)this)->q;
    if ((i < 0) || (i > n)) {
        return NULL;  // offset out of bounds
    }
    if (i == n) {
        queue_give(q, value);
        ((Array)this)->n = (n + 1);
    } else {
        queue_bind(q, i, value);
    }
    return this;
}
static SCOPE_KIND the_array_kind = {
    { { array_kind_of_method,
        base_equal_to_method },
      array_lookup_method },
    array_bind_method
};
Kind k_array = (Kind)&the_array_kind;

static void
trace_string_cache()
{
    int i;
    char c = '[';

    TRACE(fprintf(stderr, "string_cache = "));
    for (i = 0; i < string_cache.n; ++i) {
        String s = (String)string_cache.base[i];
        TRACE(fprintf(stderr, "%c \"%s\"", c, s->s));
        c = ',';
    }
    TRACE(fprintf(stderr, " ]\n"));
}
void
test_object()
{
    Object x, y;

    TRACE(fprintf(stderr, "---- test_object ----\n"));
/*
    static OBJECT the_bool_object = {
        (Kind)&the_boolean_kind
    };
    Object o_bool = &the_bool_object;
    x = call_not(o_bool);
    assert(x == o_true);
    assert(x == o_false);
*/
/*
    string_intern("0");
    trace_string_cache();
    string_intern("1");
    trace_string_cache();
    string_intern("2");
    trace_string_cache();
    string_intern("3");
    trace_string_cache();
    string_intern("4");
    trace_string_cache();
*/
/*
    string_intern("4");
    trace_string_cache();
    string_intern("3");
    trace_string_cache();
    string_intern("2");
    trace_string_cache();
    string_intern("1");
    trace_string_cache();
    string_intern("0");
    trace_string_cache();
*/
    x = string_intern("foo");
    trace_string_cache();
    string_intern("bar");
    trace_string_cache();
    string_intern("baz");
    trace_string_cache();
    string_intern("quux");
    trace_string_cache();
    string_intern("-");
    trace_string_cache();
    y = string_intern("foo");
    trace_string_cache();
    assert(x == y);
    x = string_new("foo");
    assert(x != y);
/*
*/
}
