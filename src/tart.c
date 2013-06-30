/*

tart.c -- Tiny Actor Run-Time

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
#include "action.h"

/*
 *  Unit tests
 */

/**
LET dump_pair_of_pairs_beh(label) = \((a, b), (c, d)).[ Console.println("%s: ((%s, %s), (%s, %s))", label, a, b, c, d) ]
**/
void
act_dump_pair_of_pairs(Event e)
{
    TRACE(fprintf(stderr, "act_dump_pair_of_pairs{self=%p, msg=%p}\n", e->actor, e->message));
    char * label = e->actor->behavior->context;  // (cust, head, tail)
    if (label) {
        TRACE(fprintf(stderr, "%s: ", label));
    }
    Pair p = e->message;
    Pair ab = p->h;
    Pair cd = p->t;
    TRACE(fprintf(stderr, "(%p, %p) = ", ab, cd));
    TRACE(fprintf(stderr, "((%s, %s), (%s, %s))\n", ab->h, ab->t, cd->h, cd->t));
}

/**
CREATE once WITH oneshot_beh(sink)
CREATE fwd WITH forward_beh(once)
SEND () TO fwd
SEND once TO fwd
**/
void
run_tests()
{
    TRACE(fprintf(stderr, "NIL = %p\n", NIL));
    TRACE(fprintf(stderr, "&sink_actor = %p\n", &sink_actor));
    Config cfg = config_new();
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));

    // forwarding example
    Actor a_once = actor_new(behavior_new(act_oneshot, &sink_actor));
    TRACE(fprintf(stderr, "a_once = %p\n", a_once));    
    Actor a_fwd = actor_new(behavior_new(act_forward, a_once));
    TRACE(fprintf(stderr, "a_fwd = %p\n", a_fwd));
    config_send(cfg, a_fwd, NIL);
    config_send(cfg, a_fwd, a_once);
    while (config_dispatch(cfg))
        ;

    // fork-join example
    char * s_zero = "zero";
    TRACE(fprintf(stderr, "s_zero = %p \"%s\"\n", s_zero, s_zero));
    Actor a_zero = actor_new(behavior_new(act_prefix, s_zero));
    char * s_one = "one";
    TRACE(fprintf(stderr, "s_one = %p \"%s\"\n", s_one, s_one));
    Actor a_one = actor_new(behavior_new(act_prefix, s_one));
    char * s_123 = "123";
    TRACE(fprintf(stderr, "s_123 = %p \"%s\"\n", s_123, s_123));
    char * s_456 = "456";
    TRACE(fprintf(stderr, "s_one = %p \"%s\"\n", s_456, s_456));
    Actor a_dump = actor_new(behavior_new(act_dump_pair_of_pairs, "fork-join"));
    Behavior beh = behavior_new(act_fork, PR(a_dump, PR(a_zero, a_one)));
    config_send(cfg, actor_new(beh), PR(s_123, s_456));
    config_send(cfg, actor_new(beh), PR(s_456, s_123));
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
