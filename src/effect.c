/*

effect.c -- Tiny Actor Run-Time

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

#include "effect.h"

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
    actor_become(fx->self, fx->behavior);
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
LET busy_beh = \m.[
    SEND m TO SELF
]
**/
void
act_busy(Event e)
{
    TRACE(fprintf(stderr, "act_busy{self=%p, msg=%p}\n", e->actor, e->message));
    // re-queue event
    config_enqueue(e->sponsor, e);
}

BEHAVIOR busy_behavior = { { act_busy }, NIL };

/**
LET begin_beh(cust) = \_.[
    SEND Effect.new() TO cust
    BECOME busy_beh
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
    // become busy
    actor_become(e->actor, &busy_behavior);
}

/**
LET send_beh(cust, target, message) = \fx.[
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
LET create_beh(cust, beh) = \fx.[
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
LET become_beh(cust, beh) = \fx.[
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
LET commit_beh(cust, beh) = \fx.[
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

BEHAVIOR commit_behavior = { { act_commit }, NIL };
ACTOR commit_actor = { { act_serial }, &commit_behavior };
