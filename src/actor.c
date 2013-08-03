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

PAIR the_nil_pair_actor = {
    { beh_pair }, 
    NIL, 
    NIL 
};

void
beh_pair(Event e)
{
    TRACE(fprintf(stderr, "beh_pair{event=%p}\n", e));
    expr_value(e);  // DON'T PANIC!
}

inline Actor
pair_new(Any h, Any t)
{
    Pair p = NEW(PAIR);
    BEH(p) = beh_pair;
    p->h = h;
    p->t = t;
    return (Actor)p;
}

inline Actor
list_new()
{
    return a_empty_list;
}
inline int
list_empty_p(Actor list)
{
    return (list == a_empty_list);
}
inline Pair
list_pop(Actor list)  // returns: (first, rest)
{
    if (beh_pair != BEH(list)) { halt("list_pop: pair required"); }
    return (Pair)list;
}
inline Actor
list_push(Actor list, Any item)
{
    if (beh_pair != BEH(list)) { halt("list_push: pair required"); }
    return PR(item, list);
}

inline Actor
deque_new()
{
    return PR(NIL, NIL);
}
inline int
deque_empty_p(Actor queue)
{
    if (beh_pair != BEH(queue)) { halt("deque_empty_p: pair required"); }
    Pair q = (Pair)queue;
    return (q->h == NIL);
}
inline void
deque_give(Actor queue, Any item)
{
    if (beh_pair != BEH(queue)) { halt("deque_give: pair required"); }
    Pair q = (Pair)queue;
    Actor p = PR(item, NIL);
    if (q->h == NIL) {
        q->h = p;
    } else {
        Pair t = q->t;
        t->t = p;
    }
    q->t = p;
}
inline Any
deque_take(Actor queue)
{
    if (deque_empty_p(queue)) { halt("deque_take from empty!"); }
//    if (beh_pair != BEH(queue)) { halt("deque_take: pair required"); }
    Pair q = (Pair)queue;
    Pair p = q->h;
    Any item = p->h;
    q->h = p->t;
    p = FREE(p);
    return item;
}
inline void
deque_return(Actor queue, Any item)
{
    if (beh_pair != BEH(queue)) { halt("deque_return: pair required"); }
    Pair q = (Pair)queue;
    Actor p = PR(item, q->h);
    if (q->h == NIL) {
        q->t = p;
    }
    q->h = p;
}
inline Any
deque_lookup(Actor queue, int index)
{
    if (beh_pair != BEH(queue)) { halt("deque_lookup: pair required"); }
    Pair q = (Pair)queue;
    Pair p = q->h;
    while (p != NIL) {
        if (beh_pair != BEH(p)) { halt("deque_lookup: non-pair in chain"); }
        if (index <= 0) {
            return p->h;
        }
        --index;
        p = p->t;
    }
    return NULL;  // not found
}
inline void
deque_bind(Actor queue, int index, Any item)
{
    if (beh_pair != BEH(queue)) { halt("deque_bind: pair required"); }
    Pair q = (Pair)queue;
    Pair p = q->h;
    while (p != NIL) {
        if (beh_pair != BEH(p)) { halt("deque_bind: non-pair in chain"); }
        if (index <= 0) {
            p->h = item;
        }
        --index;
        p = p->t;
    }
    // not found
}

ACTOR the_empty_dict_actor = { expr_env_empty };
inline Actor
dict_new()
{
    return a_empty_dict;
}
inline int
dict_empty_p(Actor dict)
{
    return (dict == a_empty_dict);
}
inline Any
dict_lookup(Actor dict, Any key)
{
    while (!dict_empty_p(dict)) {
        if (expr_env != BEH(dict)) { halt("dict_lookup: non-dict in chain"); }
        Pair p = (Pair)dict;
        Pair q = p->h;
        if (q->h == key) {
            return q->t;  // value
        }
        dict = p->t;  // next
    }
    return NULL;  // NOT FOUND
}
inline Actor
dict_bind(Actor dict, Any key, Any value)
{
    Actor a = PR(PR(key, value), dict);
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
    expr_value(e);
}
inline Config
config_new()
{
    Config cfg = NEW(CONFIG);
    BEH(cfg) = beh_config;
    cfg->events = deque_new();
    cfg->actors = list_new();
    return cfg;
}
inline void
config_enqueue(Config cfg, Event e)
{
    if (beh_event != BEH(e)) { halt("config_enqueue: event actor required"); }
    deque_give(cfg->events, e);
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
    if (deque_empty_p(cfg->events)) {
        TRACE(fprintf(stderr, "config_dispatch: <EMPTY>\n"));
        return 0;
    }
    Event e = deque_take(cfg->events);
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
ACTOR the_ignore_actor = { beh_ignore };

void
beh_halt(Event e)
{
    TRACE(fprintf(stderr, "beh_halt{event=%p}\n", e));
    halt("HALT!");
}
VALUE the_halt_actor = { { beh_halt }, NOTHING };  // qualifies as both VALUE and SERIAL
