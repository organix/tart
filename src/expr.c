/*

expr.c -- Tiny Actor Run-Time

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

#include "expr.h"

void
val_request(Event e)
{
    TRACE(fprintf(stderr, "val_request{event=%p}\n", e));
    Request r = (Request)SELF(e);
    TRACE(fprintf(stderr, "val_request: ok=%p, fail=%p, req=%p\n", r->ok, r->fail, r->req));
    expr_value(e);
}
static Actor
request_new(Actor ok, Actor fail, Actor req)
{
    Request r = NEW(REQUEST);
    BEH(r) = val_request;
    r->ok = ok;
    r->fail = fail;
    r->req = req;
    return (Actor)r;
}

void
val_req_eval(Event e)
{
    TRACE(fprintf(stderr, "val_req_eval{event=%p}\n", e));
    ReqEval req = (ReqEval)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_eval: env=%p\n", req->env));
    expr_value(e);
}
Actor
req_eval_new(Actor ok, Actor fail, Actor env)
{
    ReqEval req = NEW(REQ_EVAL);
    BEH(req) = val_req_eval;
    req->env = env;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_match(Event e)
{
    TRACE(fprintf(stderr, "val_req_match{event=%p}\n", e));
    ReqMatch req = (ReqMatch)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_match: value=%p, env=%p\n", req->value, req->env));
    expr_value(e);
}
Actor
req_match_new(Actor ok, Actor fail, Actor value, Actor env)
{
    ReqMatch req = NEW(REQ_MATCH);
    BEH(req) = val_req_match;
    req->value = value;
    req->env = env;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_bind(Event e)
{
    TRACE(fprintf(stderr, "val_req_bind{event=%p}\n", e));
    ReqBind req = (ReqBind)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_bind: key=%p, value=%p\n", req->key, req->value));
    expr_value(e);
}
Actor
req_bind_new(Actor ok, Actor fail, Actor key, Actor value)
{
    ReqBind req = NEW(REQ_BIND);
    BEH(req) = val_req_bind;
    req->key = key;
    req->value = value;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_lookup(Event e)
{
    TRACE(fprintf(stderr, "val_req_lookup{event=%p}\n", e));
    ReqLookup req = (ReqLookup)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_lookup: key=%p\n", req->key));
    expr_value(e);
}
Actor
req_lookup_new(Actor ok, Actor fail, Actor key)
{
    ReqLookup req = NEW(REQ_LOOKUP);
    BEH(req) = val_req_lookup;
    req->key = key;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_combine(Event e)
{
    TRACE(fprintf(stderr, "val_req_combine{event=%p}\n", e));
    ReqCombine req = (ReqCombine)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_combine: opnd=%p, env=%p\n", req->opnd, req->env));
    expr_value(e);
}
Actor
req_combine_new(Actor ok, Actor fail, Actor opnd, Actor env)
{
    ReqCombine req = NEW(REQ_COMBINE);
    BEH(req) = val_req_combine;
    req->opnd = opnd;
    req->env = env;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_read(Event e)
{
    TRACE(fprintf(stderr, "val_req_read{event=%p}\n", e));
//    ReqRead req = (ReqRead)REQ(SELF(e));
    expr_value(e);
}
Actor
req_read_new(Actor ok, Actor fail)
{
    ReqRead req = NEW(REQ_READ);
    BEH(req) = val_req_read;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_write(Event e)
{
    TRACE(fprintf(stderr, "val_req_write{event=%p}\n", e));
    ReqWrite req = (ReqWrite)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_write: value=%p\n", req->value));
    expr_value(e);
}
Actor
req_write_new(Actor ok, Actor fail, Actor value)
{
    ReqWrite req = NEW(REQ_WRITE);
    BEH(req) = val_req_write;
    req->value = value;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_call(Event e)
{
    TRACE(fprintf(stderr, "val_req_call{event=%p}\n", e));
    ReqCall req = (ReqCall)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_call: selector=%p, parameter=%p\n", req->selector, req->parameter));
    expr_value(e);
}
Actor
req_call_new(Actor ok, Actor fail, Actor selector, Actor parameter)
{
    ReqCall req = NEW(REQ_CALL);
    BEH(req) = val_req_call;
    req->selector = selector;
    req->parameter = parameter;
    return request_new(ok, fail, (Actor)req);
}

/**
LET value_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    (#match, $SELF, env) : [ SEND env TO ok ]
    _ : [ SEND (SELF, msg) TO fail ]
    END
]
**/
void
expr_value(Event e)
{
    TRACE(fprintf(stderr, "expr_value{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("expr_value: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_value: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "expr_value: (#eval, _)\n"));
        config_send(SPONSOR(e), r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "expr_value: (#match, %p, %p)\n", rm->value, rm->env));
        if (SELF(e) == rm->value) {
            config_send(SPONSOR(e), r->ok, rm->env);
        } else {
            TRACE(fprintf(stderr, "expr_value: MISMATCH!\n"));
            config_send(SPONSOR(e), r->fail, (Actor)e);
        }
    } else {
        TRACE(fprintf(stderr, "expr_value: FAIL!\n"));
        config_send(SPONSOR(e), r->fail, (Actor)e);
    }
}

/**
LET empty_env_beh((key, value), next) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#bind, key', value') : [
        CREATE dict' WITH env_beh((key', value'), SELF)
        SEND dict' TO ok
    ]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
expr_env_empty(Event e)
{
    TRACE(fprintf(stderr, "expr_env_empty{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("expr_env_empty: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_env_empty: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_bind == BEH(r->req)) {  // (#bind, key, value)
        ReqBind rb = (ReqBind)r->req;
        TRACE(fprintf(stderr, "expr_env_empty: (#bind, %p -> %p)\n", rb->key, rb->value));
        Actor dict = dict_bind(SELF(e), rb->key, rb->value);
        config_send(SPONSOR(e), r->ok, dict);
    } else {
        expr_value(e);  // delegate
    }
}

/**
LET env_beh((key, value), next) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#lookup, $key) : [ SEND value TO ok ]
    (#lookup, _) : [ SEND msg TO parent ]
    (#bind, key', value') : [
        CREATE dict' WITH env_beh((key', value'), SELF)
        SEND dict' TO ok
    ]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
expr_env(Event e)
{
    TRACE(fprintf(stderr, "expr_env{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("expr_env: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_env: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_lookup == BEH(r->req)) {  // (#lookup, _)
        ReqLookup rl = (ReqLookup)r->req;
        TRACE(fprintf(stderr, "expr_env: (#lookup, _)\n"));
        Actor value = dict_lookup(SELF(e), rl->key);
        TRACE(fprintf(stderr, "expr_env: (#lookup, %p) -> %p\n", rl->key, value));
        if (value != NULL) {
            config_send(SPONSOR(e), r->ok, value);
        } else {
            TRACE(fprintf(stderr, "expr_env: FAIL!\n"));
            config_send(SPONSOR(e), r->fail, (Actor)e);
        }
    } else if (val_req_bind == BEH(r->req)) {  // (#bind, key, value)
        ReqBind rb = (ReqBind)r->req;
        TRACE(fprintf(stderr, "expr_env: (#bind, %p -> %p)\n", rb->key, rb->value));
        Actor dict = dict_bind(SELF(e), rb->key, rb->value);
        config_send(SPONSOR(e), r->ok, dict);
    } else {
        expr_value(e);  // delegate
    }
}

/**
LET skip_ptrn_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#match, _, env) : [ SEND env TO ok ]
    _ : value_beh(msg)  # delegate
    END
]
**/
static void
ptrn_skip(Event e)
{
    TRACE(fprintf(stderr, "ptrn_skip{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("ptrn_skip: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "ptrn_skip: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_match == BEH(r->req)) {  // (#match, _, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "ptrn_skip: (#match, %p, %p)\n", rm->value, rm->env));
        config_send(SPONSOR(e), r->ok, rm->env);
    } else {
        expr_value(e);  // delegate
    }
}
/**
CREATE skip_ptrn WITH skip_ptrn_beh
**/
ACTOR the_ptrn_skip_actor = { ptrn_skip };

/**
LET bind_ptrn_beh(name) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#match, value, env) : [ SEND ((ok, fail), #bind, name, value) TO env ]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
ptrn_bind(Event e)
{
    TRACE(fprintf(stderr, "ptrn_bind{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor name = DATA(SELF(e));  // (name)
    if (val_request != BEH(MSG(e))) { halt("ptrn_bind: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "ptrn_bind: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "ptrn_bind: (#match, %p, %p)\n", rm->value, rm->env));
        config_send(SPONSOR(e), rm->env, req_bind_new(r->ok, r->fail, name, rm->value));
    } else {
        expr_value(e);  // delegate
    }
}

/**
LET name_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [ SEND ((ok, fail), #lookup, SELF) TO env ]
    _ : [ SEND (SELF, msg) TO fail ]
    END
]
**/
void
expr_name(Event e)
{
    TRACE(fprintf(stderr, "expr_name{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("expr_name: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_name: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, env)
        ReqEval re = (ReqEval)r->req;
        TRACE(fprintf(stderr, "expr_name: (#eval, %p)\n", re->env));
        config_send(SPONSOR(e), re->env, req_lookup_new(r->ok, r->fail, SELF(e)));
    } else {
        TRACE(fprintf(stderr, "expr_name: FAIL!\n"));
        config_send(SPONSOR(e), r->fail, (Actor)e);
    }
}

/**
LET eval_body_beh((ok, fail), body) = \env.[
    SEND ((ok, fail), #eval, env) TO body
]
**/
static void
beh_eval_body(Event e)
{
    TRACE(fprintf(stderr, "beh_eval_body{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor a = DATA(SELF(e));  // ((ok, fail), body)
    if (beh_pair != BEH(a)) { halt("beh_eval_body: (cust, body) required"); }
    Pair p = (Pair)a;
    Actor body = p->t;
    a = p->h;
    if (beh_pair != BEH(a)) { halt("beh_eval_body: (ok, fail) required"); }
    Pair cust = (Pair)a;
    Actor ok = cust->h;
    Actor fail = cust->t;
    Actor env = MSG(e);  // (env)
    TRACE(fprintf(stderr, "beh_eval_body: ok=%p, fail=%p, body=%p, env=%p\n", ok, fail, body, env));
    config_send(SPONSOR(e), body, req_eval_new(ok, fail, env));
}
static inline void
val_expect(Event e)
{
    TRACE(fprintf(stderr, "val_expect{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Any expect = DATA(SELF(e));
    Any actual = MSG(e);
    if (expect != actual) {
        TRACE(fprintf(stderr, "val_expect: %p != %p\n", expect, actual));
        halt("unexpected");
    }
}
#define a_empty_env (a_empty_dict)
void
test_expr()
{
    Actor expr;
    Actor cust;

    TRACE(fprintf(stderr, "---- test_expr ----\n"));
    Config cfg = config_new();
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    TRACE(fprintf(stderr, "a_empty_env = %p\n", a_empty_env));
    /* empty environment evaluates to itself */
    expr = a_empty_env;
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    cust = value_new(val_expect, a_empty_env);
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    config_send(cfg, expr, req_eval_new(cust, a_halt, a_empty_env));
    /* the configuration evaluates to itself */
    expr = (Actor)cfg;
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    cust = value_new(val_expect, cfg);
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    config_send(cfg, expr, req_eval_new(cust, a_halt, a_empty_env));
    /* dispatch until empty */
    while (config_dispatch(cfg) == a_true)
        ;

    /* name binding can be resolved */
    cust = value_new(val_expect, cfg);
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    Actor s_x = actor_new(expr_name);
    TRACE(fprintf(stderr, "s_x = %p\n", s_x));
    expr = value_new(beh_eval_body, PR(PR(cust, a_halt), s_x));
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    config_send(cfg, a_empty_dict, req_bind_new(expr, a_halt, s_x, (Actor)cfg));
    /* dispatch until empty */
    while (config_dispatch(cfg) == a_true)
        ;

    /* pair values can be matched */
    cust = value_new(val_expect, a_empty_env);
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    Actor p = PR(a_true, a_false);
    TRACE(fprintf(stderr, "p = %p\n", p));
    Actor q = PR(a_true, a_false);
    TRACE(fprintf(stderr, "q = %p\n", q));
    if (p == q) { halt("expected p != q"); }
    config_send(cfg, p, req_match_new(cust, a_halt, p, a_empty_env));  // match p to itself
    config_send(cfg, q, req_match_new(cust, a_halt, p, a_empty_env));  // match p to q
    /* dispatch until empty */
    while (config_dispatch(cfg) == a_true)
        ;
}
