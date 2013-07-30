/*

actor.c -- Tiny Actor Run-Time

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

#include "actor.h"

void
beh_halt(Event e)
{
    TRACE(fprintf(stderr, "beh_halt{event=%p}\n", e));
    halt("HALT!");
}

void
beh_pair(Event e)
{
    TRACE(fprintf(stderr, "beh_pair{event=%p}\n", e));
    expr_value(e);  // DON'T PANIC!
}

PAIR the_nil_pair = {
    { beh_pair },
    &the_nil_pair,
    &the_nil_pair
};

inline Pair
pair_new(Any h, Any t)
{
    Pair p = NEW(PAIR);
    BEH(p) = beh_pair;
    p->h = h;
    p->t = t;
    return p;
}

inline Pair
list_new()
{
    return NIL;
}
inline int
list_empty_p(Pair list)
{
//    if (beh_pair != BEH(list)) { halt("list_empty_p: list required"); }
    return (list == NIL);
}
inline Pair
list_pop(Pair list)
{
    if (beh_pair != BEH(list)) { halt("list_pop: pair required"); }
    return list;
}
inline Pair
list_push(Pair list, Any item)
{
    if (beh_pair != BEH(list)) { halt("list_push: pair required"); }
    return PR(item, list);
}

inline Pair
deque_new()
{
    return PR(NIL, NIL);
}
inline int
deque_empty_p(Pair q)
{
    if (beh_pair != BEH(q)) { halt("deque_empty_p: pair required"); }
    return (q->h == NIL);
}
inline void
deque_give(Pair q, Any item)
{
    if (beh_pair != BEH(q)) { halt("deque_give: pair required"); }
    Pair p = PR(item, NIL);
    if (deque_empty_p(q)) {
        q->h = p;
    } else {
        Pair t = q->t;
        t->t = p;
    }
    q->t = p;
}
inline Any
deque_take(Pair q)
{
    if (beh_pair != BEH(q)) { halt("deque_take: pair required"); }
    if (deque_empty_p(q)) {
        halt("deque_take from empty!");
    }
    Pair p = q->h;
    Any item = p->h;
    q->h = p->t;
    p = FREE(p);
    return item;
}
inline void
deque_return(Pair q, Any item)
{
    if (beh_pair != BEH(q)) { halt("deque_return: pair required"); }
    Pair p = PR(item, q->h);
    if (deque_empty_p(q)) {
        q->t = p;
    }
    q->h = p;
}
inline Any
deque_lookup(Pair q, int i)
{
    if (beh_pair != BEH(q)) { halt("deque_lookup: pair required"); }
    Pair p = q->h;
    while (p != NIL) {
        if (beh_pair != BEH(p)) { halt("deque_lookup: non-pair in chain"); }
        if (i <= 0) {
            return p->h;
        }
        --i;
        p = p->t;
    }
    return NULL;  // not found
}
inline void
deque_bind(Pair q, int i, Any item)
{
    if (beh_pair != BEH(q)) { halt("deque_bind: pair required"); }
    Pair p = q->h;
    while (p != NIL) {
        if (beh_pair != BEH(p)) { halt("deque_bind: non-pair in chain"); }
        if (i <= 0) {
            p->h = item;
        }
        --i;
        p = p->t;
    }
    // not found
}

inline Pair
dict_new()
{
    return NIL;
}
inline int
dict_empty_p(Pair dict)
{
    return (dict == NIL);
}
inline Any
dict_lookup(Pair dict, Any key)
{
    while (!dict_empty_p(dict)) {
        if (beh_pair != BEH(dict)) { halt("dict_lookup: non-pair in chain"); }
        Pair p = dict->h;
        if (p->h == key) {
            return p->t;
        }
        dict = dict->t;
    }
    return NULL;  // NOT FOUND
}
inline Pair
dict_bind(Pair dict, Any key, Any value)
{
    if (beh_pair != BEH(dict)) { halt("dict_bind: pair required"); }
    return PR(PR(key, value), dict);
}

inline Actor
value_new(Action beh, Any data)  // create a "unserialized" (value) actor
{
    Value v = NEW(VALUE);
    CODE(v) = beh;
    DATA(v) = data;
    return (Actor)v;
}
inline Actor
serial_new(Action beh, Any data)  // create a "serialized" actor
{
    Serial s = NEW(SERIAL);
    BEH(s) = act_serial;
    VALUE(s) = value_new(beh, data);
    return (Actor)s;
}
inline Actor
serial_with_value(Actor v)  // create a "serialized" actor with "behavior" value
{
    Serial s = NEW(SERIAL);
    BEH(s) = act_serial;
    VALUE(s) = v;
    return (Actor)s;
}
inline void
actor_become(Actor s, Actor v)
{
    if (act_serial != BEH(s)) { halt("actor_become: serialized actor required"); }
    VALUE(s) = v;  // an "unserialzed" behavior actor
}

void
beh_event(Event e)
{
    TRACE(fprintf(stderr, "beh_event{event=%p}\n", e));
    beh_halt(e);
}
inline Event
event_new(Config cfg, Actor a, Any msg)
{
    if (beh_config != BEH(cfg)) { halt("event_new: config actor required"); }
    Event e = NEW(EVENT);
    BEH(e) = beh_event;
    e->sponsor = cfg;
    e->actor = a;
    e->message = msg;
    return e;
}

void
beh_config(Event e)
{
    TRACE(fprintf(stderr, "beh_config{event=%p}\n", e));
    beh_halt(e);
}
inline Config
config_new()
{
    Config cfg = NEW(CONFIG);
    BEH(cfg) = beh_config;
    cfg->event_q = deque_new();
    cfg->actors = NIL;
    return cfg;
}
inline void
config_enqueue(Config cfg, Event e)
{
    if (beh_event != BEH(e)) { halt("config_enqueue: event actor required"); }
    deque_give(cfg->event_q, e);
}
inline void
config_enlist(Config cfg, Actor a)
{
    cfg->actors = list_push(cfg->actors, a);
}
inline void
config_send(Config cfg, Actor target, Any msg)
{
    if (beh_config != BEH(cfg)) { halt("config_send: config actor required"); }
    TRACE(fprintf(stderr, "config_send: actor=%p, msg=%p\n", target, msg));
    config_enqueue(cfg, event_new(cfg, target, msg));
}
int
config_dispatch(Config cfg)
{
    if (beh_config != BEH(cfg)) { halt("config_dispatch: config actor required"); }
    if (deque_empty_p(cfg->event_q)) {
        TRACE(fprintf(stderr, "config_dispatch: <EMPTY>\n"));
        return 0;
    }
    Event e = deque_take(cfg->event_q);
    TRACE(fprintf(stderr, "config_dispatch: event=%p, actor=%p, msg=%p\n", e, SELF(e), MSG(e)));
    (CODE(SELF(e)))(e);  // INVOKE BEHAVIOR
    return 1;
}

void
act_serial(Event e)  // "serialized" actor behavior
{
    TRACE(fprintf(stderr, "act_serial{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    (STRATEGY(SELF(e)))(e);  // INVOKE SERIALIZED BEHAVIOR
}

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}

VALUE the_halt_actor = { { beh_halt }, &the_halt_actor };
VALUE the_ignore_actor = { { beh_ignore }, NOTHING };

void
val_request(Event e)
{
    TRACE(fprintf(stderr, "val_request{event=%p}\n", e));
    Request r = (Request)SELF(e);
    TRACE(fprintf(stderr, "val_request: ok=%p, fail=%p, req=%p\n", r->ok, r->fail, r->req));
    beh_halt(e);
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
    beh_halt(e);
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
    TRACE(fprintf(stderr, "val_req_match: value=&p, env=%p\n", req->value, req->env));
    beh_halt(e);
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
    beh_halt(e);
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
    beh_halt(e);
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
    beh_halt(e);
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

/**
LET value_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
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
        config_send(e->sponsor, r->ok, SELF(e));
    } else {
        TRACE(fprintf(stderr, "expr_value: FAIL!\n"));
        config_send(e->sponsor, r->fail, e);
    }
}
