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
#include "actor.h"
#include "expr.h"
#include "number.h"
#include "string.h"
#include "action.h"
#include "universe.h"

void
halt(char * msg)
{
    TRACE(fprintf(stderr, "%s **** HALTED ****\n", msg));
    assert(0);
    for (;;)    // loop forever
        ;
}

/*
 *  Unit tests
 */

void
val_in_config(Event e)
{
    TRACE(fprintf(stderr, "val_in_config{   config=%p, self=%p, msg=%p}\n", SPONSOR(e), SELF(e), MSG(e)));
    config_send(SPONSOR(e), a_ignore, a_true);
}

void
val_seed(Event e)
{
    TRACE(fprintf(stderr, "val_seed{        config=%p, self=%p, msg=%p}\n", SPONSOR(e), SELF(e), MSG(e)));
    Actor config = MSG(e);
    Actor a_in_config = value_new(val_in_config, NOTHING);
    config_send((Config)config, a_in_config, a_true); // send message to new config
    config_send(SPONSOR(e), config, actor_new(val_dispatch)); // schedule guest configuration for dispatch
}

void
val_nest(Event e)
{
    TRACE(fprintf(stderr, "val_nest{        config=%p, self=%p, msg=%p}\n", SPONSOR(e), SELF(e), MSG(e)));
    Actor config = MSG(e);
    config_send(SPONSOR(e), config, actor_new(val_dispatch)); // schedule guest configuration for dispatch
    Actor a_cust = value_new(val_seed, NOTHING);
    Actor a_create_config = value_new(val_create_config, a_cust);
    TRACE(fprintf(stderr, "val_nest: a_cust=%p, a_create_config=%p\n", a_cust, a_create_config));
    config_send((Config)config, config, a_create_config);
}

void
run_tests()
{
    TRACE(fprintf(stderr, "---- tart unit tests ----\n"));
    TRACE(fprintf(stderr, "NIL      = %p\n", NIL));
    TRACE(fprintf(stderr, "NOTHING  = %p\n", NOTHING));
    TRACE(fprintf(stderr, "a_halt   = %p\n", a_halt));
    TRACE(fprintf(stderr, "a_ignore = %p\n", a_ignore));

    TRACE(fprintf(stderr, "---- configuration creation ----\n"));
    Config cfg = host_new();
    Actor a_cust = value_new(val_seed, NOTHING);
    Actor a_create_config = value_new(val_create_config, a_cust);
    TRACE(fprintf(stderr, "config          = %p\n", cfg));
    TRACE(fprintf(stderr, "a_cust          = %p\n", a_cust));
    TRACE(fprintf(stderr, "a_create_config = %p\n", a_create_config));
    config_send(cfg, (Actor)cfg, a_create_config);
    while (config_dispatch(cfg) != NOTHING)
        ;

    TRACE(fprintf(stderr, "---- configuration nesting ----\n"));
    Actor a_nesting_cust = value_new(val_nest, NOTHING);
    a_create_config = value_new(val_create_config, a_nesting_cust);
    TRACE(fprintf(stderr, "a_nesting_cust  = %p\n", a_nesting_cust));
    TRACE(fprintf(stderr, "a_create_config = %p\n", a_create_config));
    config_send(cfg, (Actor)cfg, a_create_config);
    while (config_dispatch(cfg) != NOTHING)
        ;

    test_action();
    test_universe();
    test_expr();
    test_number();
    test_string();
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
