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

inline Actor
actor_eqv_method(Config cfg, Actor this, Actor that)
{
    return (this == that) ? a_true : a_false;
}

PAIR the_nil_pair_actor = { ACTOR_DECL(beh_pair), NIL, NIL };
inline Actor
pair_new(Config cfg, Actor h, Actor t)
{
    Pair p = (Pair)config_create(cfg, sizeof(PAIR), beh_pair);
    p->h = h;
    p->t = t;
    return (Actor)p;
}

inline Actor
deque_new(Config cfg)
{
    Pair p = (Pair)config_create(cfg, sizeof(PAIR), beh_deque);
    p->h = NIL;
    p->t = NIL;
    return (Actor)p;
}
inline Actor
deque_empty_p(Config cfg, Actor queue)
{
    if (beh_deque != BEH(queue)) { config_fail(cfg, e_inval); }  // deque required
    Pair q = (Pair)queue;
    return ((q->h == NIL) ? a_true : a_false);
}
inline void
deque_give(Config cfg, Actor queue, Actor item)
{
    if (beh_deque != BEH(queue)) { config_fail(cfg, e_inval); }  // deque required
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
    if (deque_empty_p(cfg, queue) != a_false) { config_fail(cfg, e_inval); }  // deque_take from empty
//    if (beh_deque != BEH(queue)) { config_fail(cfg, e_inval); }  // deque required
    Pair q = (Pair)queue;
    Pair p = (Pair)(q->h);
    Actor item = p->h;
    q->h = p->t;
    config_destroy(cfg, (Actor)p);
    return item;
}

ACTOR the_empty_dict_actor = ACTOR_DECL(expr_env_empty);
inline Actor
dict_lookup(Config cfg, Actor dict, Actor key)
{
    while (dict_empty_p(cfg, dict) == a_false) {
        if (expr_env != BEH(dict)) { config_fail(cfg, e_inval); }  // non-dict in chain
        Pair p = (Pair)dict;
        Actor a = p->h;
        if (beh_pair != BEH(a)) { config_fail(cfg, e_inval); }  // non-pair entry
        Pair q = (Pair)a;
        if (actor_eqv(cfg, key, q->h) == a_true) {
            return q->t;  // value
        }
        dict = p->t;  // next
    }
    return NOTHING;  // not found
}
inline Actor
dict_bind(Config cfg, Actor dict, Actor key, Actor value)
{
    Pair p = (Pair)config_create(cfg, sizeof(PAIR), expr_env);
    p->h = pair_new(cfg, key, value);
    p->t = dict;
    return (Actor)p;
}

void
beh_fifo(Event e)
{
    TRACE(fprintf(stderr, "beh_fifo{event=%p}\n", e));
    expr_value(e);
}
inline Actor
fifo_new(Config cfg, size_t n)  // WARNING! n must be a power of 2
{
    size_t b = sizeof(FIFO) + (n * sizeof(Actor));
    Fifo f = (Fifo)config_create(cfg, b, beh_fifo);
    f->h = 0;
    f->t = 0;
    f->m = (n - 1);
    return (Actor)f;
}
inline Actor
fifo_empty_p(Config cfg, Actor q)
{
    if (beh_fifo != BEH(q)) { config_fail(cfg, e_inval); }  // fifo required
    Fifo f = (Fifo)q;
    return (f->h == f->t) ? a_true : a_false;
}
inline Actor
fifo_give(Config cfg, Actor q, Actor item)
{
    if (beh_fifo != BEH(q)) { config_fail(cfg, e_inval); }  // fifo required
    Fifo f = (Fifo)q;
    size_t h = f->h;
    size_t t = f->t;
    size_t tt = (t + 1) & f->m;
    if (h == tt) {
        return a_false;
    }
    f->p[t] = item;
    f->t = tt;
    return a_true;
}
inline Actor
fifo_take(Config cfg, Actor q)
{
    if (fifo_empty_p(cfg, q) == a_true) { config_fail(cfg, e_inval); }  // fifo_take from empty!
    Fifo f = (Fifo)q;
    size_t h = f->h;
    Actor item = f->p[h];
    f->h = (h + 1) & f->m;
    return item;
}

inline Actor
actor_new(Config cfg, Action beh)  // create an actor with only a behavior procedure
{
    return config_create(cfg, sizeof(ACTOR), beh);
}
inline Actor
value_new(Config cfg, Action beh, Any data)  // create a "unserialized" (value) actor
{
    Value v = (Value)config_create(cfg, sizeof(VALUE), beh);
//    CODE(v) = beh;  <-- already set by config_create()
    DATA(v) = data;
    return (Actor)v;
}
inline Actor
serial_with_value(Config cfg, Actor v)  // create a "serialized" actor with "behavior" value
{
    Serial s = (Serial)config_create(cfg, sizeof(SERIAL), act_serial);
    s->beh_now = v;  // an "unserialzed" behavior actor
    return (Actor)s;
}
inline Actor
serial_new(Config cfg, Action beh, Any data)  // create a "serialized" actor
{
    return serial_with_value(cfg, value_new(cfg, beh, data));
}
void
act_serial(Event e)  // "serialized" actor behavior
{
    TRACE(fprintf(stderr, "act_serial{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Serial s = (Serial)SELF(e);
    s->beh_next = s->beh_now;  // default behavior for next event
    (CODE(s->beh_now))(e);  // INVOKE CURRENT SERIALIZED BEHAVIOR
    s->beh_now = s->beh_next;  // commit behavior change (if any)
}

inline void
actor_become(Event e, Actor v)
{
    if (act_serial != BEH(SELF(e))) { config_fail(SPONSOR(e), e_inval); }  // serialized actor required
    Serial s = (Serial)SELF(e);
    s->beh_next = v;  // remember "behavior" for later commit
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
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); }  // config actor required
    Event e = (Event)config_create(cfg, sizeof(EVENT), beh_event);
    e->sponsor = cfg;
    e->target = a;
    e->message = m;
    e->actors = deque_new(cfg);
    e->events = deque_new(cfg);
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
    TRACE(fprintf(stderr, "config_fail: cfg=%p, reason=%p\n", cfg, reason));
    halt("root_config_fail!");
}
inline static Actor
root_config_create(Config cfg, size_t n_bytes, Action beh)
{
    TRACE(fprintf(stderr, "root_config_create: n_bytes=%d\n", (int)n_bytes));
    Actor a = ALLOC(n_bytes);
    if (!a) { config_fail(cfg, e_nomem); }
    return ACTOR_INIT(a, beh);
}
inline static void
root_config_destroy(Config cfg, Actor victim)
{
    FREE(victim);
}
inline static void
root_config_send(Event e, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "config_send: actor=%p, msg=%p\n", target, msg));
    deque_give(SPONSOR(e), e->events, event_new(SPONSOR(e), target, msg));
}
inline static Actor
root_config_event_new(Event e, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "config_event_new: actor=%p, msg=%p\n", target, msg));
    Event ne = (Event)config_create(SPONSOR(e), sizeof(EVENT), beh_event);
    ne->sponsor = SPONSOR(e);
    ne->target = target;
    ne->message = msg;
    ne->actors = deque_new(SPONSOR(e));
    ne->events = deque_new(SPONSOR(e));
    return (Actor)ne;
}
static inline Actor
config_dequeue(Config cfg)
{
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); }  // config actor required
    if (deque_empty_p(cfg, cfg->events) != a_false) {
        TRACE(fprintf(stderr, "config_dequeue: <EMPTY>\n"));
        return NOTHING;
    }
    Actor a = deque_take(cfg, cfg->events);
    return a;
}
static inline Actor
effects_events_dequeue(Config cfg, Event e) 
{
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); } // config actor required
    if (deque_empty_p(cfg, e->events) != a_false) {
        TRACE(fprintf(stderr, "effects_events_dequeue: <EMPTY>\n"));
        return NOTHING;
    }
    Actor a = deque_take(cfg, e->events);
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
        config_apply_effects(cfg, e);
    }
    return a;
}
void
config_apply_effects(Config cfg, Event e)
{
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); } // config actor required
    if (beh_event != BEH(e)) { config_fail(cfg, e_inval); } // event actor required
    Actor e_fx;
    while ((e_fx = effects_events_dequeue(cfg, e)) != NOTHING) {
        TRACE(fprintf(stderr, "config_dispatch: applying effect event=%p\n", e_fx));
        config_enqueue(cfg, e_fx);        
    }
}
static PAIR the_root_event_q = { ACTOR_DECL(beh_deque), NIL, NIL };
CONFIG the_root_config = {
    ACTOR_DECL(beh_config), 
    root_config_fail, 
    root_config_create, 
    root_config_destroy, 
    root_config_send, 
    root_config_event_new,
    ((Actor)&the_root_event_q)
};

struct quota_config {
    CONFIG      _cfg;
    size_t      n_bytes;  // available storage
    size_t      mask;  // allocation/alignment granularity mask (2^n - 1)
    uint8_t *   free;  // pointer to start of next allocation
    uint8_t     memory[0];  // managed memory
};
inline static void
quota_config_fail(Config cfg, Actor reason)
{
    TRACE(fprintf(stderr, "quota_config_fail: cfg=%p, reason=%p\n", cfg, reason));
    halt("quota_config_fail!");
}
inline static Actor
quota_config_create(Config cfg, size_t n_bytes, Action beh)
{
    struct quota_config * self = (struct quota_config *)cfg;
    n_bytes = (n_bytes + self->mask) & ~self->mask;  // round up to next quanta
    DEBUG(fprintf(stderr, "quota_config_create: n_bytes=%d\n", (int)n_bytes));
    if (self->n_bytes < n_bytes) { config_fail(cfg, e_nomem); }
    Actor a = (Actor)(self->free);
    self->free += n_bytes;
    self->n_bytes -= n_bytes;
    return ACTOR_INIT(a, beh);
}
inline static void
quota_config_destroy(Config cfg, Actor victim)
{
    // quota is not re-usable in this implementation
}
inline static void
quota_config_send(Event e, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "quota_config_send: actor=%p, msg=%p\n", target, msg));
    deque_give(SPONSOR(e), e->events, event_new(SPONSOR(e), target, msg));
}
inline static Actor 
quota_config_event_new(Event e, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "config_event_new: actor=%p, msg=%p\n", target, msg));
    Event ne = (Event)config_create(SPONSOR(e), sizeof(EVENT), beh_event);
    ne->sponsor = SPONSOR(e);
    ne->target = target;
    ne->message = msg;
    ne->actors = deque_new(SPONSOR(e));
    ne->events = deque_new(SPONSOR(e));
    return (Actor)ne;
}
inline Config
quota_config_new(Config sponsor, size_t n_bytes)
{
    Config cfg = (Config)config_create(sponsor, sizeof(struct quota_config) + n_bytes, beh_config);
    TRACE(fprintf(stderr, "quota_config_new: sponsor=%p cfg=%p\n", sponsor, cfg));
    struct quota_config * self = (struct quota_config *)cfg;
    self->n_bytes = n_bytes;  // available storage
    self->mask = (1 << 4) - 1;  // allocation/alignment granularity mask (2^n - 1)
    self->free = &self->memory[0];  // pointer to start of next allocation
    TRACE(fprintf(stderr, "quota_config_new: n_bytes=%d\n", (int)n_bytes));
    cfg->fail = quota_config_fail;  // error reporting procedure
    cfg->create = quota_config_create;  // actor creation procedure
    cfg->destroy = quota_config_destroy;  // reclaim actor resources
    cfg->send = quota_config_send;  // message send procedure
    cfg->event_new = quota_config_event_new; // event creation procedure
    cfg->events = deque_new(cfg);
    return cfg;
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
    config_send(e, tail, (Actor)r);
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "beh_pair: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "beh_pair: (#eval, _)\n"));
        config_send(e, r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "beh_pair: (#match, %p, %p)\n", rm->value, rm->env));
        if (SELF(e) == rm->value) {
            config_send(e, r->ok, rm->env);
        } else if (beh_pair == BEH(rm->value)) {
            Pair p = (Pair)SELF(e);
            Pair q = (Pair)rm->value;
            Actor ok = value_new(SPONSOR(e), beh_pair_0, req_match_new(SPONSOR(e), r->ok, r->fail, q->t, p->t));
            config_send(e, p->h, req_match_new(SPONSOR(e), ok, r->fail, q->h, rm->env));
        } else {
            TRACE(fprintf(stderr, "beh_pair: MISMATCH!\n"));
            config_send(e, r->fail, (Actor)e);
        }
    } else if (val_req_read == BEH(r->req)) {  // (#read)
        Pair p = (Pair)SELF(e);
        TRACE(fprintf(stderr, "beh_pair: (#read) -> (%p, %p)\n", p->h, p->t));
        config_send(e, r->ok, (Actor)list_pop(SPONSOR(e), SELF(e)));
    } else if (val_req_write == BEH(r->req)) {  // (#write, value)
        ReqWrite rw = (ReqWrite)r->req;
        TRACE(fprintf(stderr, "beh_pair: (#write, %p)\n", rw->value));
        config_send(e, r->ok, list_push(SPONSOR(e), SELF(e), rw->value));
    } else {
        TRACE(fprintf(stderr, "beh_pair: FAIL!\n"));
        config_send(e, r->fail, (Actor)e);
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "beh_deque: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "beh_deque: (#eval, _)\n"));
        config_send(e, r->ok, SELF(e));
    } else if (val_req_match == BEH(r->req)) {  // (#match, value, env)
        ReqMatch rm = (ReqMatch)r->req;
        TRACE(fprintf(stderr, "beh_deque: (#match, %p, %p)\n", rm->value, rm->env));
        if (SELF(e) == rm->value) {
            config_send(e, r->ok, rm->env);
        } else {
            TRACE(fprintf(stderr, "beh_deque: MISMATCH!\n"));
            config_send(e, r->fail, (Actor)e);
        }
    } else if (val_req_read == BEH(r->req)) {  // (#read)
        Pair p = (Pair)SELF(e);
        TRACE(fprintf(stderr, "beh_deque: (#read) -> (%p, %p)\n", p->h, p->t));
        config_send(e, r->ok, PR(deque_take(SPONSOR(e), SELF(e)), SELF(e)));
    } else if (val_req_write == BEH(r->req)) {  // (#write, value)
        ReqWrite rw = (ReqWrite)r->req;
        TRACE(fprintf(stderr, "beh_deque: (#write, %p)\n", rw->value));
        deque_give(SPONSOR(e), SELF(e), rw->value);
        config_send(e, r->ok, SELF(e));
    } else {
        TRACE(fprintf(stderr, "beh_deque: FAIL!\n"));
        config_send(e, r->fail, (Actor)e);
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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "comb_true: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_combine == BEH(r->req)) {  // (#comb, (expr, _), env)
        ReqCombine rc = (ReqCombine)r->req;
        TRACE(fprintf(stderr, "comb_true: (#comb, %p, %p)\n", rc->opnd, rc->env));
        if (beh_pair == BEH(rc->opnd)) {
            Pair p = (Pair)rc->opnd;
            Actor expr = p->h;
            config_send(e, expr, req_eval_new(SPONSOR(e), r->ok, r->fail, rc->env));
        } else {
            TRACE(fprintf(stderr, "comb_true: opnd must be a Pair\n"));
            config_send(e, r->fail, (Actor)e);
        }
    } else {
        expr_value(e);  // delegate
    }
}
/**
CREATE true_oper WITH oper_true_beh
**/
ACTOR the_true_actor = ACTOR_DECL(comb_true);

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
    if (val_request != BEH(MSG(e))) { config_fail(SPONSOR(e), e_inval); }  // request msg required
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "comb_false: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_combine == BEH(r->req)) {  // (#comb, (_, expr), env)
        ReqCombine rc = (ReqCombine)r->req;
        TRACE(fprintf(stderr, "comb_false: (#comb, %p, %p)\n", rc->opnd, rc->env));
        if (beh_pair == BEH(rc->opnd)) {
            Pair p = (Pair)rc->opnd;
            Actor expr = p->t;
            config_send(e, expr, req_eval_new(SPONSOR(e), r->ok, r->fail, rc->env));
        } else {
            TRACE(fprintf(stderr, "comb_false: opnd must be a Pair\n"));
            config_send(e, r->fail, (Actor)e);
        }
    } else {
        expr_value(e);  // delegate
    }
}
/**
CREATE false_oper WITH oper_false_beh
**/
ACTOR the_false_actor = ACTOR_DECL(comb_false);

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}
ACTOR the_ignore_actor = ACTOR_DECL(beh_ignore);

void
beh_halt(Event e)
{
    TRACE(fprintf(stderr, "beh_halt{event=%p}\n", e));
    config_fail(SPONSOR(e), e_inval);  // HALT!
}
VALUE the_halt_actor = { ACTOR_DECL(beh_halt), NOTHING };  // qualifies as both VALUE and SERIAL

ACTOR fail_reason_nomem = ACTOR_DECL(expr_value);  // "out of memory" failure reason
ACTOR fail_reason_inval = ACTOR_DECL(expr_value);  // "invalid argument" failure reason
