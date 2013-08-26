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

Serial [*|*]
        | +--> [*|*]
        V       | +--> STATE
   act_serial   V
               STRATEGY

Pair   [*|*|*]
        | | +--> tail
        | +--> head
        V
     beh_pair

Config [*|*|*|*|*]
        | | | | +--> actors
        | | | +--> events
        | | +--> configs
        | +--> [*|*]
        V       | +--> STATE
   beh_config   V
               STRATEGY

Event [*|*|*|*]
       | | | +--> message
       | | +--> target
       | +--> sponsor (Config)
       V
    beh_event
**/

typedef struct actor ACTOR, *Actor;
typedef struct pair PAIR, *Pair;
typedef struct value VALUE, *Value;
typedef struct serial SERIAL, *Serial;
typedef struct event EVENT, *Event;
typedef struct config CONFIG, *Config;

typedef void (*Action)(Event e);

#define NIL ((Actor)(&the_nil_pair_actor))
#define PR(h,t) pair_new((h),(t))
#define NOTHING (a_halt)

#define BEH(a)      (((Actor)(a))->beh)
#define SPONSOR(e)  (((Event)(e))->sponsor)
#define SELF(e)     (((Event)(e))->target)
#define MSG(e)      (((Event)(e))->message)
#define CODE(v)     BEH(v)
#define DATA(v)     (((Value)(v))->data)
#define VALUE(s)    (((Serial)(s))->value)
#define STRATEGY(s) CODE(VALUE(s))
#define STATE(s)    DATA(VALUE(s))
#define TERMINUS(c) VALUE(c)

#define a_empty_list ((Actor)(&the_nil_pair_actor))
#define a_empty_dict ((Actor)(&the_empty_dict_actor))
#define a_true ((Actor)(&the_true_actor))
#define a_false ((Actor)(&the_false_actor))
#define a_ignore ((Actor)(&the_ignore_actor))
#define a_halt ((Actor)(&the_halt_actor))

struct actor {
    Action      beh;
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
    Actor       value;
};

struct event {
    ACTOR       _act;
    Config      sponsor;    // sponsor configuration
    Actor       target;     // target actor
    Actor       message;    // message to deliver
};

struct config {
    SERIAL      terminus;
    Actor       configs;    // "guest" configurations
    Actor       events;     // queue of messages in-transit
    Actor       actors;     // list of actors created
};

extern Actor    pair_new(Actor h, Actor t);

extern Actor    list_new();
extern Actor    list_empty_p(Actor list);
extern Pair     list_pop(Actor list);  // returns: (first, rest)
extern Actor    list_push(Actor list, Actor item);

extern Actor    deque_new();
extern Actor    deque_empty_p(Actor queue);
extern void     deque_give(Actor queue, Actor item);
extern Actor    deque_take(Actor queue);
extern void     deque_return(Actor queue, Actor item);
extern Actor    deque_lookup(Actor queue, Actor index);
extern void     deque_bind(Actor queue, Actor index, Actor item);

extern Actor    dict_new();
extern Actor    dict_empty_p(Actor dict);
extern Actor    dict_lookup(Actor dict, Actor key);
extern Actor    dict_bind(Actor dict, Actor key, Actor value);

extern Actor    actor_new(Action beh);
extern Actor    value_new(Action beh, Any data);
extern Actor    serial_new(Action beh, Any data);
extern Actor    serial_with_value(Actor v);
extern void     actor_become(Actor s, Actor v);

extern Actor    event_new(Config cfg, Actor a, Actor msg);

extern Config   config_new(Actor terminus);
extern void     config_enqueue(Config cfg, Actor e);
extern void     config_enlist(Config cfg, Actor a);
extern void     config_send(Config cfg, Actor target, Actor msg);
extern Actor    config_dispatch(Config cfg);

extern void     beh_pair(Event e);
extern void     beh_deque(Event e);
extern void     beh_event(Event e);
extern void     beh_config(Event e);
extern void     act_serial(Event e);  // "serialized" actor behavior
extern void     beh_halt(Event e);

extern void     val_dispatch(Event e);
extern Actor    dispatch_new(Actor cust, Actor config);

extern void     val_create_config(Event e);
extern void     val_destroy_config(Event e);

extern PAIR the_nil_pair_actor;
extern ACTOR the_empty_dict_actor;
extern ACTOR the_true_actor;
extern ACTOR the_false_actor;
extern ACTOR the_ignore_actor;
extern VALUE the_halt_actor;

#endif /* _ACTOR_H_ */
