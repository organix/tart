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
#include "expr.h"
#include "number.h"

PAIR the_nil_pair_actor = { { beh_pair }, NIL, NIL };
inline Actor
pair_new(Config cfg, Actor h, Actor t)
{
    Pair p = CREATE(PAIR, beh_pair);
    p->h = h;
    p->t = t;
    return (Actor)p;
}

inline Actor
deque_new(Config cfg)
{
    TRACE(fprintf(stderr, "deque_new: cfg = %p\n", cfg));
    Actor a = pair_new(cfg, NIL, NIL);
    TRACE(fprintf(stderr, "deque_new: a = %p\n", a));
    BEH(a) = beh_deque;  // override pair behavior with deque behavior
    return a;
}
inline Actor
deque_empty_p(Config cfg, Actor queue)
{
    if (beh_deque != BEH(queue)) { halt("deque_empty_p: deque required"); }
    Pair q = (Pair)queue;
    return ((q->h == NIL) ? a_true : a_false);
}
inline void
deque_give(Config cfg, Actor queue, Actor item)
{
    if (beh_deque != BEH(queue)) { halt("deque_give: deque required"); }
    Pair q = (Pair)queue;
    Actor p = pair_new(cfg, item, NIL);
    if (q->h == NIL) {
        q->h = p;
    } else {
        Pair t = (Pair)(q->t);
        t->t = p;
    }
    q->t = p;
}
inline Actor
deque_take(Config cfg, Actor queue)
{
    if (deque_empty_p(cfg, queue) != a_false) { halt("deque_take from empty!"); }
//    if (beh_deque != BEH(queue)) { halt("deque_take: deque required"); }
    Pair q = (Pair)queue;
    Pair p = (Pair)(q->h);
    Actor item = p->h;
    q->h = p->t;
    FREE(p);  // [FIXME] de-allocation should also be done through the sponsor/config, or we count on garbage-collection
    return item;
}
inline void
deque_return(Config cfg, Actor queue, Actor item)
{
    if (beh_deque != BEH(queue)) { halt("deque_return: deque required"); }
    Pair q = (Pair)queue;
    Actor p = pair_new(cfg, item, q->h);
    if (q->h == NIL) {
        q->t = p;
    }
    q->h = p;
}
inline Actor
deque_lookup(Config cfg, Actor queue, Actor index)
{
    int i;

    if (beh_integer != BEH(index)) { halt("deque_lookup: index must be an integer"); }
    i = ((Integer)index)->i;
    if (beh_deque != BEH(queue)) { halt("deque_lookup: deque required"); }
    Pair q = (Pair)queue;
    Actor a = q->h;
    while (a != NIL) {
        if (beh_pair != BEH(a)) { halt("deque_lookup: non-pair in chain"); }
        Pair p = (Pair)a;
        if (i <= 0) {
            return p->h;
        }
        --i;
        a = p->t;
    }
    return NOTHING;  // not found
}
inline void
deque_bind(Config cfg, Actor queue, Actor index, Actor item)
{
    int i;

    if (beh_integer != BEH(index)) { halt("deque_lookup: index must be an integer"); }
    i = ((Integer)index)->i;
    if (beh_deque != BEH(queue)) { halt("deque_bind: deque required"); }
    Pair q = (Pair)queue;
    Actor a = q->h;
    while (a != NIL) {
        if (beh_pair != BEH(a)) { halt("deque_bind: non-pair in chain"); }
        Pair p = (Pair)a;
        if (i <= 0) {
            p->h = item;
            break;
        }
        --i;
        a = p->t;
    }
    // not found
}

ACTOR the_empty_dict_actor = { expr_env_empty };
inline Actor
dict_new(Config cfg)
{
    return a_empty_dict;
}
inline Actor
dict_empty_p(Config cfg, Actor dict)
{
    return ((dict == a_empty_dict) ? a_true : a_false);
}
inline Actor
dict_lookup(Config cfg, Actor dict, Actor key)
{
    while (dict_empty_p(cfg, dict) == a_false) {
        if (expr_env != BEH(dict)) { halt("dict_lookup: non-dict in chain"); }
        Pair p = (Pair)dict;
        Actor a = p->h;
        if (beh_pair != BEH(a)) { halt("dict_lookup: non-pair entry"); }
        Pair q = (Pair)a;
        if (q->h == key) {
            return q->t;  // value
        }
        dict = p->t;  // next
    }
    return NOTHING;  // not found
}
inline Actor
dict_bind(Config cfg, Actor dict, Actor key, Actor value)
{
    Actor a = pair_new(cfg, pair_new(cfg, key, value), dict);
    BEH(a) = expr_env;  // override pair behavior with env behavior
    return a;
}

inline Actor
actor_new(Action beh)  // create an actor with only a behavior procedure
{
    Actor a = NEW(ACTOR);
    BEH(a) = beh;
    return a;
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
    expr_value(e);
}
inline Actor
event_new(Config cfg, Actor a, Actor m)
{
    if (beh_config != BEH(cfg)) { halt("event_new: config actor required"); }
    Event e = NEW(EVENT);
    BEH(e) = beh_event;
    e->sponsor = cfg;
    e->target = a;
    e->message = m;
    return (Actor)e;
}

void
beh_config(Event e)
{
    TRACE(fprintf(stderr, "beh_config{event=%p}\n", e));
    expr_value(e);
}
inline static void
root_config_fail(Config cfg, Actor reason)
{
    TRACE(fprintf(stderr, "root_config_fail: cfg=%p, reason=%p\n", cfg, reason));
    halt("root_config_fail!");
}
inline static Actor
root_config_create(Config cfg, size_t n_bytes, Action beh)
{
    Actor a = ALLOC(n_bytes);
    BEH(a) = beh;
//    config_enlist(cfg, a);  <-- [FIXME] --- INFINITE RECURSION WHEN "CREATE"ING THE PAIR TO HOLD THE NEW ACTOR!!!
    return a;
}
inline static void
root_config_send(Config cfg, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "root_config_send: actor=%p, msg=%p\n", target, msg));
    config_enqueue(cfg, event_new(cfg, target, msg));
}
inline Config
config_new()
{
    Config cfg = NEW(CONFIG);
    TRACE(fprintf(stderr, "config_new: cfg = %p\n", cfg));
    BEH(cfg) = beh_config;
    cfg->fail = root_config_fail;  // error reporting procedure
    cfg->create = root_config_create;  // actor creation procedure
    cfg->send = root_config_send;  // event creation procedure
    cfg->events = deque_new(cfg);
    cfg->actors = list_new();
    return cfg;
}
Actor
config_dequeue(Config cfg)
{
    if (beh_config != BEH(cfg)) { halt("config_dispatch: config actor required"); }
    if (deque_empty_p(cfg, cfg->events) != a_false) {
        TRACE(fprintf(stderr, "config_dispatch: <EMPTY>\n"));
        return NOTHING;
    }
    Actor a = deque_take(cfg, cfg->events);
    return a;
}
Actor
config_dispatch(Config cfg)
{
    Actor a = config_dequeue(cfg);
    if (beh_event == BEH(a)) {
        Event e = (Event)a;
        TRACE(fprintf(stderr, "config_dispatch: event=%p, actor=%p, msg=%p\n", e, SELF(e), MSG(e)));
        (CODE(SELF(e)))(e);  // INVOKE ACTION PROCEDURE
    }
    return a;
}

void
act_serial(Event e)  // "serialized" actor behavior
{
    TRACE(fprintf(stderr, "act_serial{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    (STRATEGY(SELF(e)))(e);  // INVOKE SERIALIZED BEHAVIOR
}

/**
LET pair_beh_0((ok, fail), t, tail) = \env'.[
    SEND ((ok, fail), #match, t, env') TO tail
]
**/
static void
beh_pair_0(Event e)
{
    TRACE(fprintf(stderr, "beh_pair_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Request r = (Request)DATA(SELF(e));
    ReqMatch rm = (ReqMatch)r->req;
    Actor tail = rm->env;  // extract hijacked target
    rm->env = (Actor)MSG(e);  // replace with extended environment
    config_send(SPONSOR(e), tail, (Actor)r);
}
/**
LET pair_beh(head, tail) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    (#match, $SELF, env) : [ SEND env TO ok ]
    (#match, (h, t), env) : [
        SEND ((ok', fail), #match, h, env) TO head
        CREATE ok' WITH \env'.[
            SEND ((ok, fail), #match, t, env') TO tail
        ]
    ]
    (#read) : [ SEND list_pop(SELF) TO ok ]
    (#write, value) : [ SEND list_push(SELF, value) TO ok ]
    _ : [ SEND (SELF, msg) TO fail ]
    END
]
**/
void
beh_pair(Event e)
{
    TRACE(fprintf(stderr, "beh_pair{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("beh_pair: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "beh_pair: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "beh_pair: (#eval, _)\n"));
        config_send(SPONSOR(e), r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "beh_pair: (#match, %p, %p)\n", rm->value, rm->env));
        if (SELF(e) == rm->value) {
            config_send(SPONSOR(e), r->ok, rm->env);
        } else if (beh_pair == BEH(rm->value)) {
            Pair p = (Pair)SELF(e);
            Pair q = (Pair)rm->value;
            Actor ok = value_new(beh_pair_0, req_match_new(r->ok, r->fail, q->t, p->t));
            config_send(SPONSOR(e), p->h, req_match_new(ok, r->fail, q->h, rm->env));
        } else {
            TRACE(fprintf(stderr, "beh_pair: MISMATCH!\n"));
            config_send(SPONSOR(e), r->fail, (Actor)e);
        }
    } else if (val_req_read == BEH(r->req)) {  // (#read)
        Pair p = (Pair)SELF(e);
        TRACE(fprintf(stderr, "beh_pair: (#read) -> (%p, %p)\n", p->h, p->t));
        config_send(SPONSOR(e), r->ok, (Actor)list_pop(SELF(e)));
    } else if (val_req_write == BEH(r->req)) {  // (#write, value)
        ReqWrite rw = (ReqWrite)r->req;
        TRACE(fprintf(stderr, "beh_pair: (#write, %p)\n", rw->value));
        config_send(SPONSOR(e), r->ok, list_push(SELF(e), rw->value));
    } else {
        TRACE(fprintf(stderr, "beh_pair: FAIL!\n"));
        config_send(SPONSOR(e), r->fail, (Actor)e);
    }
}

/**
LET deque_beh(head, tail) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    (#match, $SELF, env) : [ SEND env TO ok ]
    (#read) : [ SEND (deque_take(SELF), SELF) TO ok ]
    (#write, value) : [
        LET _ = $deque_give(SELF, value)
        IN SEND SELF TO ok
    ]
    _ : [ SEND (SELF, msg) TO fail ]
    END
]
**/
void
beh_deque(Event e)
{
    TRACE(fprintf(stderr, "beh_deque{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("beh_deque: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "beh_deque: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "beh_deque: (#eval, _)\n"));
        config_send(SPONSOR(e), r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "beh_deque: (#match, %p, %p)\n", rm->value, rm->env));
        if (SELF(e) == rm->value) {
            config_send(SPONSOR(e), r->ok, rm->env);
        } else {
            TRACE(fprintf(stderr, "beh_deque: MISMATCH!\n"));
            config_send(SPONSOR(e), r->fail, (Actor)e);
        }
    } else if (val_req_read == BEH(r->req)) {  // (#read)
        Pair p = (Pair)SELF(e);
        TRACE(fprintf(stderr, "beh_deque: (#read) -> (%p, %p)\n", p->h, p->t));
        config_send(SPONSOR(e), r->ok, PR(deque_take(SPONSOR(e), SELF(e)), SELF(e)));
    } else if (val_req_write == BEH(r->req)) {  // (#write, value)
        ReqWrite rw = (ReqWrite)r->req;
        TRACE(fprintf(stderr, "beh_deque: (#write, %p)\n", rw->value));
        deque_give(SPONSOR(e), SELF(e), rw->value);
        config_send(SPONSOR(e), r->ok, SELF(e));
    } else {
        TRACE(fprintf(stderr, "beh_deque: FAIL!\n"));
        config_send(SPONSOR(e), r->fail, (Actor)e);
    }
}

/**
LET oper_true_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
	CASE req OF
	(#comb, (expr, _), env) : [
        SEND ((ok, fail), #eval, env) TO expr
    ]
	_ : value_beh(msg)  # delegate
	END
]
**/
static void
comb_true(Event e)
{
    TRACE(fprintf(stderr, "comb_true{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("comb_true: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "comb_true: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_combine == BEH(r->req)) {  // (#comb, (expr, _), env)
        ReqCombine rc = (ReqCombine)r->req;
        TRACE(fprintf(stderr, "comb_true: (#comb, %p, %p)\n", rc->opnd, rc->env));
        if (beh_pair == BEH(rc->opnd)) {
            Pair p = (Pair)rc->opnd;
            Actor expr = p->h;
            config_send(SPONSOR(e), expr, req_eval_new(r->ok, r->fail, rc->env));
        } else {
            TRACE(fprintf(stderr, "comb_true: opnd must be a Pair\n"));
            config_send(SPONSOR(e), r->fail, (Actor)e);
        }
    } else {
        expr_value(e);  // delegate
    }
}
/**
CREATE true_oper WITH oper_true_beh
**/
ACTOR the_true_actor = { comb_true };

/**
LET oper_false_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
	CASE req OF
	(#comb, (_, expr), env) : [
        SEND ((ok, fail), #eval, env) TO expr
    ]
	_ : value_beh(msg)  # delegate
	END
]
**/
static void
comb_false(Event e)
{
    TRACE(fprintf(stderr, "comb_false{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("comb_false: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "comb_false: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_combine == BEH(r->req)) {  // (#comb, (_, expr), env)
        ReqCombine rc = (ReqCombine)r->req;
        TRACE(fprintf(stderr, "comb_false: (#comb, %p, %p)\n", rc->opnd, rc->env));
        if (beh_pair == BEH(rc->opnd)) {
            Pair p = (Pair)rc->opnd;
            Actor expr = p->t;
            config_send(SPONSOR(e), expr, req_eval_new(r->ok, r->fail, rc->env));
        } else {
            TRACE(fprintf(stderr, "comb_false: opnd must be a Pair\n"));
            config_send(SPONSOR(e), r->fail, (Actor)e);
        }
    } else {
        expr_value(e);  // delegate
    }
}
/**
CREATE false_oper WITH oper_false_beh
**/
ACTOR the_false_actor = { comb_false };

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}
ACTOR the_ignore_actor = { beh_ignore };

void
beh_halt(Event e)
{
    TRACE(fprintf(stderr, "beh_halt{event=%p}\n", e));
    halt("HALT!");
}
VALUE the_halt_actor = { { beh_halt }, NOTHING };  // qualifies as both VALUE and SERIAL
