/*

actor.h -- Tiny Actor Run-Time

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
#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "tart.h"

/**
Actor  [*|...]
        |
        V
       BEH

Value  [*|*]
        | +--> DATA
        V
       CODE

Serial [*|*|*]---+
        | |      V
        | +---> [*|*]
        V        | +--> STATE
   act_serial    V
                STRATEGY

Pair   [*|*|*]
        | | +--> tail
        | +--> head
        V
     beh_pair
**/

typedef struct actor ACTOR, *Actor;
typedef struct pair PAIR, *Pair;
typedef struct value VALUE, *Value;
typedef struct serial SERIAL, *Serial;
typedef struct event EVENT, *Event;
typedef struct config CONFIG, *Config;

typedef void (*Action)(Event e);

#define NIL ((Actor)(&the_nil_pair_actor))
#define NOTHING ((Actor)(&the_halt_actor))

#define BEH(a)      (((Actor)(a))->beh)
#define SPONSOR(e)  (((Event)(e))->sponsor)
#define SELF(e)     (((Event)(e))->target)
#define MSG(e)      (((Event)(e))->message)
#define CODE(v)     BEH(v)
#define DATA(v)     (((Value)(v))->data)
#define STRATEGY(s) CODE(((Serial)(s))->beh_now)
#define STATE(s)    DATA(((Serial)(s))->beh_now)
#define PR(h,t)     (pair_new(SPONSOR(e),(h),(t)))

#define a_empty_list ((Actor)(&the_nil_pair_actor))
#define a_empty_dict ((Actor)(&the_empty_dict_actor))
#define a_root_config ((Actor)(&the_root_config))
#define a_true ((Actor)(&the_true_actor))
#define a_false ((Actor)(&the_false_actor))
#define a_ignore ((Actor)(&the_ignore_actor))
#define a_halt ((Actor)(&the_halt_actor))

struct actor {
    Action      beh;
    Actor       (*match)(Config cfg, Actor pattern, Actor value);  // polymorphic value matching (pattern == self)
};

struct pair {
    ACTOR       _act;
    Actor       h;
    Actor       t;
};

struct value {
    ACTOR       _act;
    Any         data;
};

struct serial {
    ACTOR       _act;
    Actor       beh_now;      // current "behavior" actor
    Actor       beh_next;      // "behavior" for next event
};

struct event {
    ACTOR       _act;
    Config      sponsor;    // sponsor configuration
    Actor       target;     // target actor
    Actor       message;    // message to deliver
};

struct config {
    ACTOR       _act;
    void        (*fail)(Config cfg, Actor reason);  // error reporting procedure
    Actor       (*create)(Config cfg, size_t n_bytes, Action beh);  // actor creation procedure
    void        (*destroy)(Config cfg, Actor victim);  // reclaim actor resources
    void        (*send)(Config cfg, Actor target, Actor msg);  // event creation procedure
    Actor       events;     // queue of messages in-transit
};

extern Actor    actor_match_method(Config cfg, Actor this, Actor that);
#define         actor_match(cfg, ptrn, value)   (((ptrn)->match)((cfg), (ptrn), (value)))

extern Actor    pair_new(Config cfg, Actor h, Actor t);

#define         list_new(cfg)                   (a_empty_list)
#define         list_empty_p(cfg, list)         ((a_empty_list == (list)) ? a_true : a_false)
#define         list_pop(cfg, list)             ((Pair)(list))      // returns: (first, rest)
#define         list_push(cfg, list, item)      (pair_new((cfg),(item),(list)))

extern Actor    deque_new(Config cfg);
extern Actor    deque_empty_p(Config cfg, Actor queue);
extern void     deque_give(Config cfg, Actor queue, Actor item);
extern Actor    deque_take(Config cfg, Actor queue);

#define         dict_new(cfg)                   (a_empty_dict)
#define         dict_empty_p(cfg, dict)         ((a_empty_dict == (dict)) ? a_true : a_false)
extern Actor    dict_lookup(Config cfg, Actor dict, Actor key);
extern Actor    dict_bind(Config cfg, Actor dict, Actor key, Actor value);

extern Actor    actor_new(Config cfg, Action beh);
extern Actor    value_new(Config cfg, Action beh, Any data);
extern Actor    serial_with_value(Config cfg, Actor v);
extern Actor    serial_new(Config cfg, Action beh, Any data);
extern void     actor_become(Actor s, Actor v);

extern Actor    event_new(Config cfg, Actor a, Actor msg);

extern Config   config_new();
#define         config_create(cfg, size, beh)   (((cfg)->create)((cfg), (size), (beh)))
#define         config_destroy(cfg, victim)     (((cfg)->destroy)((cfg), (victim)))
#define         config_enqueue(cfg, e)          (deque_give((cfg), (cfg)->events, (e)))
#define         config_send(cfg, target, msg)   (((cfg)->send)((cfg), (target), (msg)))
extern Actor    config_dispatch(Config cfg);

extern void     beh_pair(Event e);
extern void     beh_deque(Event e);
extern void     beh_event(Event e);
extern void     beh_config(Event e);
extern void     act_serial(Event e);  // "serialized" actor behavior
extern void     beh_halt(Event e);

extern PAIR the_nil_pair_actor;
extern ACTOR the_empty_dict_actor;
extern CONFIG the_root_config;
extern ACTOR the_true_actor;
extern ACTOR the_false_actor;
extern ACTOR the_ignore_actor;
extern VALUE the_halt_actor;

#endif /* _ACTOR_H_ */
