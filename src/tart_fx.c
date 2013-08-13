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
#include "actor.h"
//#include "object.h"
#include "effect.h"

void
halt(char * msg)
{
    TRACE(fprintf(stderr, "%s **** HALTED ****\n", msg));
    assert(0);
    for (;;)    // loop forever
        ;
}

/**
LET forward_beh(a) = \m.SEND () TO \_.[ SEND m TO a ]
**/
void
fx_forward(Event e)
{
    TRACE(fprintf(stderr, "fx_forward{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor a = STATE(SELF(e));  // target
    Any m = MSG(e);  // message
    Actor args = PR(a_commit, PR(a, m));  // (cust, target, message)
    Actor a_send = serial_new(act_send, args);
    Actor a_begin = serial_new(act_begin, a_send);
    TRACE(fprintf(stderr, "fx_forward: delegate=%p\n", a_begin));
    // invoke delegate
    config_send(SPONSOR(e), a_begin, NIL);  // NOTE: act_begin() ignores MSG(e)
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
    TRACE(fprintf(stderr, "fx_oneshot{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor a = STATE(SELF(e));  // target
    Any m = MSG(e);  // message
    Actor args = PR(a_commit, PR(a, m));  // (cust, target, message)
    Actor a_send = serial_new(act_send, args);
    Actor a_begin = serial_new(act_begin, a_send);
    TRACE(fprintf(stderr, "fx_oneshot: delegate=%p\n", a_begin));
    // become sink
    actor_become(SELF(e), a_ignore);
    // invoke delegate
    config_send(SPONSOR(e), a_begin, NIL);  // NOTE: act_begin() ignores MSG(e)
}

/**
LET tag_beh(cust) = \msg.[ SEND (SELF, msg) TO cust ]
**/
void
fx_tag(Event e)
{
    TRACE(fprintf(stderr, "fx_tag{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor cust = STATE(SELF(e));  // cust
    Any msg = MSG(e);  // msg
    Actor args = PR(a_commit, PR(cust, PR(SELF(e), msg)));  // (cust, target, message)
    Actor a_send = serial_new(act_send, args);
    Actor a_begin = serial_new(act_begin, a_send);
    TRACE(fprintf(stderr, "fx_tag: delegate=%p\n", a_begin));
    // invoke delegate
    config_send(SPONSOR(e), a_begin, NIL);  // NOTE: act_begin() ignores MSG(e)
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
test_effect()
{
    TRACE(fprintf(stderr, "---- test_effect ----\n"));
    TRACE(fprintf(stderr, "a_commit = %p\n", a_commit));
    Config cfg = config_new();
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));

    // forwarding example
    Actor a_tag = serial_new(fx_tag, a_ignore);
    TRACE(fprintf(stderr, "a_tag = %p\n", a_tag));    
    Actor a_once = serial_new(fx_oneshot, a_ignore);
    TRACE(fprintf(stderr, "a_once = %p\n", a_once));    
    Actor a_fwd = serial_new(fx_forward, a_once);
    TRACE(fprintf(stderr, "a_fwd = %p\n", a_fwd));
    Actor args = PR(a_commit, PR(a_fwd, NIL));  // (cust, target, message)
    Actor a = serial_new(act_send, args);
    Actor a_doit = serial_new(act_begin, a);
    TRACE(fprintf(stderr, "a_doit = %p\n", a_doit));
    config_send(cfg, a_doit, a_once);
    config_send(cfg, a_doit, a_tag);
    while (config_dispatch(cfg) == a_true)
        ;
}

void
run_tests()
{
    TRACE(fprintf(stderr, "---- tart_fx unit tests ----\n"));
    TRACE(fprintf(stderr, "NIL = %p\n", NIL));
    TRACE(fprintf(stderr, "a_halt = %p\n", a_halt));
    TRACE(fprintf(stderr, "a_ignore = %p\n", a_ignore));
//    test_object();
    test_effect();
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
