/*

action.c -- Tiny Actor Run-Time

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

#include "action.h"

/**
LET forward_beh(a) = \m.[ SEND m TO a ]
**/
void
act_forward(Event e)
{
    TRACE(fprintf(stderr, "act_forward{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor a = DATA(DATA(SELF(e)));  // target
    Any m = MSG(e);  // message
    config_send(e->sponsor, a, m);
}

/**
LET oneshot_beh(a) = \m.[
    SEND m TO a
    BECOME \_.[]
]
**/
void
act_oneshot(Event e)
{
    TRACE(fprintf(stderr, "act_oneshot{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor a = DATA(DATA(SELF(e)));  // target
    Any m = MSG(e);  // message
    config_send(e->sponsor, a, m);
    actor_become(SELF(e), a_sink);
}

/**
LET prefix_beh(prefix) = \(cust, req).[ SEND (prefix, req) TO cust ]
**/
void
act_prefix(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_prefix{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Any prefix = DATA(DATA(SELF(e)));  // prefix
    p = MSG(e);  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    config_send(e->sponsor, cust, PR(prefix, req));
}

/**
LET label_beh(cust, label) = \msg.[ SEND (label, msg) TO cust ]
**/
void
act_label(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_label{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = DATA(DATA(SELF(e)));  // (cust, label)
    Actor cust = p->h;
    Any label = p->t;
    config_send(e->sponsor, cust, PR(label, MSG(e)));
}

/**
LET tag_beh(cust) = \msg.[ SEND (SELF, msg) TO cust ]
**/
void
act_tag(Event e)
{
    TRACE(fprintf(stderr, "act_tag{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor cust = DATA(DATA(SELF(e)));  // cust
    Any msg = MSG(e);  // msg
    config_send(e->sponsor, cust, PR(SELF(e), msg));
}
/**
LET join_0_beh(cust, first, k_rest) = \($k_rest, rest).[ SEND (first, rest) TO cust ]
**/
static void
act_join_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_join_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = DATA(DATA(SELF(e)));  // (cust, first, k_rest)
    Actor cust = p->h;
    p = p->t;
    Any first = p->h;
    Actor k_rest = p->t;
    p = MSG(e);  // ($k_rest, rest)
    if (p->h == k_rest) {
        Any rest = p->t;
        config_send(e->sponsor, cust, PR(first, rest));
        return;
    }
}
/**
LET join_1_beh(cust, k_first, rest) = \($k_first, first).[ SEND (first, rest) TO cust ]
**/
static void
act_join_1(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_join_1{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = DATA(DATA(SELF(e)));  // (cust, k_first, rest)
    Actor cust = p->h;
    p = p->t;
    Actor k_first = p->h;
    Any rest = p->t;
    p = MSG(e);  // ($k_first, first)
    if (p->h == k_first) {
        Any first = p->t;
        config_send(e->sponsor, cust, PR(first, rest));
        return;
    }
}
/**
LET join_beh(cust, k_first, k_rest) = \msg.[
	CASE msg OF
	($k_first, first) : [
		BECOME join_0_beh(cust, first, k_rest)
	]
	($k_rest, rest) : [
		BECOME join_1_beh(cust, k_first, rest)
	]
	END
]
**/
static void
act_join(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_join{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = DATA(DATA(SELF(e)));  // (cust, k_first, k_rest)
    Actor cust = p->h;
    p = p->t;
    Actor k_first = p->h;
    Actor k_rest = p->t;
    p = MSG(e);  // ($k_first, first) | ($k_rest, rest)
    if (p->h == k_first) {
        Any first = p->t;
        actor_become(SELF(e), behavior_new(act_join_0, PR(cust, PR(first, k_rest))));
        return;
    }
    if (p->h == k_rest) {
        Any rest = p->t;
        actor_become(SELF(e), behavior_new(act_join_1, PR(cust, PR(k_first, rest))));
        return;
    }
}
/**
LET fork_beh(cust, head, tail) = \(h_req, t_req).[
	CREATE k_head WITH tag_beh(SELF)
	CREATE k_tail WITH tag_beh(SELF)
	SEND (k_head, h_req) TO head
	SEND (k_tail, t_req) TO tail
	BECOME join_beh(cust, k_head, k_tail)
]
**/
void
act_fork(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_fork{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = DATA(DATA(SELF(e)));  // (cust, head, tail)
    Actor cust = p->h;
    p = p->t;
    Actor head = p->h;
    Actor tail = p->t;
    p = MSG(e);  // (h_req, t_req)
    Any h_req = p->h;
    Any t_req = p->t;
    Actor k_head = actor_new(behavior_new(act_tag, SELF(e)));
    Actor k_tail = actor_new(behavior_new(act_tag, SELF(e)));
    config_send(e->sponsor, head, PR(k_head, h_req));
    config_send(e->sponsor, tail, PR(k_tail, t_req));
    Pair args = PR(cust, PR(k_head, k_tail));  // (cust, k_head, k_tail)
    actor_become(SELF(e), behavior_new(act_join, args));
}
