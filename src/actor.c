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
    beh_halt(e);
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
    Actor a = NEW(ACTOR);
    CODE(a) = beh;
    DATA(a) = data;
    return a;
}
inline Actor
actor_new(Action beh, Any data)  // create a "serialized" actor
{
    return value_new(act_serial, value_new(beh, data));
}
inline void
actor_become(Actor a, Actor v)
{
    if (!SERIAL(a)) { halt("actor_become: serialized actor required"); }
    DATA(a) = v;  // an "unserialzed" behavior actor is the data
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
    TRACE(fprintf(stderr, "config_send: actor=%p, msg=%p\n", target, msg));
    config_enqueue(cfg, event_new(cfg, target, msg));
}
int
config_dispatch(Config cfg)
{
    if (deque_empty_p(cfg->event_q)) {
        TRACE(fprintf(stderr, "config_dispatch: <EMPTY>\n"));
        return 0;
    }
    Event e = deque_take(cfg->event_q);
    TRACE(fprintf(stderr, "config_dispatch: event=%p, actor=%p, msg=%p\n", e, SELF(e), MSG(e)));
    (CODE(SELF(e)))(e);  // INVOKE ACTOR METHOD
    return 1;
}

void
act_serial(Event e)  // "serialized" actor behavior
{
    TRACE(fprintf(stderr, "act_serial{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    (CODE(DATA(SELF(e))))(e);  // INVOKE BEHAVIOR METHOD
}

static void
beh_ignore(Event e)
{
    TRACE(fprintf(stderr, "beh_ignore{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}

ACTOR the_halt_actor = { { beh_halt }, &the_halt_actor };
ACTOR the_ignore_actor = { { beh_ignore }, NOTHING };
