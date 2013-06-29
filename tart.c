/*

tart.c -- Tiny Actor Run-Time

To compile:
    cc -o tart tart.c

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

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#undef inline /*inline*/
#undef ALWAYS_USE_EFFECTS /**/

#define TRACE(x)    x   /* enable/disable trace statements */
#define DEBUG(x)        /* enable/disable debug statements */

#define ALLOC(S)    (calloc((S), 1))
#define NEW(T)      ((T *)calloc(sizeof(T), 1))
#define FREE(p)     ((p) = (free(p), NULL))

typedef void *Any;

typedef struct pair PAIR, *Pair;
typedef struct config CONFIG, *Config;
typedef struct behavior BEHAVIOR, *Behavior;
typedef struct actor ACTOR, *Actor;
typedef struct event EVENT, *Event;
typedef struct effect EFFECT, *Effect;

typedef void (*Action)(Event e);

#define NIL ((Pair)0)

struct pair {
    Any             h;
    Any             t;
};

inline Pair
pair_new(Any h, Any t)
{
    Pair q = NEW(PAIR);
    q->h = h;
    q->t = t;
    return q;
}

inline int
list_empty_p(Pair list)
{
    return (list == NIL);
}

inline Pair
list_pop(Pair list)
{
    return list;
}

inline Pair
list_push(Pair list, Any item)
{
    return pair_new(item, list);
}

inline Pair
queue_new()
{
    return pair_new(NIL, NIL);
}

inline int
queue_empty_p(Pair q)
{
    return (q->h == NIL);
}

inline void
queue_give(Pair q, Any item)
{
    Pair p = pair_new(item, NIL);
    if (queue_empty_p(q)) {
        q->h = p;
    } else {
        Pair t = q->t;
        t->t = p;
    }
    q->t = p;
}

inline Any
queue_take(Pair q)
{
    // if (queue_empty_p(q)) ERROR("can't take from empty queue");
    Pair p = q->h;
    Any item = p->h;
    q->h = p->t;
    p = FREE(p);
    return item;
}

struct config {
    Pair            event_q;    // queue of messages in-transit
    Pair            actors;     // list of actors created
};

inline Config
config_new()
{
    Config cfg = NEW(CONFIG);
    cfg->event_q = queue_new();
    cfg->actors = NIL;
    return cfg;
}

inline void
config_enqueue(Config cfg, Event e)
{
    queue_give(cfg->event_q, e);
}

inline void
config_enlist(Config cfg, Actor a)
{
    cfg->actors = list_push(cfg->actors, a);
}

struct behavior {
    Action          action;     // code
    Any             context;    // data
};

inline Behavior
behavior_new(Action act, Any data)
{
    Behavior beh = NEW(BEHAVIOR);
    beh->action = act;
    beh->context = data;
    return beh;
}

struct actor {
    Behavior        behavior;   // current behavior
};

inline Actor
actor_new(Behavior beh)
{
    Actor a = NEW(ACTOR);
    a->behavior = beh;
    return a;
}

inline void
actor_become(Actor a, Behavior beh)
{
    a->behavior = beh;
}

struct event {
    Config          sponsor;    // sponsor configuration
    Actor           actor;      // target actor
    Any             message;    // message to deliver
};

inline Event
event_new(Config cfg, Actor a, Any msg)
{
    Event e = NEW(EVENT);
    e->sponsor = cfg;
    e->actor = a;
    e->message = msg;
    return e;
}

inline void
config_send(Config cfg, Actor target, Any msg)
{
    config_enqueue(cfg, event_new(cfg, target, msg));
}

inline void
config_create(Config cfg, Behavior beh)
{
    config_enlist(cfg, actor_new(beh));
}

void
act_sink(Event e)
{
    TRACE(fprintf(stderr, "act_sink{self=%p, msg=%p}\n", e->actor, e->message));
}

BEHAVIOR sink_behavior = { act_sink, NIL };
ACTOR sink_actor = { &sink_behavior };

struct effect {
    Config          sponsor;    // sponsor configuration
    Actor           self;       // currently active (meta-)actor
    Pair            actors;     // list of actors created
    Pair            events;     // list of messages sent
    Behavior        behavior;   // replacement behavior
};

inline Effect
effect_new(Config cfg, Actor a)
{
    Effect fx = NEW(EFFECT);
    fx->sponsor = cfg;
    fx->self = a;
    fx->actors = NIL;
    fx->events = NIL;
    fx->behavior = a->behavior;
    return fx;
}

inline void
effect_send(Effect fx, Actor target, Any msg)
{
    fx->events = list_push(fx->events, event_new(fx->sponsor, target, msg));
}

inline void
effect_create(Effect fx, Behavior beh)
{
    fx->actors = list_push(fx->actors, actor_new(beh));
}

inline void
effect_become(Effect fx, Behavior beh)
{
    fx->behavior = beh;
}

inline void
effect_commit(Effect fx)
{
    Pair p;

    // update actor behavior
    fx->self->behavior = fx->behavior;
    // add new actors to configuration
    for (p = fx->actors; !list_empty_p(p); p = p->t) {
        config_enlist(fx->sponsor, p->h);
    }
    // add new events to dispatch queue
    for (p = fx->events; !list_empty_p(p); p = p->t) {
        config_enqueue(fx->sponsor, p->h);
    }
}

/**
begin_beh(cust) = \_.[
    SEND Effect.new() TO cust
]
**/
void
act_begin(Event e)
{
    TRACE(fprintf(stderr, "act_begin{self=%p, msg=%p}\n", e->actor, e->message));
    Actor cust = e->actor->behavior->context;  // cust
    // initialize effects
    Effect fx = effect_new(e->sponsor, e->actor);
    // trigger continuation
    config_send(e->sponsor, cust, fx);
}

/**
send_beh(cust, target, message) = \fx.[
    fx.send(target, message)
    SEND fx TO cust
]
**/
void
act_send(Event e)
{
    TRACE(fprintf(stderr, "act_send{self=%p, msg=%p}\n", e->actor, e->message));
    Pair p = e->actor->behavior->context;  // (cust, target, message)
    Actor cust = p->h;
    p = p->t;
    Actor target = p->h;
    Any message = p->t;
    TRACE(fprintf(stderr, "act_send: to=%p, msg=%p\n", target, message));
    // store new event in effects
    effect_send(e->message, target, message);
    // trigger continuation
    config_send(e->sponsor, cust, e->message);
}

/**
create_beh(cust, beh) = \fx.[
    fx.create(beh)
    SEND fx TO cust
]
**/
void
act_create(Event e)
{
    TRACE(fprintf(stderr, "act_create{self=%p, msg=%p}\n", e->actor, e->message));
    Pair p = e->actor->behavior->context;  // (cust, behavior)
    Actor cust = p->h;
    Behavior beh = p->t;
    // store new actor in effects
    effect_create(e->message, beh);
    // trigger continuation
    config_send(e->sponsor, cust, e->message);
}

/**
become_beh(cust, beh) = \fx.[
    fx.become(beh)
    SEND fx TO cust
]
**/
void
act_become(Event e)
{
    TRACE(fprintf(stderr, "act_become{self=%p, msg=%p}\n", e->actor, e->message));
    Pair p = e->actor->behavior->context;  // (cust, behavior)
    Actor cust = p->h;
    Behavior beh = p->t;
    // store new behavior in effects
    effect_become(e->message, beh);
    // trigger continuation
    config_send(e->sponsor, cust, e->message);
}

/**
commit_beh(cust, beh) = \fx.[
    fx.commit()
]
**/
void
act_commit(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_commit{self=%p, msg=%p}\n", e->actor, e->message));
    effect_commit(e->message);
}

BEHAVIOR commit_behavior = { act_commit, NIL };
ACTOR commit_actor = { &commit_behavior };

/**
forward_beh(a) = \m.[ SEND m TO a ]
forward_beh(a) = \m.SEND () TO \_.[ SEND m TO a ]
**/
void
act_forward(Event e)
{
    TRACE(fprintf(stderr, "act_forward{self=%p, msg=%p}\n", e->actor, e->message));
    Actor a = e->actor->behavior->context;  // target
    Any m = e->message;  // message
#ifdef ALWAYS_USE_EFFECTS
    Pair args = pair_new(&commit_actor, pair_new(a, m));  // (cust, target, message)
    Actor a_send = actor_new(behavior_new(act_send, args));
    Actor a_begin = actor_new(behavior_new(act_begin, a_send));
    TRACE(fprintf(stderr, "act_forward: delegate=%p\n", a_begin));
    // invoke delegate
    config_send(e->sponsor, a_begin, NIL);  // NOTE: act_begin() ignores e->message
#else /*ALWAYS_USE_EFFECTS*/
    config_send(e->sponsor, a, m);
#endif /*ALWAYS_USE_EFFECTS*/
}

/**
oneshot_beh(a) = \m.[
    SEND m TO a
    BECOME \_.[]
]
**/
void
act_oneshot(Event e)
{
    TRACE(fprintf(stderr, "act_oneshot{self=%p, msg=%p}\n", e->actor, e->message));
    Actor a = e->actor->behavior->context;  // target
    Any m = e->message;  // message
#ifdef ALWAYS_USE_EFFECTS
    Pair args = pair_new(&commit_actor, pair_new(a, m));  // (cust, target, message)
    Actor a_send = actor_new(behavior_new(act_send, args));
    Actor a_begin = actor_new(behavior_new(act_begin, a_send));
    TRACE(fprintf(stderr, "act_oneshot: delegate=%p\n", a_begin));
    // become sink
    e->actor->behavior = &sink_behavior;
    // invoke delegate
    config_send(e->sponsor, a_begin, NIL);  // NOTE: act_begin() ignores e->message
#else /*ALWAYS_USE_EFFECTS*/
    config_send(e->sponsor, a, m);
    actor_become(e->actor, &sink_behavior);
#endif /*ALWAYS_USE_EFFECTS*/
}

int
config_dispatch(Config cfg)
{
    if (queue_empty_p(cfg->event_q)) {
        TRACE(fprintf(stderr, "config_dispatch: --EMPTY--\n"));
        return 0;
    }
    Event e = queue_take(cfg->event_q);
    TRACE(fprintf(stderr, "config_dispatch: actor=%p, event=%p\n", e->actor, e));
    (e->actor->behavior->action)(e);  // INVOKE ACTOR BEHAVIOR
    return 1;
}

/*
 *  Unit tests
 */
void
run_tests()
{
    TRACE(fprintf(stderr, "NIL = %p\n", NIL));

/**
CREATE sink WITH \_.[]
**/
    Actor a_commit = &commit_actor;  // WAS: actor_new(behavior_new(act_commit, NIL));
    TRACE(fprintf(stderr, "a_commit = %p\n", a_commit));
    Actor a_sink = &sink_actor;  // WAS: actor_new(behavior_new(act_begin, a_commit));
    TRACE(fprintf(stderr, "a_sink = %p\n", a_sink));
/**
CREATE doit WITH \_.[ SEND [] TO sink ]
**/
/*
    Pair args = pair_new(a_commit, pair_new(a_sink, NIL));  // (cust, target, message)
    Actor a = actor_new(behavior_new(act_send, args));
    Actor a_doit = actor_new(behavior_new(act_begin, a));
*/
//    Actor a_doit = actor_new(behavior_new(act_forward, a_sink));
    Actor a_doit = actor_new(behavior_new(act_oneshot, a_sink));
    TRACE(fprintf(stderr, "a_doit = %p\n", a_doit));
    
    Config cfg = config_new();
    config_send(cfg, a_doit, NIL);
    config_send(cfg, a_doit, a_doit);
    while (config_dispatch(cfg))
        ;
}

/*
 *  Main entry-point
 */
int
main()
{
    run_tests();
    return 0;
}
