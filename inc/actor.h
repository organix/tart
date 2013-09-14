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
Actor  [*|*|...]
        | +--> eqv()
        V
       BEH

Value  [*|*|*]
        | | +--> DATA
        | +--> eqv()
        V
       CODE

Serial [*|*|*|*]------+
        | | |          V
        | | +------> [*|*]
        | +-> eqv()   |
        V             | +--> STATE
   act_serial         V
                     STRATEGY

Pair   [*|*|*|*]
        | | | +--> tail
        | | +--> head
        | +--> eqv()
        V
     beh_pair
**/

typedef struct actor ACTOR, *Actor;
typedef struct pair PAIR, *Pair;
typedef struct fifo FIFO, *Fifo;
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

#define ACTOR_INIT(a,beh)   ( ((a)->eqv = actor_eqv_method), (BEH(a) = (beh)), (a) )
#define ACTOR_DECL(beh)     { (beh), actor_eqv_method }

#define a_root_config (&the_root_config)

#define a_empty_list ((Actor)(&the_nil_pair_actor))
#define a_empty_dict ((Actor)(&the_empty_dict_actor))
#define a_true ((Actor)(&the_true_actor))
#define a_false ((Actor)(&the_false_actor))
#define a_ignore ((Actor)(&the_ignore_actor))
#define a_halt ((Actor)(&the_halt_actor))

struct actor {
    Action      beh;
    // Actor       (*eqv)(Config cfg, Actor pattern, Actor value);  // polymorphic equivalence relation
    void        (*eqv)(Event e, Actor cust, Actor pattern, Actor value);
};

struct pair {
    ACTOR       _act;
    Actor       h;
    Actor       t;
};

struct fifo {
    ACTOR       _act;
    size_t      h;  // offset for next "take"
    size_t      t;  // offset for next "give"
    size_t      m;  // wrap-around offset mask
    Actor       p[0];  // managed actor references
};

struct value {
    ACTOR       _act;
    Any         data;
};

struct serial {
    ACTOR       _act;
    Actor       beh_now;  // current "behavior" actor
    Actor       beh_next;  // "behavior" for next event
};

struct event {
    ACTOR       _act;
    Config      sponsor;  // sponsor configuration
    Actor       target;  // target actor
    Actor       message;  // message to deliver
    Actor       actors;  // effects: newly created actors
    Actor       events;  // effects: newly created events
};

struct config {
    ACTOR       _act;
    void        (*fail)(Config cfg, Actor reason);  // error reporting procedure
    Actor       (*create)(Config cfg, size_t n_bytes, Action beh);  // actor creation procedure
    void        (*destroy)(Config cfg, Actor victim);  // reclaim actor resources
    void        (*send)(Event e, Actor target, Actor msg);  // event creation procedure
    Actor       events;  // queue of messages in-transit
};

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

extern Actor    fifo_new(Config cfg, size_t n);  // WARNING! n must be a power of 2
extern Actor    fifo_empty_p(Config cfg, Actor q);
extern Actor    fifo_give(Config cfg, Actor q, Actor item);
extern Actor    fifo_take(Config cfg, Actor q);

extern Actor    actor_new(Config cfg, Action beh);
extern Actor    value_new(Config cfg, Action beh, Any data);
extern Actor    serial_with_value(Config cfg, Actor v);
extern Actor    serial_new(Config cfg, Action beh, Any data);
extern void     actor_eqv_method(Event e, Actor cust, Actor this, Actor that);
#define         actor_eqv(e, cust, this, that)   (((this)->eqv)((e), (cust), (this), (that)))

extern void     actor_become(Event e, Actor v);
extern Actor    event_new(Config cfg, Actor a, Actor msg);

#define         config_fail(cfg, reason)        (((cfg)->fail)((cfg), (reason)))
#define         config_create(cfg, size, beh)   (((cfg)->create)((cfg), (size), (beh)))
#define         config_destroy(cfg, victim)     (((cfg)->destroy)((cfg), (victim)))
#define         config_enqueue(cfg, e)          (deque_give((cfg), (cfg)->events, (e)))
#define         config_send(e, target, msg)     (((SPONSOR(e))->send)((e), (target), (msg)))
extern Actor    config_dispatch(Config cfg);
extern void     config_apply_effects(Config cfg, Event e);

extern Config   quota_config_new(Config sponsor, size_t n_bytes);
extern void     quota_config_report(Config cfg);

extern void     beh_pair(Event e);
extern void     beh_deque(Event e);
extern void     beh_fifo(Event e);
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

#define e_nomem ((Actor)(&fail_reason_nomem))
#define e_inval ((Actor)(&fail_reason_inval))

extern ACTOR fail_reason_nomem;
extern ACTOR fail_reason_inval;

#endif /* _ACTOR_H_ */
