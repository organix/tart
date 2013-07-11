/*

universe.c -- Tiny Actor Run-Time

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

#include "universe.h"

/**
LET fail_beh = \msg.[
    THROW (#FAIL!, msg)
]
**/
static inline void
act_fail(Event e)
{
    TRACE(fprintf(stderr, "act_fail{self=%p, msg=%p}\n", e->actor, e->message));
    halt("FAIL!");
}
static BEHAVIOR fail_behavior = { act_fail, NIL };
ACTOR fail_actor = { &fail_behavior };

/**
CREATE empty_env WITH \(cust, req).[
    CASE req OF
    (#lookup, _) : [
        LET (ok, fail) = $cust IN [
            SEND (cust, req) TO fail
        ]
    ]
    _ : value_beh(cust, req)  # delegate
    END
]
**/
static void
act_empty_env(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_empty_env{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_lookup == p->h) {  // (#lookup, _)
        p = (Pair)cust;
        Actor fail = p->t;
        config_send(e->sponsor, fail, e->message);
    } else {
        act_value(e);
    }
}
static BEHAVIOR empty_env_behavior = { act_empty_env, NIL };
ACTOR empty_env_actor = { &empty_env_behavior };

/**
LET value_beh = \(cust, req).[
    CASE req OF
    (#eval, _) : [ SEND SELF TO cust ]
    _ : fail_beh(cust, req)  # default
    END
]
**/
void
act_value(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_value{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_eval == p->h) {  // (#eval, _)
        config_send(e->sponsor, cust, e->actor);
    } else {
        act_fail(e);  // default
    }
}

/**
LET scope_beh(dict, parent) = \(cust, req).[
    CASE req OF
    (#lookup, name) : [
        LET (ok, fail) = $cust IN [
            LET value = $(dict_lookup(dict, name))
            CASE value OF
            ? : [ SEND (cust, req) TO parent ]
            _ : [ SEND value TO ok ]
            END
        ]
    ]
    (#bind, name, value) : [
        LET dict' = $(dict_bind(dict, name, value))
        BECOME scope_beh(dict', parent)
        SEND SELF TO cust
    ]
    _ : value_beh(cust, req)  # delegate
    END
]
**/
void
act_scope(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_scope{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (dict, parent)
    Pair dict = p->h;
    Actor parent = p->t;
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_lookup == p->h) {  // (#lookup, name)
        Actor name = p->t;
        p = (Pair)cust;
        Actor ok = p->h;
        Actor fail = p->t;
        Any value = dict_lookup(dict, name);
        if (value == NULL) {
            config_send(e->sponsor, parent, e->message);
        } else {
            config_send(e->sponsor, ok, value);
        }
    } else if (s_bind == p->h) {  // (#bind, name, value)
        p = p->t;
        Actor name = p->h;
        Any value = p->t;
        dict = dict_bind(dict, name, value);
//        actor_become(e->actor, behavior_new(act_scope, PR(dict, parent)));  -- see next two lines for equivalent
        p = e->actor->behavior->context;  // (dict, parent)
        p->h = dict;  // WARNING! this directly manipulates the data in this behavior
        config_send(e->sponsor, cust, e->actor);
    } else {
        act_value(e);  // delegate
    }
}

/**
LET skip_ptrn_beh = \(cust, req).[
    CASE req OF
    (#match, _, env) : [
        LET (ok, _) = $cust IN [
            SEND env TO ok
        ]
    ]
    _ : value_beh(cust, req)  # delegate
    END
]
**/
static void
act_skip_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_skip_ptrn{self=%p, msg=%p}\n", e->actor, e->message));
    Actor name = e->actor->behavior->context;  // (name)
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_match == p->h) {  // (#match, value, env)
        p = p->t;
        Any value = p->h;
        Actor env = p->t;
        p = (Pair)cust;
        Actor ok = p->h;
        Actor fail = p->t;
        config_send(e->sponsor, ok, env);
    } else {
        act_value(e);  // delegate
    }
}
static BEHAVIOR skip_ptrn_behavior = { act_skip_ptrn, NIL };
ACTOR skip_ptrn_actor = { &skip_ptrn_behavior };

/**
LET bind_ptrn_beh(name) = \(cust, req).[
    CASE req OF
    (#match, value, env) : [
        LET (ok, fail) = $cust IN [
            SEND (ok, #bind, name, value) TO env
        ]
    ]
    _ : value_beh(cust, req)  # delegate
    END
]
**/
void
act_bind_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_bind_ptrn{self=%p, msg=%p}\n", e->actor, e->message));
    Actor name = e->actor->behavior->context;  // (name)
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_match == p->h) {  // (#match, value, env)
        p = p->t;
        Any value = p->h;
        Actor env = p->t;
        p = (Pair)cust;
        Actor ok = p->h;
        Actor fail = p->t;
        config_send(e->sponsor, env, PR(ok, PR(s_bind, PR(name, value))));
    } else {
        act_value(e);  // delegate
    }
}

/**
LET name_beh(value) = \(cust, req).[
    CASE req OF
    (#eval, env) : [
        SEND ((cust, fail), #lookup, SELF) TO env
    ]
    _ : fail_beh(cust, req)  # default
    END
]
**/
void
act_name(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_name{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        config_send(e->sponsor, env, PR(PR(cust, a_fail), PR(s_lookup, e->actor)));
    } else {
        act_fail(e);  // default
    }
}

/**
LET comb_0_beh(cust, opnd, env) = \comb.[
    SEND (cust, #comb, opnd, env) TO comb
]
**/
static void
act_comb_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_comb_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (cust, opnd, env)
    Actor cust = p->h;
    p = p->t;
    Actor opnd = p->h;
    Actor env = p->t;
    Actor comb = e->message;  // (comb)
    config_send(e->sponsor, comb, PR(cust, PR(s_comb, PR(opnd, env))));
}
/**
LET comb_beh(oper, opnd) = \(cust, req).[
    CASE req OF
    (#eval, env) : [
        CREATE comb_0 WITH comb_0_beh(cust, opnd, env)
        SEND (comb_0, #eval, env) TO oper
    ]
    _ : fail_beh(cust, req)  # default
    END
]
**/
void
act_comb(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_comb{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (oper, opnd)
    Actor oper = p->h;
    Actor opnd = p->t;
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        Actor comb_0 = actor_new(behavior_new(act_comb_0, PR(cust, PR(opnd, env))));
        config_send(e->sponsor, oper, PR(comb_0, PR(s_eval, env)));
    } else {
        act_fail(e);  // default
    }
}

/**
LET appl_0_beh(cust, comb, env) = \args.[
    SEND (cust, #comb, args, env) TO comb
]
**/
static void
act_appl_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_appl_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (cust, comb, env)
    Actor cust = p->h;
    p = p->t;
    Actor comb = p->h;
    Actor env = p->t;
    Actor args = e->message;  // (args)
    config_send(e->sponsor, comb, PR(cust, PR(s_comb, PR(args, env))));
}
/**
LET appl_beh(comb) = \(cust, req).[
    CASE req OF
    (#comb, opnd, env) : [
        CREATE appl_0 WITH appl_0_beh(cust, comb, env)
        SEND (appl_0, #eval, env) TO opnd
    ]
    _ : value_beh(cust, req)  # delegate
    END
]
**/
void
act_appl(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_appl{self=%p, msg=%p}\n", e->actor, e->message));
    Actor comb =  e->actor->behavior->context;  // (comb)
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_comb == p->h) {  // (#comb, opnd, env)
        p = p->t;
        Actor opnd = p->h;
        Actor env = p->t;
        Actor appl_0 = actor_new(behavior_new(act_appl_0, PR(cust, PR(comb, env))));
        config_send(e->sponsor, opnd, PR(appl_0, PR(s_eval, env)));
    } else {
        act_value(e);  // delegate
    }
}

/**
LET oper_0_beh(cust, evar, env_d) = \env_p.[
    SEND ((cust, fail), #match, env_d, env_p) TO evar
]
**/
static void
act_oper_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_oper_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (cust, evar, env_d)
    Actor cust = p->h;
    p = p->t;
    Actor evar = p->t;
    Actor env_d = p->t;
    Actor env_p = e->message;  // (env_p)
    config_send(e->sponsor, evar, PR(PR(cust, a_fail), PR(s_match, PR(env_d, env_p))));
}
/**
LET oper_1_beh(cust, body) = \env_e.[
    SEND (cust, #eval, env_e) TO body
]
**/
static void
act_oper_1(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_oper_1{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (cust, body)
    Actor cust = p->h;
    Actor body = p->t;
    Actor env_e = e->message;  // (env_e)
    config_send(e->sponsor, body, PR(cust, PR(s_eval, env_e)));
}
/**
LET oper_beh(env_s, form, evar, body) = \(cust, req).[
	CASE req OF
	(#comb, opnd, env_d) : [
        CREATE scope WITH scope_beh(dict_new(), env_s)
        SEND ((oper_0, fail), #match, opnd, scope) TO form
        CREATE oper_0 WITH \env_p.[  # (oper_1, evar, env_d)
            CASE evar OF
            $skip_ptrn : [  # optimize "lambda" case
                SEND (cust, #eval, env_p) TO body
            ]
            _ : [
                SEND ((oper_1, fail), #match, env_d, env_p) TO evar
                CREATE oper_1 WITH \env_e.[  # (cust, body)
                    SEND (cust, #eval, env_e) TO body
                ]
            ]
        ]
    ]
	_ : value_beh(cust, req)  # delegate
	END
]
**/
void
act_oper(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_oper{self=%p, msg=%p}\n", e->actor, e->message));
    p =  e->actor->behavior->context;  // (env_s, form, evar, body)
    Actor env_s = p->h;
    p = p->t;
    Actor form = p->h;
    p = p->t;
    Actor evar = p->h;
    Actor body = p->t;
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_comb == p->h) {  // (#comb, opnd, env_d)
        p = p->t;
        Actor opnd = p->h;
        Actor env_d = p->t;
        Actor scope = actor_new(behavior_new(act_scope, PR(dict_new(), env_s)));
        if (a_skip_ptrn == evar) {  // optimize "lambda" case
            Actor oper_1 = actor_new(behavior_new(act_oper_1, PR(cust, body)));
            config_send(e->sponsor, form, PR(PR(oper_1, a_fail), PR(s_match, PR(opnd, scope))));
        } else {
            Actor oper_1 = actor_new(behavior_new(act_oper_1, PR(cust, body)));
            Actor oper_0 = actor_new(behavior_new(act_oper_0, PR(oper_1, PR(evar, env_d))));
            config_send(e->sponsor, form, PR(PR(oper_0, a_fail), PR(s_match, PR(opnd, scope))));
        }
    } else {
        act_value(e);  // delegate
    }
}

/**
LET vau_beh(form, evar, body) = \(cust, req).[
    CASE req OF
    (#eval, env) : [
        CREATE oper WITH oper_beh(env, form, evar, body)
        SEND oper TO cust
    ]
    _ : fail_beh(cust, req)  # default
    END
]
**/
void
act_vau(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_vau{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        p =  e->actor->behavior->context;  // (form, evar, body)
        Actor oper = actor_new(behavior_new(act_oper, PR(env, p)));
        config_send(e->sponsor, cust, oper);
    } else {
        act_fail(e);  // default
    }
}

/**
LET lambda_beh(form, body) = \(cust, req).[
    CASE req OF
    (#eval, env) : [
        CREATE oper WITH oper_beh(env, form, _, body)
        CREATE appl WITH appl_beh(oper)
        SEND appl TO cust
    ]
    _ : fail_beh(cust, req)  # default
    END
]
**/
void
act_lambda(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_lambda{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // (cust, req)
    Actor cust = p->h;
    Any req = p->t;
    p = req;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        p =  e->actor->behavior->context;  // (form, body)
        Actor form = p->h;
        Actor body = p->t;
        Actor oper = actor_new(behavior_new(act_oper, PR(env, PR(form, PR(a_skip_ptrn, body)))));
        Actor appl = actor_new(behavior_new(act_appl, oper));
        config_send(e->sponsor, cust, appl);
    } else {
        act_fail(e);  // default
    }
}

/*
($define! #t ($vau (x) #ignore ($vau (y) e (eval x e))))  ; usage: ((#t cnsq) altn) ==> cnsq
($define! #f ($vau (x) #ignore ($vau (y) e (eval y e))))  ; usage: ((#f cnsq) altn) ==> altn
*/

/*
 *  NOTE: These global references are initialized by universe_init().
 */
Actor b_true;
Actor b_false;
static Pair symbol_table = NIL;
Actor s_eval;
Actor s_match;
Actor s_lookup;
Actor s_bind;
Actor s_comb;

static Actor
symbol_intern(char * name)
{
    Actor a_symbol = dict_lookup(symbol_table, name);
    if (a_symbol == NULL) {
        a_symbol = actor_new(behavior_new(act_name, name));
        symbol_table = dict_bind(symbol_table, name, a_symbol);
    }
    return a_symbol;
}

/*
0 = \(f, x).x
1 = \(f, x).f(x)
true = \(a, b).a
false = \(a, b).b
*/
void
universe_init(Config cfg)
{
    b_true = actor_new(behavior_new(act_value, (Any)(0 == 0)));
    TRACE(fprintf(stderr, "b_true = %p\n", b_true));
    b_false = actor_new(behavior_new(act_value, (Any)(0 != 0)));
    TRACE(fprintf(stderr, "b_false = %p\n", b_false));
    s_comb = symbol_intern("comb");
    s_bind = symbol_intern("bind");
    s_lookup = symbol_intern("lookup");
    s_match = symbol_intern("match");
    s_eval = symbol_intern("eval");
}

static inline void
act_expect(Event e)
{
    TRACE(fprintf(stderr, "act_expect{self=%p, msg=%p}\n", e->actor, e->message));
    Any expect = e->actor->behavior->context;
    Any actual = e->message;
    if (expect != actual) {
        TRACE(fprintf(stderr, "act_expect: %p != %p\n", expect, actual));
        halt("unexpected");
    }
}
void
test_universe()
{
    Actor s_x;
    Actor I;
    Actor form;
    Actor body;
    Actor expr;
    Actor test;

    TRACE(fprintf(stderr, "---- test_universe ----\n"));
    Config cfg = config_new();
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    universe_init(cfg);

    // (\x.x)(#t) -> #t
    s_x = symbol_intern("x");
    TRACE(fprintf(stderr, "s_x = %p\n", s_x));
    form = actor_new(behavior_new(act_bind_ptrn, s_x));
    body = s_x;
    I = actor_new(behavior_new(act_lambda, PR(form, body)));
    TRACE(fprintf(stderr, "I = %p\n", I));
    expr = actor_new(behavior_new(act_comb, PR(I, b_true)));
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    test = actor_new(behavior_new(act_expect, b_true));
    TRACE(fprintf(stderr, "test = %p\n", test));
    config_send(cfg, expr, PR(test, PR(s_eval, a_empty_env)));
    while (config_dispatch(cfg))
        ;
}
