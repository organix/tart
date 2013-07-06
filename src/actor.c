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

inline Behavior
behavior_new(Action act, Any data)
{
    Behavior beh = NEW(BEHAVIOR);
    beh->action = act;
    beh->context = data;
    return beh;
}

inline Event
event_new(Config cfg, Actor a, Any msg)
{
    Event e = NEW(EVENT);
    e->sponsor = cfg;
    e->actor = a;
    e->message = msg;
    return e;
}

inline Config
config_new()
{
    Config cfg = NEW(CONFIG);
    cfg->event_q = deque_new();
    cfg->actors = NIL;
    return cfg;
}

inline void
config_enqueue(Config cfg, Event e)
{
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
    config_enqueue(cfg, event_new(cfg, target, msg));
}

inline void
config_create(Config cfg, Behavior beh)
{
    config_enlist(cfg, actor_new(beh));
}

int
config_dispatch(Config cfg)
{
    if (deque_empty_p(cfg->event_q)) {
        TRACE(fprintf(stderr, "config_dispatch: <EMPTY>\n"));
        return 0;
    }
    Event e = deque_take(cfg->event_q);
    TRACE(fprintf(stderr, "config_dispatch: actor=%p, event=%p\n", e->actor, e));
    (e->actor->behavior->action)(e);  // INVOKE ACTOR BEHAVIOR
    return 1;
}

static void
act_sink(Event e)
{
    TRACE(fprintf(stderr, "act_sink{self=%p, msg=%p}\n", e->actor, e->message));
}

BEHAVIOR sink_behavior = { act_sink, NIL };
ACTOR sink_actor = { &sink_behavior };
