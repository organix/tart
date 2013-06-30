/*

tart_fx.c -- Tiny Actor Run-Time

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

#include "tart.h"
#include "pair.h"
#include "actor.h"
#include "effect.h"

/**
LET forward_beh(a) = \m.SEND () TO \_.[ SEND m TO a ]
**/
void
fx_forward(Event e)
{
    TRACE(fprintf(stderr, "fx_forward{self=%p, msg=%p}\n", e->actor, e->message));
    Actor a = e->actor->behavior->context;  // target
    Any m = e->message;  // message
    Pair args = PR(&commit_actor, PR(a, m));  // (cust, target, message)
    Actor a_send = actor_new(behavior_new(act_send, args));
    Actor a_begin = actor_new(behavior_new(act_begin, a_send));
    TRACE(fprintf(stderr, "fx_forward: delegate=%p\n", a_begin));
    // invoke delegate
    config_send(e->sponsor, a_begin, NIL);  // NOTE: act_begin() ignores e->message
}

/**
LET oneshot_beh(a) = \m.[
    SEND m TO a
    BECOME \_.[]
]
**/
void
fx_oneshot(Event e)
{
    TRACE(fprintf(stderr, "fx_oneshot{self=%p, msg=%p}\n", e->actor, e->message));
    Actor a = e->actor->behavior->context;  // target
    Any m = e->message;  // message
    Pair args = PR(&commit_actor, PR(a, m));  // (cust, target, message)
    Actor a_send = actor_new(behavior_new(act_send, args));
    Actor a_begin = actor_new(behavior_new(act_begin, a_send));
    TRACE(fprintf(stderr, "fx_oneshot: delegate=%p\n", a_begin));
    // become sink
    actor_become(e->actor, &sink_behavior);
    // invoke delegate
    config_send(e->sponsor, a_begin, NIL);  // NOTE: act_begin() ignores e->message
}

/**
LET tag_beh(cust) = \msg.[ SEND (SELF, msg) TO cust ]
**/
void
fx_tag(Event e)
{
    TRACE(fprintf(stderr, "fx_tag{self=%p, msg=%p}\n", e->actor, e->message));
    Actor cust = e->actor->behavior->context;  // cust
    Any msg = e->message;  // msg
    Pair args = PR(&commit_actor, PR(cust, PR(e->actor, msg)));  // (cust, target, message)
    Actor a_send = actor_new(behavior_new(act_send, args));
    Actor a_begin = actor_new(behavior_new(act_begin, a_send));
    TRACE(fprintf(stderr, "fx_tag: delegate=%p\n", a_begin));
    // invoke delegate
    config_send(e->sponsor, a_begin, NIL);  // NOTE: act_begin() ignores e->message
}

/*
 *  Unit tests
 */

/**
CREATE tag WITH tag_beh(sink)
CREATE once WITH oneshot_beh(tag)
CREATE fwd WITH forward_beh(once)
CREATE doit WITH \_.SEND () TO \_.[ SEND () TO fwd ]
SEND once TO doit
SEND tag TO doit
**/
void
run_tests()
{
    TRACE(fprintf(stderr, "NIL = %p\n", NIL));
    TRACE(fprintf(stderr, "&sink_actor = %p\n", &sink_actor));
    TRACE(fprintf(stderr, "&commit_actor = %p\n", &commit_actor));
    Config cfg = config_new();
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));

    // forwarding example
    Actor a_tag = actor_new(behavior_new(fx_tag, &sink_actor));
    TRACE(fprintf(stderr, "a_tag = %p\n", a_tag));    
    Actor a_once = actor_new(behavior_new(fx_oneshot, &sink_actor));
    TRACE(fprintf(stderr, "a_once = %p\n", a_once));    
    Actor a_fwd = actor_new(behavior_new(fx_forward, a_once));
    TRACE(fprintf(stderr, "a_fwd = %p\n", a_fwd));
    Pair args = PR(&commit_actor, PR(a_fwd, NIL));  // (cust, target, message)
    Actor a = actor_new(behavior_new(act_send, args));
    Actor a_doit = actor_new(behavior_new(act_begin, a));
    TRACE(fprintf(stderr, "a_doit = %p\n", a_doit));
    config_send(cfg, a_doit, a_once);
    config_send(cfg, a_doit, a_tag);
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
