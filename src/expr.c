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
static void
request_new(Event e, Actor cust, Actor ok, Actor fail, Actor req)
{
    // Request r = (Request)config_create(cfg, sizeof(REQUEST), val_request);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQUEST), val_request);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    r->ok = ok;
    r->fail = fail;
    r->req = req;
    // return (Actor)r;
    config_send(e, cust, (Actor)r);
}

void
val_req_eval(Event e)
{
    TRACE(fprintf(stderr, "val_req_eval{event=%p}\n", e));
    ReqEval req = (ReqEval)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_eval: env=%p\n", req->env));
    expr_value(e);
}
void
req_eval_new(Event e, Actor cust, Actor ok, Actor fail, Actor env)
{
    // ReqEval req = (ReqEval)config_create(cfg, sizeof(REQ_EVAL), val_req_eval);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_EVAL), val_req_eval);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqEval req = (ReqEval)effect->message;
    req->env = env;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);
}

void
val_req_match(Event e)
{
    TRACE(fprintf(stderr, "val_req_match{event=%p}\n", e));
    ReqMatch req = (ReqMatch)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_match: value=%p, env=%p\n", req->value, req->env));
    expr_value(e);
}
void
req_match_new(Event e, Actor cust, Actor ok, Actor fail, Actor value, Actor env)
{
    // ReqMatch req = (ReqMatch)config_create(cfg, sizeof(REQ_MATCH), val_req_match);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_MATCH), val_req_match);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqMatch req = (ReqMatch)effect->message;
    req->value = value;
    req->env = env;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);
}

void
val_req_bind(Event e)
{
    TRACE(fprintf(stderr, "val_req_bind{event=%p}\n", e));
    ReqBind req = (ReqBind)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_bind: key=%p, value=%p\n", req->key, req->value));
    expr_value(e);
}
void
req_bind_new(Event e, Actor cust, Actor ok, Actor fail, Actor key, Actor value)
{
    // ReqBind req = (ReqBind)config_create(cfg, sizeof(REQ_BIND), val_req_bind);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_BIND), val_req_bind);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqBind req = (ReqBind)effect->message;    
    req->key = key;
    req->value = value;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);
}

void
val_req_lookup(Event e)
{
    TRACE(fprintf(stderr, "val_req_lookup{event=%p}\n", e));
    ReqLookup req = (ReqLookup)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_lookup: key=%p\n", req->key));
    expr_value(e);
}
void
req_lookup_new(Event e, Actor cust, Actor ok, Actor fail, Actor key)
{
    // ReqLookup req = (ReqLookup)config_create(cfg, sizeof(REQ_LOOKUP), val_req_lookup);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_LOOKUP), val_req_lookup);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqLookup req = (ReqLookup)effect->message;    
    req->key = key;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);
}

void
val_req_combine(Event e)
{
    TRACE(fprintf(stderr, "val_req_combine{event=%p}\n", e));
    ReqCombine req = (ReqCombine)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_combine: opnd=%p, env=%p\n", req->opnd, req->env));
    expr_value(e);
}
void
req_combine_new(Event e, Actor cust, Actor ok, Actor fail, Actor opnd, Actor env)
{
    // ReqCombine req = (ReqCombine)config_create(cfg, sizeof(REQ_COMBINE), val_req_combine);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_COMBINE), val_req_combine);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqCombine req = (ReqCombine)effect->message;        
    req->opnd = opnd;
    req->env = env;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);
}

void
val_req_read(Event e)
{
    TRACE(fprintf(stderr, "val_req_read{event=%p}\n", e));
//    ReqRead req = (ReqRead)REQ(SELF(e));
    expr_value(e);
}
void
req_read_new(Event e, Actor cust, Actor ok, Actor fail)
{
    // ReqRead req = (ReqRead)config_create(cfg, sizeof(REQ_READ), val_req_read);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_READ), val_req_read);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqRead req = (ReqRead)effect->message;       
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);
}

void
val_req_write(Event e)
{
    TRACE(fprintf(stderr, "val_req_write{event=%p}\n", e));
    ReqWrite req = (ReqWrite)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_write: value=%p\n", req->value));
    expr_value(e);
}
void
req_write_new(Event e, Actor cust, Actor ok, Actor fail, Actor value)
{
    // ReqWrite req = (ReqWrite)config_create(cfg, sizeof(REQ_WRITE), val_req_write);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_WRITE), val_req_write);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqWrite req = (ReqWrite)effect->message;       
    req->value = value;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);    
}

void
val_req_call(Event e)
{
    TRACE(fprintf(stderr, "val_req_call{event=%p}\n", e));
    ReqCall req = (ReqCall)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_call: selector=%p, parameter=%p\n", req->selector, req->parameter));
    expr_value(e);
}
void
req_call_new(Event e, Actor cust, Actor ok, Actor fail, Actor selector, Actor parameter)
{
    // ReqCall req = (ReqCall)config_create(cfg, sizeof(REQ_CALL), val_req_call);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(REQ_CALL), val_req_call);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    ReqCall req = (ReqCall)effect->message;     
    req->selector = selector;
    req->parameter = parameter;
    // return request_new(cfg, ok, fail, (Actor)req);
    groundout = config_event_new(e, NOTHING, NOTHING);
    request_new(groundout, NOTHING, ok, fail, (Actor)req);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Request r = (Request)effect->message;
    config_send(e, cust, (Actor)r);       
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_value: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "expr_value: (#eval, _)\n"));
        config_send(e, r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "expr_value: (#match, %p, %p)\n", rm->value, rm->env));
        if (SELF(e) == rm->value) {
            config_send(e, r->ok, rm->env);
        } else {
            TRACE(fprintf(stderr, "expr_value: MISMATCH!\n"));
            config_send(e, r->fail, (Actor)e);
        }
    } else {
        TRACE(fprintf(stderr, "expr_value: FAIL!\n"));
        config_send(e, r->fail, (Actor)e);
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_env_empty: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_bind == BEH(r->req)) {  // (#bind, key, value)
        ReqBind rb = (ReqBind)r->req;
        TRACE(fprintf(stderr, "expr_env_empty: (#bind, %p -> %p)\n", rb->key, rb->value));
        // Actor dict = dict_bind(SPONSOR(e), SELF(e), rb->key, rb->value);
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        dict_bind(groundout, NOTHING, SELF(e), rb->key, rb->value);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor dict = effect->message;
        config_send(e, r->ok, dict);
    } else {
        expr_value(e);  // delegate
    }
}

/**
LET env_beh((key, value), next) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#lookup, $key) : [ SEND value TO ok ]
    (#lookup, _) : [ SEND msg TO next ]
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_env: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_lookup == BEH(r->req)) {  // (#lookup, _)
        ReqLookup rl = (ReqLookup)r->req;
        TRACE(fprintf(stderr, "expr_env: (#lookup, _)\n"));
        // Actor value = dict_lookup(SPONSOR(e), SELF(e), rl->key);
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        dict_lookup(groundout, NOTHING, SELF(e), rl->key);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor value = effect->message;
        TRACE(fprintf(stderr, "expr_env: (#lookup, %p) -> %p\n", rl->key, value));
        if (value == NOTHING) {
            TRACE(fprintf(stderr, "expr_env: FAIL!\n"));
            config_send(e, r->fail, (Actor)e);
        } else {
            config_send(e, r->ok, value);
        }
    } else if (val_req_bind == BEH(r->req)) {  // (#bind, key, value)
        ReqBind rb = (ReqBind)r->req;
        TRACE(fprintf(stderr, "expr_env: (#bind, %p -> %p)\n", rb->key, rb->value));
        // Actor dict = dict_bind(SPONSOR(e), SELF(e), rb->key, rb->value);
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        dict_bind(groundout, NOTHING, SELF(e), rb->key, rb->value);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor dict = effect->message;
        config_send(e, r->ok, dict);
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "ptrn_skip: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_match == BEH(r->req)) {  // (#match, _, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "ptrn_skip: (#match, %p, %p)\n", rm->value, rm->env));
        config_send(e, r->ok, rm->env);
    } else {
        expr_value(e);  // delegate
    }
}
/**
CREATE skip_ptrn WITH skip_ptrn_beh
**/
ACTOR the_ptrn_skip_actor = ACTOR_DECL(ptrn_skip);

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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "ptrn_bind: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "ptrn_bind: (#match, %p, %p)\n", rm->value, rm->env));
        // config_send(e, rm->env, req_bind_new(SPONSOR(e), r->ok, r->fail, name, rm->value));
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        req_bind_new(groundout, NOTHING, r->ok, r->fail, name, rm->value);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        config_send(e, rm->env, effect->message);
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_name: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, env)
        ReqEval re = (ReqEval)r->req;
        TRACE(fprintf(stderr, "expr_name: (#eval, %p)\n", re->env));
        // config_send(e, re->env, req_lookup_new(SPONSOR(e), r->ok, r->fail, SELF(e)));
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        req_lookup_new(groundout, NOTHING, r->ok, r->fail, SELF(e));
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        config_send(e, re->env, effect->message);
    } else {
        TRACE(fprintf(stderr, "expr_name: FAIL!\n"));
        config_send(e, r->fail, (Actor)e);
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
    if (beh_pair != BEH(a)) { config_fail(SPONSOR(e), e_inval); }  // (cust, body) required
    Pair p = (Pair)a;
    Actor body = p->t;
    a = p->h;
    if (beh_pair != BEH(a)) { config_fail(SPONSOR(e), e_inval); }  // (ok, fail) required
    Pair cust = (Pair)a;
    Actor ok = cust->h;
    Actor fail = cust->t;
    Actor env = MSG(e);  // (env)
    TRACE(fprintf(stderr, "beh_eval_body: ok=%p, fail=%p, body=%p, env=%p\n", ok, fail, body, env));
    // config_send(e, body, req_eval_new(SPONSOR(e), ok, fail, env));
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    req_eval_new(groundout, NOTHING, ok, fail, env);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    config_send(e, body, effect->message);
}
static inline void
val_expect(Event e)
{
    TRACE(fprintf(stderr, "val_expect{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor expect = DATA(SELF(e));
    Actor actual = MSG(e);
    if (expect != actual) {
        TRACE(fprintf(stderr, "val_expect: %p != %p\n", expect, actual));
        config_fail(SPONSOR(e), e_inval);  // unexpected message
    }
}
#define a_empty_env (a_empty_dict)
void
test_expr()
{
    Actor expr;
    Actor cust;

    TRACE(fprintf(stderr, "---- test_expr ----\n"));
    // Config cfg = quota_config_new(a_root_config, 8000);
    Event bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING);
    a_groundout_event->message = NOTHING; // reset groundout event
    quota_config_new(bootstrap, NOTHING, 8000);
    Event effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Config cfg = (Config)effect->message;
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    TRACE(fprintf(stderr, "a_empty_env = %p\n", a_empty_env));
    /* empty environment evaluates to itself */
    expr = a_empty_env;
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    // cust = value_new(cfg, val_expect, a_empty_env);
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event
    value_new(bootstrap, NOTHING, val_expect, a_empty_env);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    cust = effect->message;
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    // bootstrapping event to provide SPONSOR(e) only
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event
    // config_send(bootstrap, expr, req_eval_new(cfg, cust, a_halt, a_empty_env));
    req_eval_new(bootstrap, NOTHING, cust, a_halt, a_empty_env);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Actor req = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event    
    config_send(bootstrap, expr, req);
    Event bootstrap2 = config_event_new(a_groundout_event, NOTHING, NOTHING);
    a_groundout_event->message = NOTHING; // reset groundout event       
    // config_apply_effects(cfg, bootstrap);
    config_apply_effects(bootstrap2, cfg, bootstrap);
    /* the configuration evaluates to itself */
    expr = (Actor)cfg;
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    // cust = value_new(cfg, val_expect, cfg);
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event  
    value_new(bootstrap, NOTHING, val_expect, cfg);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    cust = effect->message;
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event  
    // config_send(bootstrap, expr, req_eval_new(cfg, cust, a_halt, a_empty_env));
    req_eval_new(bootstrap, NOTHING, cust, a_halt, a_empty_env);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    req = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    config_send(bootstrap, expr, req);    
    // config_apply_effects(cfg, bootstrap);
    bootstrap2 = config_event_new(a_groundout_event, NOTHING, NOTHING);
    a_groundout_event->message = NOTHING; // reset groundout event       
    config_apply_effects(bootstrap2, cfg, bootstrap);
    /* dispatch until empty */
    config_dispatch(cfg);
    config_dispatch(cfg);
    config_dispatch(cfg);
    config_dispatch(cfg);
    config_dispatch(cfg);
    config_dispatch(cfg);
    return;
    // while (config_dispatch(cfg) != NOTHING)
    // ;

    /* name binding can be resolved */
    // cust = value_new(cfg, val_expect, cfg);    
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event     
    value_new(bootstrap, NOTHING, val_expect, cfg);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    cust = effect->message;
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    // Actor s_x = actor_new(cfg, expr_name);
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event    
    actor_new(bootstrap, NOTHING, expr_name);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Actor s_x = effect->message;
    TRACE(fprintf(stderr, "s_x = %p\n", s_x));
    // expr = value_new(cfg, beh_eval_body, pair_new(cfg, pair_new(cfg, cust, a_halt), s_x));
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event
    pair_new(bootstrap, NOTHING, cust, a_halt);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Actor p = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    pair_new(bootstrap, NOTHING, p, s_x);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    p = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    value_new(bootstrap, NOTHING, beh_eval_body, p);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    expr = effect->message;
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    // config_send(bootstrap, a_empty_dict, req_bind_new(cfg, expr, a_halt, s_x, (Actor)cfg));
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    req_bind_new(bootstrap, NOTHING, expr, a_halt, s_x, (Actor)cfg);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Actor rb = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    config_send(bootstrap, a_empty_dict, rb);    
    // config_apply_effects(cfg, bootstrap);
    bootstrap2 = config_event_new(a_groundout_event, NOTHING, NOTHING);
    a_groundout_event->message = NOTHING; // reset groundout event    
    config_apply_effects(bootstrap2, cfg, bootstrap);   
    /* dispatch until empty */
    while (config_dispatch(cfg) != NOTHING)
        ;

    /* pair values can be matched */
    // cust = value_new(cfg, val_expect, a_empty_env);
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event         
    value_new(bootstrap, NOTHING, val_expect, a_empty_env);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    cust = effect->message;
    TRACE(fprintf(stderr, "cust = %p\n", cust));
    // Actor p = pair_new(cfg, a_true, a_false);
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    pair_new(bootstrap, NOTHING, a_true, a_false);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    p = effect->message;    
    TRACE(fprintf(stderr, "p = %p\n", p));
    // Actor q = pair_new(cfg, a_true, a_false);
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    pair_new(bootstrap, NOTHING, a_true, a_false);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Actor q = effect->message;    
    TRACE(fprintf(stderr, "q = %p\n", q));
    if (p == q) { halt("expected p != q"); }
    // config_send(bootstrap, p, req_match_new(cfg, cust, a_halt, p, a_empty_env));  // match p to itself
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    req_match_new(bootstrap, NOTHING, cust, a_halt, p, a_empty_env);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    Actor rm = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    config_send(bootstrap, p, rm);
    // config_apply_effects(cfg, bootstrap);
    bootstrap2 = config_event_new(a_groundout_event, NOTHING, NOTHING);
    a_groundout_event->message = NOTHING; // reset groundout event    
    config_apply_effects(bootstrap2, cfg, bootstrap);      
    // config_send(bootstrap, q, req_match_new(cfg, cust, a_halt, p, a_empty_env));  // match p to q
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    req_match_new(bootstrap, NOTHING, cust, a_halt, p, a_empty_env);
    effect = (Event)((Pair)((Pair)((Pair)bootstrap->events)->h)->h);
    rm = effect->message;
    bootstrap = config_event_new(a_groundout_event, NOTHING, NOTHING); 
    a_groundout_event->message = NOTHING; // reset groundout event 
    config_send(bootstrap, q, rm);    
    // config_apply_effects(cfg, bootstrap);
    bootstrap2 = config_event_new(a_groundout_event, NOTHING, NOTHING);
    a_groundout_event->message = NOTHING; // reset groundout event    
    config_apply_effects(bootstrap2, cfg, bootstrap);          
    /* dispatch until empty */
    while (config_dispatch(cfg) != NOTHING)
        ;
}
