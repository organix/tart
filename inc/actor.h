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
#include "pair.h"

typedef struct config CONFIG, *Config;
typedef struct actor ACTOR, *Actor;

#define MSG(e)  (((Event)(e))->message)
#define SELF(e) (((Event)(e))->actor)
#define CODE(a) (((Actor)(a))->_meth.code)
#define DATA(a) (((Actor)(a))->context)
#define STATE(a) (DATA(DATA(a)))
#define SERIAL(a) (act_serial == CODE(a))

#define a_halt (&halt_actor)
#define a_sink (&sink_actor)

#define NOTHING (a_halt)

struct actor {
    METHOD      _meth;      // code
    Any         context;    // data
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

extern Actor    value_new(Action beh, Any data);

extern Actor    actor_new(Action beh, Any data);
extern void     actor_become(Actor a, Actor v);

extern Event    event_new(Config cfg, Actor a, Any msg);

extern Config   config_new();
extern void     config_enqueue(Config cfg, Event e);
extern void     config_enlist(Config cfg, Actor a);
extern void     config_send(Config cfg, Actor target, Any msg);
extern int      config_dispatch(Config cfg);

extern void     act_serial(Event e);  // "serialized" actor behavior

extern ACTOR halt_actor;
extern ACTOR sink_actor;

#endif /* _ACTOR_H_ */
