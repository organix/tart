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
        V       | +--> DATA
   act_serial   V
               CODE

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

#define NIL (&the_nil_pair)
#define PR(h,t) pair_new((h),(t))
#define NOTHING (a_halt)

#define BEH(a)      (((Actor)(a))->beh)
#define MSG(e)      (((Event)(e))->message)
#define SELF(e)     (((Event)(e))->actor)
#define CODE(v)     BEH(v)
#define DATA(v)     (((Value)(v))->data)
#define VALUE(s)    (((Serial)(s))->value)
#define STRATEGY(s) CODE(VALUE(s))
#define STATE(s)    DATA(VALUE(s))

#define a_halt ((Actor)(&the_halt_actor))
#define a_ignore ((Actor)(&the_ignore_actor))

struct actor {
    Action      beh;
};

struct pair {
    ACTOR       _act;
    Any         h;
    Any         t;
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
    Config      sponsor;    // sponsor configuration
    Actor       actor;      // target actor
    Any         message;    // message to deliver
};

struct config {
    Pair        event_q;    // queue of messages in-transit
    Pair        actors;     // list of actors created
};

extern Pair     pair_new(Any h, Any t);

extern Pair     list_new();
extern int      list_empty_p(Pair list);
extern Pair     list_pop(Pair list);
extern Pair     list_push(Pair list, Any item);

extern Pair     deque_new();
extern int      deque_empty_p(Pair q);
extern void     deque_give(Pair q, Any item);
extern Any      deque_take(Pair q);
extern void     deque_return(Pair q, Any item);
extern Any      deque_lookup(Pair q, int i);
extern void     deque_bind(Pair q, int i, Any item);

extern Pair     dict_new();
extern int      dict_empty_p(Pair dict);
extern Any      dict_lookup(Pair dict, Any key);
extern Pair     dict_bind(Pair dict, Any key, Any value);

extern Actor    value_new(Action beh, Any data);

extern Actor    serial_new(Action beh, Any data);
extern Actor    serial_with_value(Actor v);
extern void     actor_become(Actor s, Actor v);

extern Event    event_new(Config cfg, Actor a, Any msg);

extern Config   config_new();
extern void     config_enqueue(Config cfg, Event e);
extern void     config_enlist(Config cfg, Actor a);
extern void     config_send(Config cfg, Actor target, Any msg);
extern int      config_dispatch(Config cfg);

extern void     beh_halt(Event e);
extern void     beh_pair(Event e);
extern void     act_serial(Event e);  // "serialized" actor behavior

extern PAIR the_nil_pair;
extern VALUE the_halt_actor;
extern VALUE the_ignore_actor;

#endif /* _ACTOR_H_ */
