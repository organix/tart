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

inline void
actor_eqv_method(Event e, Actor cust, Actor this, Actor that)
{
    // return (this == that) ? a_true : a_false;
    if (this == that) {
        config_send(e, cust, a_true);
        return;
    } else {
        config_send(e, cust, a_false);
        return;
    }
}

PAIR the_nil_pair_actor = { ACTOR_DECL(beh_pair), NIL, NIL };
inline void
pair_new(Event e, Actor cust, Actor h, Actor t)
{
    // Pair p = (Pair)config_create(cfg, sizeof(PAIR), beh_pair);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    TRACE(fprintf(stderr, "pair_new: creating pair e=%p\n", e));
    config_create(groundout, NOTHING, sizeof(PAIR), beh_pair);
    TRACE(fprintf(stderr, "pair_new: pair created e=%p\n", e));
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Pair p = (Pair)effect->message;
    if (p == NOTHING) { 
        TRACE(fprintf(stderr, "pair_new: go->events=%p, go->es->h=%p, go->es->h->h=%p\n", groundout->events, ((Pair)groundout->events)->h, ((Pair)((Pair)groundout->events)->h)->h));
        TRACE(fprintf(stderr, "pair_new: sponsor=%p, e=%p, groundout=%p\n", SPONSOR(e), e, groundout));
        TRACE(fprintf(stderr, "pair_new: NOTHING=%p\n", NOTHING));
        halt("pair_new: created NOTHING!"); 
    }
    p->h = h;
    p->t = t;
    config_send(e, cust, (Actor)p);
    // return (Actor)p;
}

inline void
deque_new(Event e, Actor cust)
{
    TRACE(fprintf(stderr, "deque_new: e=%p\n", e));
    // Pair p = (Pair)config_create(cfg, sizeof(PAIR), beh_deque);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(PAIR), beh_deque);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Pair p = (Pair)effect->message;
    if (p == NOTHING) { halt("new deque is NOTHING!"); }
    p->h = NIL;
    p->t = NIL;
    if (e == &the_groundout_event) {
        e->message = (Actor)p;
    } else {
        config_send(e, cust, (Actor)p);
    }
    // return (Actor)p;
}
inline Actor
deque_empty_p(Config cfg, Actor queue)
{
    if (beh_deque != BEH(queue)) { config_fail(cfg, e_inval); }  // deque required
    Pair q = (Pair)queue;
    return ((q->h == NIL) ? a_true : a_false);
}
inline void
deque_give(Event e, Actor cust, Actor queue, Actor item)
{
    if (beh_deque != BEH(queue)) { 
        TRACE(fprintf(stderr, "deque_give: e_inval=%p, deque required\n", e_inval));
        config_fail(SPONSOR(e), e_inval); 
    }
    Pair q = (Pair)queue;
    if (q->h == NOTHING) {
        TRACE(fprintf(stderr, "deque_give: deque head is NOTHING!\n"));
    }
    // Actor p = pair_new(cfg, item, NIL);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    pair_new(groundout, NOTHING, item, NIL);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Actor p = effect->message;
    if (p == NOTHING) { 
        TRACE(fprintf(stderr, "deque_give: queue=%p, item=%p\n", queue, item));
        halt("deque_give: giving NOTHING to deque!"); 
    }
    if (q->h == NIL) {
        q->h = p;
    } else {
        Pair t = (Pair)(q->t);
        t->t = p;
    }
    q->t = p;
    TRACE(fprintf(stderr, "deque_give: q=%p, q->h=%p, p->h=%p, q->t=%p\n", q, q->h, ((Pair)p)->h, q->t));
    if (q->h == NOTHING) { halt("deque_give: deque head is NOTHING!"); }
}
inline Actor
deque_take(Config cfg, Actor queue)
{
    if (((Pair)queue)->h == NOTHING) { halt("deque_take(start): deque head is NOTHING!"); }
    if (deque_empty_p(cfg, queue) != a_false) { config_fail(cfg, e_inval); }  // deque_take from empty
//    if (beh_deque != BEH(queue)) { config_fail(cfg, e_inval); }  // deque required
    Pair q = (Pair)queue;
    Pair p = (Pair)(q->h);
    TRACE(fprintf(stderr, "deque_take: q=%p, q->h=%p, q->h->h=%p, q->t=%p\n", q, q->h, p->h, q->t));
    Actor item = p->h;
    q->h = p->t;
    TRACE(fprintf(stderr, "deque_take: item=%p, q=%p, q->h=%p, q->t=%p\n", item, q, q->h, q->t));
    config_destroy(cfg, (Actor)p);
    if (((Pair)queue)->h == NOTHING) { halt("deque_take(done): deque head is NOTHING!"); }
    return item;
}

ACTOR the_empty_dict_actor = ACTOR_DECL(expr_env_empty);
inline void
dict_lookup(Event e, Actor cust, Actor dict, Actor key)
{
    while (dict_empty_p(cfg, dict) == a_false) {
        if (expr_env != BEH(dict)) { config_fail(SPONSOR(e), e_inval); }  // non-dict in chain
        Pair p = (Pair)dict;
        Actor a = p->h;
        if (beh_pair != BEH(a)) { config_fail(SPONSOR(e), e_inval); }  // non-pair entry
        Pair q = (Pair)a;
        // if (actor_eqv(cfg, key, q->h) == a_true) {
        //     return q->t;  // value
        // }
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        actor_eqv(groundout, NOTHING, key, q->h);
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor eqv = effect->message;
        if (eqv == a_true) {
            config_send(e, cust, q->t); // value
        }
        dict = p->t;  // next
    }
    // return NOTHING;  // not found
    config_send(e, cust, NOTHING); // not found
}
inline void
dict_bind(Event e, Actor cust, Actor dict, Actor key, Actor value)
{
    // Pair p = (Pair)config_create(cfg, sizeof(PAIR), expr_env);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(PAIR), beh_pair);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Pair p = (Pair)effect->message;
    // p->h = pair_new(cfg, key, value);
    groundout = config_event_new(e, NOTHING, NOTHING);
    pair_new(groundout, NOTHING, key, value);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    p->h = effect->message;    
    p->t = dict;
    config_send(e, cust, (Actor)p);
    // return (Actor)p;
}

void
beh_fifo(Event e)
{
    TRACE(fprintf(stderr, "beh_fifo{event=%p}\n", e));
    expr_value(e);
}
inline void
fifo_new(Event e, Actor cust, size_t n)  // WARNING! n must be a power of 2
{
    size_t b = sizeof(FIFO) + (n * sizeof(Actor));
    // Fifo f = (Fifo)config_create(cfg, b, beh_fifo);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(b), beh_pair);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Fifo f = (Fifo)effect->message;
    f->h = 0;
    f->t = 0;
    f->m = (n - 1);
    config_send(e, cust, (Actor)f);
    // return (Actor)f;
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

inline void
actor_new(Event e, Actor cust, Action beh)  // create an actor with only a behavior procedure
{
    config_create(e, cust, sizeof(ACTOR), beh);
}
inline void
value_new(Event e, Actor cust, Action beh, Any data)  // create a "unserialized" (value) actor
{
    // Value v = (Value)config_create(cfg, sizeof(VALUE), beh);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(VALUE), beh);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Value v = (Value)effect->message;
//    CODE(v) = beh;  <-- already set by config_create()
    DATA(v) = data;
    // return (Actor)v;
    config_send(e, cust, (Actor)v);
}
inline void
serial_with_value(Event e, Actor cust, Actor v)  // create a "serialized" actor with "behavior" value
{
    // Serial s = (Serial)config_create(cfg, sizeof(SERIAL), act_serial);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_create(groundout, NOTHING, sizeof(SERIAL), act_serial);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Serial s = (Serial)effect->message;
    s->beh_now = v;  // an "unserialzed" behavior actor
    // return (Actor)s;
    config_send(e, cust, (Actor)s);
}
inline void
serial_new(Event e, Actor cust, Action beh, Any data)  // create a "serialized" actor
{
    // return serial_with_value(cfg, value_new(cfg, beh, data));
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    value_new(groundout, NOTHING, beh, data);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Actor v = effect->message;
    serial_with_value(e, cust, v);
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
inline void
event_new(Event e, Actor cust, Actor a, Actor m)
{
    // Event ev = config_event_new(e, a, m);
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    config_event_new(groundout, a, m);
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    config_send(e, cust, effect->message);
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
inline static void
root_config_create(Event e, Actor cust, size_t n_bytes, Action beh)
{
    TRACE(fprintf(stderr, "root_config_create: n_bytes=%d\n", (int)n_bytes));
    Actor a = ALLOC(n_bytes);
    if (!a) { config_fail(SPONSOR(e), e_nomem); }
    // return ACTOR_INIT(a, beh);
    if (e == &the_groundout_event) {
        // TRACE(fprintf(stderr, "root_config_create: the_groundout_event\n"));
        e->message = ACTOR_INIT(a, beh);
    } else {
        config_send(e, cust, ACTOR_INIT(a, beh));
    }
}
inline static void
root_config_destroy(Config cfg, Actor victim)
{
    FREE(victim);
}
inline static void
root_config_send(Event e, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "root_config_send: e=%p, actor=%p, msg=%p\n", e, target, msg));
    Event ne = config_event_new(e, target, msg);
    // TRACE(fprintf(stderr, "root_config_send: e->events=%p, ne=%p\n", e->events, ne));
    // deque_give(groundout, NOTHING, e->events, ne);
    // deque_give grounds out by effecting e->events
    // no effects to apply
    Pair q = (Pair)e->events;
    // create new pair
    config_create(&the_groundout_event, NOTHING, sizeof(PAIR), beh_pair);
    Pair p = (Pair)the_groundout_event.message;
    the_groundout_event.message = NOTHING; // reset the_groundout_event
    p->h = (Actor)ne;
    p->t = NIL;
    // deque_give
    if (q->h == NIL) {
        // TRACE(fprintf(stderr, "root_config_send: q->h == NIL\n"));
        q->h = (Actor)p;
    } else {
        Pair t = (Pair)(q->t);
        t->t = (Actor)p;
    }
    q->t = (Actor)p;
    // TRACE(fprintf(stderr, "root_config_send: ne=%p, e->events->h=%p, e->events->t=%p\n", ne, ((Pair)e->events)->h, ((Pair)e->events)->t));
}
EVENT the_groundout_event = { 
    ACTOR_DECL(beh_event), 
    &the_root_config,
    NOTHING,
    NOTHING,
    NOTHING
};
inline static Event
root_config_event_new(Event e, Actor target, Actor msg)
{ 
    // TRACE(fprintf(stderr, "config_event_new: actor=%p, msg=%p\n", target, msg));
    // Event ne = (Event)config_create(SPONSOR(e), sizeof(EVENT), beh_event);
    config_create(&the_groundout_event, NOTHING, sizeof(EVENT), beh_event);
    Event ne = (Event)the_groundout_event.message;
    the_groundout_event.message = NOTHING; // reset the_groundout_event
    ne->sponsor = SPONSOR(e);
    ne->target = target;
    ne->message = msg;
    // TRACE(fprintf(stderr, "config_event_new: ne=%p\n", ne));
    // ne->events = deque_new(SPONSOR(e));
    // deque_new(&the_groundout_event, NOTHING);
    config_create(&the_groundout_event, NOTHING, sizeof(PAIR), beh_deque);
    Pair p = (Pair)the_groundout_event.message;
    the_groundout_event.message = NOTHING; // reset the_groundout_event
    p->h = NIL;
    p->t = NIL;
    ne->events = (Actor)p;
    TRACE(fprintf(stderr, "root_config_event_new: sponsor=%p, e=%p, target=%p, msg=%p, ne=%p\n", SPONSOR(e), e, target, msg, ne));
    return ne;
}
static inline Actor
config_dequeue(Config cfg)
{
    if (((Pair)cfg->events)->h == NOTHING) { halt("config_deque(start): deque head is NOTHING!"); }
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); }  // config actor required
    if (deque_empty_p(cfg, cfg->events) != a_false) {
        TRACE(fprintf(stderr, "config_dequeue: <EMPTY>\n"));
        return NOTHING;
    }
    Actor a = deque_take(cfg, cfg->events);
    TRACE(fprintf(stderr, "config_dequeue: taking a=%p\n", a));
    if (((Pair)cfg->events)->h == NOTHING) { halt("config_deque(done): deque head is NOTHING!"); }
    return a;
}
static inline Actor
effects_events_dequeue(Config cfg, Event e) 
{
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); } // config actor required
    if (deque_empty_p(cfg, e->events) != a_false) {
        // TRACE(fprintf(stderr, "effects_events_dequeue: <EMPTY>\n"));
        return NOTHING;
    }
    Actor a = deque_take(cfg, e->events);
    if (a == NOTHING) { halt("effects_events_dequeue: dequeud NOTHING"); }
    return a;
}
inline void
config_apply_effects(Event se, Config cfg, Event e)
{
    if (((Pair)cfg->events)->h == NOTHING) { halt("config_apply_effects: deque head is NOTHING!"); }
    if (beh_config != BEH(cfg)) { config_fail(cfg, e_inval); } // config actor required
    if (beh_event != BEH(e)) { config_fail(cfg, e_inval); } // event actor required
    Actor e_fx;
    while ((e_fx = effects_events_dequeue(cfg, e)) != NOTHING) {
        TRACE(fprintf(stderr, "config_apply_effects: event=%p\n", e_fx));
        // config_enqueue(cfg, e_fx);      
        Event groundout = config_event_new(se, NOTHING, NOTHING);
        TRACE(fprintf(stderr, "config_apply_effects: groundout=%p\n", groundout));
        deque_give(groundout, NOTHING, cfg->events, e_fx);
        // deque_give grounds out by effecting cfg->events
        // no effects to apply
    }
}
Actor
config_dispatch(Config cfg)
{
    Actor a = config_dequeue(cfg);
    if (beh_event == BEH(a)) {
        Event e = (Event)a;
        TRACE(fprintf(stderr, "config_dispatch: event=%p, actor=%p, msg=%p\n", e, SELF(e), MSG(e)));
        (CODE(SELF(e)))(e);  // INVOKE ACTION PROCEDURE
        Event groundout = config_event_new(&the_groundout_event, NOTHING, NOTHING);
        the_groundout_event.message = NOTHING; // reset the_groundout_event
        groundout->sponsor = cfg; // this configuration is the sponsor
        config_apply_effects(groundout, cfg, e);
    }
    if (((Pair)cfg->events)->h == NOTHING) { halt("config_dispatch: deque head is NOTHING!"); }
    return a;   
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
inline static void
quota_config_create(Event e, Actor cust, size_t n_bytes, Action beh)
{
    struct quota_config * self = (struct quota_config *)SELF(e);
    n_bytes = (n_bytes + self->mask) & ~self->mask;  // round up to next quanta
    TRACE(fprintf(stderr, "quota_config_create: n_bytes=%d\n", (int)n_bytes));
    if (self->n_bytes < n_bytes) { config_fail(SPONSOR(e), e_nomem); }
    Actor a = (Actor)(self->free);
    self->free += n_bytes;
    self->n_bytes -= n_bytes;
    // return ACTOR_INIT(a, beh);
    a = ACTOR_INIT(a, beh);
    TRACE(fprintf(stderr, "quota_config_create: created a=%p\n", a));
    if (e == &the_groundout_event) {
        TRACE(fprintf(stderr, "quota_config_create: groundout handling\n"));
        e->message = a;
    } else {
        TRACE(fprintf(stderr, "quota_config_create: message send handling, e=%p\n", e));
        config_send(e, cust, a);
    }
}
inline static void
quota_config_destroy(Config cfg, Actor victim)
{
    // quota is not re-usable in this implementation
}
inline static void
quota_config_send(Event e, Actor target, Actor msg)
{
    TRACE(fprintf(stderr, "quota_config_send: sponsor=%p, e=%p, actor=%p, msg=%p\n", SPONSOR(e), e, target, msg));
    Event ne = config_event_new(e, NOTHING, NOTHING);
    // deque_give(groundout, NOTHING, e->events, (Actor)config_event_new(e, target, msg));
    // // deque_give grounds out by effecting e->events
    // // no effects to apply
    Pair q = (Pair)e->events;
    // create new pair
    the_groundout_event.target = (Actor)SPONSOR(e);
    the_groundout_event.sponsor = SPONSOR(e);
    config_create(&the_groundout_event, NOTHING, sizeof(PAIR), beh_pair);
    Pair p = (Pair)the_groundout_event.message;
    the_groundout_event.message = NOTHING; // reset the_groundout_event
    the_groundout_event.target = NOTHING; // reset the_groundout_event
    the_groundout_event.sponsor = &the_root_config; // reset the_groundout_event
    p->h = (Actor)ne;
    p->t = NIL;
    // deque_give
    if (q->h == NIL) {
        q->h = (Actor)p;
    } else {
        Pair t = (Pair)(q->t);
        t->t = (Actor)p;
    }
    q->t = (Actor)p;
    if (q->h == NOTHING) { halt("quota_config_send: head is NOTHING!"); }
}
inline static Event 
quota_config_event_new(Event e, Actor target, Actor msg)
{
    // TRACE(fprintf(stderr, "quota_config_event_new: actor=%p, msg=%p\n", target, msg));
    // Event ne = (Event)config_create(SPONSOR(e), sizeof(EVENT), beh_event);
    the_groundout_event.target = (Actor)SPONSOR(e);
    the_groundout_event.sponsor = SPONSOR(e);
    config_create(&the_groundout_event, NOTHING, sizeof(EVENT), beh_event);
    Event ne = (Event)the_groundout_event.message;
    the_groundout_event.message = NOTHING; // reset the_groundout_event
    the_groundout_event.target = NOTHING; // reset the_groundout_event
    the_groundout_event.sponsor = &the_root_config; // reset the_groundout_event
    ne->sponsor = SPONSOR(e);
    ne->target = target;
    ne->message = msg;
    // ne->events = deque_new(SPONSOR(e));
    // Event groundout = config_event_new(e, NOTHING, NOTHING);
    // deque_new(groundout, NOTHING);
    // Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    // ne->events = effect->message;  
    the_groundout_event.target = (Actor)SPONSOR(e); 
    the_groundout_event.sponsor = SPONSOR(e);
    config_create(&the_groundout_event, NOTHING, sizeof(PAIR), beh_deque);
    Pair p = (Pair)the_groundout_event.message;
    the_groundout_event.message = NOTHING; // reset the_groundout_event
    the_groundout_event.target = NOTHING; // reset the_groundout_event
    the_groundout_event.sponsor = &the_root_config; // reset the_groundout_event
    p->h = NIL;
    p->t = NIL;
    ne->events = (Actor)p;     
    if (p->h == NOTHING) { halt("quota_config_new: deque head is NOTHING!"); }
    if (((Pair)ne->events)->h == NOTHING) { halt("quota_config_new: deque head is NOTHING!"); }
    TRACE(fprintf(stderr, "quota_config_event_new: sponsor=%p, e=%p, target=%p, msg=%p, ne=%p\n", SPONSOR(e), e, target, msg, ne));
    return ne;
}
inline void
quota_config_new(Event e, Actor cust, size_t n_bytes)
{
    // TRACE(fprintf(stderr, "quota_config_new: sponsor=%p, e=%p, n_bytes=%d\n", SPONSOR(e), e, (int)n_bytes));
    // Config cfg = (Config)config_create(sponsor, sizeof(struct quota_config) + n_bytes, beh_config); 
    Event groundout = config_event_new(e, NOTHING, NOTHING);
    // TRACE(fprintf(stderr, "quota_config_new: groundout=%p\n", groundout));
    config_create(groundout, NOTHING, sizeof(struct quota_config) + n_bytes, beh_config);
    // TRACE(fprintf(stderr, "quota_config_new: groundout->events=%p, go->events->h=%p\n", groundout->events, ((Pair)groundout->events)->h));
    Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    Config cfg = (Config)effect->message;
    // TRACE(fprintf(stderr, "quota_config_new: sponsor=%p cfg=%p\n", SPONSOR(e), cfg));
    struct quota_config * self = (struct quota_config *)cfg;
    self->n_bytes = n_bytes;  // available storage
    self->mask = (1 << 4) - 1;  // allocation/alignment granularity mask (2^n - 1)
    self->free = &self->memory[0];  // pointer to start of next allocation
    // TRACE(fprintf(stderr, "quota_config_new: n_bytes=%d\n", (int)n_bytes));
    cfg->fail = quota_config_fail;  // error reporting procedure
    cfg->create = quota_config_create;  // actor creation procedure
    cfg->destroy = quota_config_destroy;  // reclaim actor resources
    cfg->send = quota_config_send;  // message send procedure
    cfg->event_new = quota_config_event_new; // event creation procedure
    // cfg->events = deque_new(cfg);
    groundout = config_event_new(e, NOTHING, NOTHING);
    deque_new(groundout, NOTHING);
    effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
    cfg->events = effect->message;
    if (cfg->events == NOTHING) { halt("cfg->events deque is NOTHING!"); }
    config_send(e, cust, (Actor)cfg);
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
            // Actor ok = value_new(SPONSOR(e), beh_pair_0, req_match_new(SPONSOR(e), r->ok, r->fail, q->t, p->t));
            // value_new(groundout, NOTHING, beh_pair_0, req_match_new(SPONSOR(e), r->ok, r->fail, q->t, p->t));
            Event groundout = config_event_new(e, NOTHING, NOTHING);
            req_match_new(groundout, NOTHING, r->ok, r->fail, q->t, p->t);
            Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
            Actor rm_new_1 = effect->message;
            groundout = config_event_new(e, NOTHING, NOTHING);
            value_new(groundout, NOTHING, beh_pair_0, rm_new_1);
            effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
            Actor ok = effect->message;
            // config_send(e, p->h, req_match_new(SPONSOR(e), ok, r->fail, q->h, rm->env));
            groundout = config_event_new(e, NOTHING, NOTHING);
            req_match_new(groundout, NOTHING, ok, r->fail, q->h, rm->env);
            effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
            Actor rm_new_2 = effect->message;
            config_send(e, p->h, rm_new_2);
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
        // config_send(e, r->ok, list_push(SPONSOR(e), SELF(e), rw->value));
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        pair_new(groundout, NOTHING, rw->value, SELF(e));
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor new_list = effect->message;
        config_send(e, r->ok, new_list);
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
        // config_send(e, r->ok, PR(deque_take(SPONSOR(e), SELF(e)), SELF(e)));
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        pair_new(groundout, NOTHING, deque_take(SPONSOR(e), SELF(e)), SELF(e));
        Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
        Actor pair = effect->message;
        config_send(e, r->ok, pair);
    } else if (val_req_write == BEH(r->req)) {  // (#write, value)
        ReqWrite rw = (ReqWrite)r->req;
        TRACE(fprintf(stderr, "beh_deque: (#write, %p)\n", rw->value));
        // deque_give(SPONSOR(e), SELF(e), rw->value);
        Event groundout = config_event_new(e, NOTHING, NOTHING);
        deque_give(groundout, NOTHING, SELF(e), rw->value);
        // deque_give grounds out by effecting SELF(e)
        // no effects to apply
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
            // config_send(e, expr, req_eval_new(SPONSOR(e), r->ok, r->fail, rc->env));
            Event groundout = config_event_new(e, NOTHING, NOTHING);
            req_eval_new(groundout, NOTHING, r->ok, r->fail, rc->env);
            Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
            ReqEval re = (ReqEval)effect->message;
            config_send(e, expr, (Actor)re);
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
            // config_send(e, expr, req_eval_new(SPONSOR(e), r->ok, r->fail, rc->env));
            Event groundout = config_event_new(e, NOTHING, NOTHING);
            req_eval_new(groundout, NOTHING, r->ok, r->fail, rc->env);
            Event effect = (Event)((Pair)((Pair)((Pair)groundout->events)->h)->h);
            ReqEval re = (ReqEval)effect->message;
            config_send(e, expr, (Actor)re);
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
