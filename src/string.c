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
#include "number.h"
#include "expr.h"

/*
struct cache {
    ACTOR       _act;
    int         n;  // number of entries in use
    int         m;  // number of entries allocated
    Actor       (*cmp)(Config cfg, Actor entry, Actor value);  // comparison function
    size_t      size;  // size of each managed value
    Actor *     base;  // pointer to contiguous block of Actor references
};
static char *
memcpy(void * dst, const void * src, size_t len)  // defined here to avoid pulling in <string.h>
{
    register const char * p = src;
    register char * q = dst;
    while (len-- > 0) {
        q[len] = p[len];
    }
    return (char *)src;
}
static void
beh_cache(Event e)
{
    TRACE(fprintf(stderr, "beh_cache{event=%p}\n", e));
    beh_halt(e);
}
static Actor
cache_intern(Config cfg, struct cache * cache, Actor value)
{
    register int a, b, c, d;
    int n, m;
    Actor p;
    
    n = cache->n;
    a = 0;
    b = n - 1;
    while (a <= b) {
        c = (a + b) >> 1;  // == ((a + b) / 2)
        p = cache->base[c];
        d = ((Integer)((cache->cmp)(cfg, p, value)))->i;
        if (d > 0) {
            b = c - 1;
        } else if (d < 0) {
            a = c + 1;
        } else {
            return p;  // FOUND!
        }
    }
    // NOT FOUND (a == insertion point)
    Config sponsor = (Config)a_root_config;  // the root config sponsors all cache allocation
    p = config_create(sponsor, cache->size, BEH(value));  // allocate "permanent" storage
    memcpy(p, value, cache->size);  // copy actor data
    m = cache->m;
    if (n < m) {  // space available for entry
        DEBUG(fprintf(stderr, "cache_intern: space available, n=%d, m=%d\n", n, m));
        for (b = n; a < b; --b) {
            cache->base[b] = cache->base[b - 1];
        }
        cache->base[a] = p;
        cache->n = n + 1;
    } else if (cache->base) {  // need to allocate more space
        Actor * bp = cache->base;
        m = m << 1;  // == (m * 2)
        DEBUG(fprintf(stderr, "cache_intern: expanded allocation, n=%d, m=%d\n", n, m));
        cache->m = m;
        cache->base = (Actor *)config_create(sponsor, sizeof(Actor) * m, beh_halt);
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
        cache->base = (Actor *)config_create(sponsor, sizeof(Actor) * m, beh_halt);
        cache->base[0] = p;
        cache->n = 1;
    }
    return p;
}
static struct cache string_cache = { { beh_cache }, 0, 0, string_diff_method, sizeof(STRING), NULL };
*/

#define STRING_ACTOR_DECL(beh)     { (beh), string_eqv_method }
STRING the_empty_string_actor = { STRING_ACTOR_DECL(beh_string), "", a_zero };

inline Actor
cstring_new(Config cfg, char * p)
{
    if (*p == '\0') { return a_empty_string; }
    String s = (String)config_create(cfg, sizeof(STRING), beh_string);
    s->_act.eqv = string_eqv_method;  // override eqv procedure
    s->p = p;  // must have '\0' terminator
    s->n = a_minus_one;  // unknown length
    return (Actor)s;
}

inline Actor
pstring_new(Config cfg, char * p, int n)
{
    if (n <= 0) { return a_empty_string; }
    String s = (String)config_create(cfg, sizeof(STRING), beh_string);
    s->_act.eqv = string_eqv_method;  // override eqv procedure
    s->p = p;  // may, or may not, have '\0' terminator
    s->n = integer_new(cfg, n);  // pre-defined length
    return (Actor)s;
}

inline Actor
string_length_method(Config cfg, Actor this)
{
    if (beh_string != BEH(this)) { config_fail(cfg, e_inval); }  // string required
    String s = (String)this;
    if (s->n == a_minus_one) {  // unknown length
        int n = 0;
        char *p = s->p;
        while (*p++) {
            ++n;
        }
        s->n = integer_new(cfg, n);
    }
    return s->n;
}

/*
inline Actor
string_intern_method(Config cfg, Actor this)  // return the canonical String instance with this value
{
    string_length_method(cfg, this);  // ensure that string length is cached
    return cache_intern(cfg, &string_cache, this);
}
*/

// Actor
// string_eqv_method(Config cfg, Actor this, Actor that)
// {
//     if (this == that) {
//         return a_true;
//     }
//     if (beh_string != BEH(this)) { config_fail(cfg, e_inval); }  // string required
//     String s = (String)this;
//     if (beh_string == BEH(that)) {
//         String t = (String)that;
//         if ((s->p == t->p) && (s->n == t->n)) {
//             return a_true;
//         }
//         Actor n = string_length_method(cfg, this);
//         Actor m = string_length_method(cfg, that);
//         if (actor_eqv(cfg, n, m) != a_true) {
//             return a_false;
//         }
//         int i = ((Integer)(n))->i;
//         char* p = s->p;
//         char* q = t->p;
//         while (i-- > 0) {
//             if (*p++ != *q++) {
//                 return a_false;
//             }
//         }
//         return a_true;
//     }
//     return a_false;
// }
void
string_eqv_method(Event e, Actor cust, Actor this, Actor that)
{
    if (this == that) {
        config_send(e, cust, a_true);
        return;
    }
    if (beh_string != BEH(this)) { config_fail(SPONSOR(e), e_inval); }  // string required
    String s = (String)this;
    if (beh_string == BEH(that)) {
        String t = (String)that;
        if ((s->p == t->p) && (s->n == t->n)) {
            config_send(e, cust, a_true);
            return;
        }
        Actor n = string_length_method(SPONSOR(e), this);
        Actor m = string_length_method(SPONSOR(e), that);
        Event groundout = (Event)event_new(SPONSOR(e), NOTHING, NOTHING);
        actor_eqv(groundout, NOTHING, n, m);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor tst = effect->message;        
        if (tst != a_true) {
            config_send(e, cust, a_false);
            return;
        }
        int i = ((Integer)(n))->i;
        char* p = s->p;
        char* q = t->p;
        while (i-- > 0) {
            if (*p++ != *q++) {
                config_send(e, cust, a_false);
                return;
            }
        }
        config_send(e, cust, a_true);
        return;
    }
    config_send(e, cust, a_false);
    return;
}


Actor
string_diff_method(Config cfg, Actor this, Actor that)
// (this < that) -> <0, (this == that) -> 0, (this > that) -> >0.
{
    if (beh_string != BEH(this)) { config_fail(cfg, e_inval); }  // string required
    String s = (String)this;
    Actor n = string_length_method(cfg, this);
    int i = ((Integer)(n))->i;
    if (beh_string != BEH(that)) { config_fail(cfg, e_inval); }  // string required
    String t = (String)that;
    Actor m = string_length_method(cfg, that);
    int j = ((Integer)(m))->i;
    char* p = s->p;
    char* q = t->p;
    while ((i > 0) && (j > 0)) {
        int d = (*p - *q);
        if (d != 0) {
            return integer_new(cfg, d);
        }
        ++p; ++q;
        --i; --j;
    }
    if (i > j) {
        return a_one;
    }
    if (i < j) {
        return a_minus_one;
    }
    return a_zero;
}

/**
LET string_beh(p, n) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    (#match, value, env) : [
        CASE string_match_method(SELF, value) OF
        TRUE : [ SEND env TO ok ]
        _ : [ SEND (SELF, msg) TO fail ]
        END
    ]
    (#read) : [ SEND string_pop(SELF) TO ok ]
    _ : [ SEND (SELF, msg) TO fail ]
    END
]
**/
void
beh_string(Event e)
{
    TRACE(fprintf(stderr, "beh_string{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "beh_string: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "beh_string: (#eval, _)\n"));
        config_send(e, r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "beh_string: (#match, %p, %p)\n", rm->value, rm->env));
        Event groundout = (Event)event_new(SPONSOR(e), NOTHING, NOTHING);
        string_eqv_method(groundout, NOTHING, SELF(e), rm->value);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor tst = effect->message;
        if (tst == a_true) {
            config_send(e, r->ok, rm->env);
        } else {
            TRACE(fprintf(stderr, "beh_string: MISMATCH!\n"));
            config_send(e, r->fail, (Actor)e);
        }
    } else if (val_req_read == BEH(r->req)) {  // (#read)
        String s = (String)SELF(e);
        if (s->n == a_minus_one) {  // indeterminate length
            char *p = s->p;
            Actor c = integer_new(SPONSOR(e), (int)*p++);
            TRACE(fprintf(stderr, "beh_string: (#read) -> (%d, @%p)\n", ((Integer)c)->i, p));
            config_send(e, r->ok, PR(c, cstring_new(SPONSOR(e), p)));
        } else {
            char *p = s->p;
            Actor c = integer_new(SPONSOR(e), (int)*p++);
            int n = ((Integer)(s->n))->i - 1;
            TRACE(fprintf(stderr, "beh_string: (#read) -> (%d, %d@%p)\n", ((Integer)c)->i, n, p));
            config_send(e, r->ok, PR(c, pstring_new(SPONSOR(e), p, n)));
        }
    } else {
        TRACE(fprintf(stderr, "beh_string: FAIL!\n"));
        config_send(e, r->fail, (Actor)e);
    }
}

void
test_string()
{
    Actor a, b, c;
    Integer n, m;
    String s, t;

    TRACE(fprintf(stderr, "---- test_string ----\n"));
    Config cfg = quota_config_new(a_root_config, 2000);
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    TRACE(fprintf(stderr, "a_empty_string = %p\n", a_empty_string));
    a = string_length_method(cfg, a_empty_string);
    if (a_zero != a) { halt("expected a_zero == a"); }
    Event groundout = (Event)event_new(cfg, NOTHING, NOTHING);
    actor_eqv(groundout, NOTHING, a, a_zero);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Actor tst = effect->message;    
    if (tst != a_true) { halt("expected actor_eqv(a, a_zero) == a_true"); }
    n = (Integer)a;
    if (n->i != 0) { halt("expected n->i == 0"); }
    s = (String)a_empty_string;
    n = (Integer)s->n;
    TRACE(fprintf(stderr, "s->n = %d\n", n->i));
    TRACE(fprintf(stderr, "s->p = \"%*s\"\n", n->i, s->p));
/*
*/
    char* p = "foobarfoo";
    char* q = p + 6;
    a = pstring_new(cfg, p, 3);
    s = (String)a;
    n = (Integer)s->n;
    TRACE(fprintf(stderr, "s = %d\"%.*s\" of \"%s\"\n", n->i, n->i, s->p, s->p));
    b = cstring_new(cfg, q);
    t = (String)b;
    m = (Integer)t->n;
    TRACE(fprintf(stderr, "t = %d\"%.*s\"\n", m->i, m->i, t->p));
    groundout = (Event)event_new(cfg, NOTHING, NOTHING);
    string_eqv_method(groundout, NOTHING, a, b);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    tst = effect->message;
    if (tst != a_true) { halt("expected string_eqv_method(a, b) == a_true"); }
    m = (Integer)t->n;
    TRACE(fprintf(stderr, "t = %d\"%.*s\"\n", m->i, m->i, t->p));
/*
*/
    a = cstring_new(cfg, "foo");
    b = cstring_new(cfg, "bar");
    c = string_diff_method(cfg, a, a);
    if (c != a_zero) { halt("expected \"foo\" == \"foo\""); }
    n = (Integer)string_diff_method(cfg, a, b);
    TRACE(fprintf(stderr, "(\"foo\" - \"bar\") -> %d\n", n->i));
    if (n->i <= 0) { halt("expected \"foo\" > \"bar\""); }
    m = (Integer)string_diff_method(cfg, b, a);
    TRACE(fprintf(stderr, "(\"bar\" - \"foo\") -> %d\n", m->i));
    if (m->i >= 0) { halt("expected \"bar\" < \"foo\""); }
    b = cstring_new(cfg, "foobar");
    n = (Integer)string_diff_method(cfg, a, b);
    TRACE(fprintf(stderr, "(\"foo\" - \"foobar\") -> %d\n", n->i));
    if (n->i >= 0) { halt("expected \"foo\" < \"foobar\""); }
    m = (Integer)string_diff_method(cfg, b, a);
    TRACE(fprintf(stderr, "(\"foobar\" - \"foo\") -> %d\n", m->i));
    if (m->i <= 0) { halt("expected \"foobar\" > \"foo\""); }

    quota_config_report(cfg);  // report on resource usage
}
