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

//#define inline      /*inline*/

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

struct effect {
    Actor           self;       // currently active (meta-)actor
    Pair            actors;     // list of actors created
    Pair            events;     // list of messages sent
    Behavior        behavior;   // replacement behavior
};

inline Effect
effect_new(Actor a)
{
    Effect fx = NEW(EFFECT);
    fx->self = a;
    fx->actors = NIL;
    fx->events = NIL;
    fx->behavior = a->behavior;
    return fx;
}

void
act_begin(Event e)
{
    TRACE(fprintf(stderr, "act_begin{self=%p, msg=%p}\n", e->actor, e->message));
    Actor cust = e->actor->behavior->context;
    // initialize effects
    Effect fx = effect_new(e->actor);
    // trigger continuation
    Event cont = event_new(e->sponsor, cust, fx);
    queue_give(e->sponsor->event_q, cont);
}

void
act_send(Event e)
{
    TRACE(fprintf(stderr, "act_send{self=%p, msg=%p}\n", e->actor, e->message));
    Pair p = e->actor->behavior->context;  // (cust, target, message)
    Actor cust = p->h;
    p = p->t;
    Actor target = p->h;
    Any message = p->t;
    // store new event in effects
    Effect fx = e->message;
    Event evt = event_new(e->sponsor, target, message);
    fx->events = list_push(fx->events, evt);
    // trigger continuation
    Event cont = event_new(e->sponsor, cust, e->message);
    queue_give(e->sponsor->event_q, cont);
}

void
act_create(Event e)
{
    TRACE(fprintf(stderr, "act_create{self=%p, msg=%p}\n", e->actor, e->message));
    Pair p = e->actor->behavior->context;  // (cust, behavior)
    Actor cust = p->h;
    Behavior beh = p->t;
    // store new actor in effects
    Effect fx = e->message;
    Actor a = actor_new(beh);
    fx->actors = list_push(fx->actors, a);
    // trigger continuation
    Event cont = event_new(e->sponsor, cust, e->message);
    queue_give(e->sponsor->event_q, cont);
}

void
act_become(Event e)
{
    TRACE(fprintf(stderr, "act_become{self=%p, msg=%p}\n", e->actor, e->message));
    Pair p = e->actor->behavior->context;  // (cust, behavior)
    Actor cust = p->h;
    Behavior beh = p->t;
    // store new behavior in effects
    Effect fx = e->message;
    fx->behavior = beh;
    // trigger continuation
    Event cont = event_new(e->sponsor, cust, e->message);
    queue_give(e->sponsor->event_q, cont);
}

void
act_commit(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_commit{self=%p, msg=%p}\n", e->actor, e->message));
    Effect fx = e->message;
    // update actor behavior
    fx->self->behavior = fx->behavior;
    // add new actors to configuration
    for (p = fx->actors; !list_empty_p(p); p = p->t) {
        e->sponsor->actors = list_push(e->sponsor->actors, p->h);
    }
    // add new events to dispatch queue
    for (p = fx->events; !list_empty_p(p); p = p->t) {
        queue_give(e->sponsor->event_q, p->h);
    }
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

/*
CREATE sink WITH \_.[]
CREATE doit WITH \_.[ SEND [] TO sink ]
fwd_beh(target) = \msg.[ SEND msg TO target ]
oneshot_beh(target) = \msg.[
    SEND msg TO target
    BECOME sink_beh
]
*/
    Actor a_commit = actor_new(behavior_new(act_commit, NIL));
    TRACE(fprintf(stderr, "a_commit = %p\n", a_commit));
    Actor a_sink = actor_new(behavior_new(act_begin, a_commit));
    TRACE(fprintf(stderr, "a_sink = %p\n", a_sink));

    // (cust, target, message)
    Pair msg = pair_new(a_commit, pair_new(a_sink, NIL));
    Actor a = actor_new(behavior_new(act_send, msg));

    Actor a_doit = actor_new(behavior_new(act_begin, a));
    TRACE(fprintf(stderr, "a_doit = %p\n", a_doit));
    
    Config cfg = config_new();
    Event e = event_new(cfg, a_doit, NIL);
    queue_give(cfg->event_q, e);
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
