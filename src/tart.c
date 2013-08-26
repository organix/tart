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
val_seed(Event e)
{
    TRACE(fprintf(stderr, "val_seed{self=%p, msg=%p}\n", SELF(e), MSG(e)));
}

void 
val_terminus(Event e)
{
    TRACE(fprintf(stderr, "val_terminus{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Config cfg = (Config)MSG(e);
    config_send(SPONSOR(e), (Actor)SPONSOR(e), value_new(val_destroy_config, cfg));
}

void
run_tests()
{
    TRACE(fprintf(stderr, "---- tart unit tests ----\n"));
    TRACE(fprintf(stderr, "NIL = %p\n", NIL));
    TRACE(fprintf(stderr, "NOTHING = %p\n", NOTHING));
    TRACE(fprintf(stderr, "a_halt = %p\n", a_halt));
    TRACE(fprintf(stderr, "a_ignore = %p\n", a_ignore));

    Config cfg = config_new(NOTHING);
    Actor a_terminus = value_new(val_terminus, NOTHING);
    Actor a_seed = value_new(val_seed, NOTHING);
    Actor a_create_config = value_new(val_create_config, PR(a_terminus, PR(a_seed, a_true)));
    TRACE(fprintf(stderr, "host config = %p\n", cfg));
    TRACE(fprintf(stderr, "a_terminus = %p\n", a_terminus));
    TRACE(fprintf(stderr, "a_seed = %p\n", a_seed));
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
