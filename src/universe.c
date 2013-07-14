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
#include "action.h"  // act_par_expr() uses act_tag()

/**
CREATE fail WITH \msg.[
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
CREATE empty_env WITH \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#lookup, _) : [ SEND msg TO fail ]
    _ : value_beh(msg)  # delegate
    END
]
**/
static void
act_empty_env(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_empty_env{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    TRACE(fprintf(stderr, "act_empty_env: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_lookup == p->h) {  // (#lookup, _)
        TRACE(fprintf(stderr, "act_empty_env: (#lookup, _) -> FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    } else {
        act_value(e);
    }
}
static BEHAVIOR empty_env_behavior = { act_empty_env, NIL };
ACTOR empty_env_actor = { &empty_env_behavior };

/**
LET value_beh() = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
act_value(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_value{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    TRACE(fprintf(stderr, "act_value: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_eval == p->h) {  // (#eval, _)
        TRACE(fprintf(stderr, "act_value: (#eval, _)\n"));
        config_send(e->sponsor, ok, e->actor);
    } else {
        TRACE(fprintf(stderr, "act_value: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}

/**
LET scope_beh(dict, parent) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#lookup, name) : [
        LET value = $(dict_lookup(dict, name))
        CASE value OF
        ? : [ SEND msg TO parent ]
        _ : [ SEND value TO ok ]
        END
    ]
    (#bind, name, value) : [
        LET dict' = $(dict_bind(dict, name, value))
        BECOME scope_beh(dict', parent)
        SEND SELF TO ok
    ]
    _ : value_beh(msg)  # delegate
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
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    TRACE(fprintf(stderr, "act_scope: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_lookup == p->h) {  // (#lookup, name)
        Actor name = p->t;
        Any value = dict_lookup(dict, name);
        TRACE(fprintf(stderr, "act_scope: (#lookup, %p \"%s\") -> %p\n", name, name->behavior->context, value));
        if (value == NULL) {
            config_send(e->sponsor, parent, e->message);
        } else {
            config_send(e->sponsor, ok, value);
        }
    } else if (s_bind == p->h) {  // (#bind, name, value)
        p = p->t;
        Actor name = p->h;
        Any value = p->t;
        TRACE(fprintf(stderr, "act_scope: (#bind, %p, %p)\n", name, value));
        dict = dict_bind(dict, name, value);
//        actor_become(e->actor, behavior_new(act_scope, PR(dict, parent)));  -- see next two lines for equivalent
        p = e->actor->behavior->context;  // (dict, parent)
        p->h = dict;  // WARNING! this directly manipulates the data in this behavior
        config_send(e->sponsor, ok, e->actor);
    } else {
        act_value(e);  // delegate
    }
}

/**
LET skip_ptrn_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#match, _, env) : [ SEND env TO ok ]
    _ : value_beh(msg)  # delegate
    END
]
**/
static void
act_skip_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_skip_ptrn{self=%p, msg=%p}\n", e->actor, e->message));
    Actor name = e->actor->behavior->context;  // (name)
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    TRACE(fprintf(stderr, "act_skip_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_match == p->h) {  // (#match, _, env)
        p = p->t;
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_skip_ptrn: (#match, _, %p)\n", env));
        config_send(e->sponsor, ok, env);
    } else {
        act_value(e);  // delegate
    }
}
/**
CREATE skip_ptrn WITH skip_ptrn_beh
**/
static BEHAVIOR skip_ptrn_behavior = { act_skip_ptrn, NIL };
ACTOR skip_ptrn_actor = { &skip_ptrn_behavior };

/**
LET bind_ptrn_beh(name) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#match, value, env) : [ SEND ((ok, fail), #bind, name, value) TO env ]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
act_bind_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_bind_ptrn{self=%p, msg=%p}\n", e->actor, e->message));
    Actor name = e->actor->behavior->context;  // (name)
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    TRACE(fprintf(stderr, "act_bind_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_match == p->h) {  // (#match, value, env)
        p = p->t;
        Any value = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_bind_ptrn: (#match, %p, %p)\n", value, env));
        config_send(e->sponsor, env, PR(PR(ok, fail), PR(s_bind, PR(name, value))));
    } else {
        act_value(e);  // delegate
    }
}

/**
LET name_beh(value) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [ SEND ((ok, fail), #lookup, SELF) TO env ]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
act_name(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_name{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "act_name: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_name: (#eval, %p)\n", env));
        config_send(e->sponsor, env, PR(cust, PR(s_lookup, e->actor)));
    } else {
        TRACE(fprintf(stderr, "act_name: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}

/**
LET comb_0_beh((ok, fail), opnd, env) = \comb.[
    SEND ((ok, fail), #comb, opnd, env) TO comb
]
**/
static void
act_comb_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_comb_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), opnd, env)
    Pair cust = p->h;
    p = p->t;
    Actor opnd = p->h;
    Actor env = p->t;
    Actor comb = e->message;  // (comb)
    TRACE(fprintf(stderr, "act_comb_0: ok=%p, fail=%p, comb=%p, opnd=%p, env=%p\n", cust->h, cust->t, comb, opnd, env));
    config_send(e->sponsor, comb, PR(cust, PR(s_comb, PR(opnd, env))));
}
/**
LET comb_beh(oper, opnd) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [
        CREATE comb_0 WITH comb_0_beh((ok, fail), opnd, env)
        SEND ((comb_0, fail), #eval, env) TO oper
    ]
    _ : [ SEND msg TO fail ]
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
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_comb: (#eval, %p)\n", env));
        Actor comb_0 = actor_new(behavior_new(act_comb_0, PR(cust, PR(opnd, env))));
        config_send(e->sponsor, oper, PR(PR(comb_0, fail), PR(s_eval, env)));
    } else {
        TRACE(fprintf(stderr, "act_comb: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}

/**
LET appl_0_beh((ok, fail), comb, env) = \args.[
    SEND ((ok, fail), #comb, args, env) TO comb
]
**/
static void
act_appl_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_appl_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), comb, env)
    Pair cust = p->h;
    p = p->t;
    Actor comb = p->h;
    Actor env = p->t;
    Actor args = e->message;  // (args)
    TRACE(fprintf(stderr, "act_appl_0: ok=%p, fail=%p, comb=%p, args=%p, env=%p\n", cust->h, cust->t, comb, args, env));
    config_send(e->sponsor, comb, PR(cust, PR(s_comb, PR(args, env))));
}
/**
LET appl_beh(comb) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#comb, opnd, env) : [
        CREATE appl_0 WITH appl_0_beh((ok, fail), comb, env)
        SEND ((appl_0, fail), #eval, env) TO opnd
    ]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
act_appl(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_appl{self=%p, msg=%p}\n", e->actor, e->message));
    Actor comb =  e->actor->behavior->context;  // (comb)
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    if (s_comb == p->h) {  // (#comb, opnd, env)
        p = p->t;
        Actor opnd = p->h;
        Actor env = p->t;
        Actor appl_0 = actor_new(behavior_new(act_appl_0, PR(cust, PR(comb, env))));
        TRACE(fprintf(stderr, "act_appl: ok=%p, fail=%p, appl_0=%p, comb=%p, opnd=%p, env=%p\n", cust->h, cust->t, appl_0, comb, opnd, env));
        config_send(e->sponsor, opnd, PR(PR(appl_0, fail), PR(s_eval, env)));
    } else {
        act_value(e);  // delegate
    }
}

/**
LET oper_0_beh((ok, fail), evar, env_d) = \env_p.[
    SEND ((ok, fail), #match, env_d, env_p) TO evar
]
**/
static void
act_oper_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_oper_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), evar, env_d)
    Pair cust = p->h;
    p = p->t;
    Actor evar = p->h;
    Actor env_d = p->t;
    Actor env_p = e->message;  // (env_p)
    TRACE(fprintf(stderr, "act_oper_0: ok=%p, fail=%p, evar=%p, env_p=%p, env_d=%p\n", cust->h, cust->t, evar, env_p, env_d));
    config_send(e->sponsor, evar, PR(cust, PR(s_match, PR(env_d, env_p))));
}
/**
LET oper_1_beh((ok, fail), body) = \env_e.[
    SEND ((ok, fail), #eval, env_e) TO body
]
**/
static void
act_oper_1(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_oper_1{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), body)
    Pair cust = p->h;
    Actor body = p->t;
    Actor env_e = e->message;  // (env_e)
    TRACE(fprintf(stderr, "act_oper_1: ok=%p, fail=%p, body=%p, env_e=%p\n", cust->h, cust->t, body, env_e));
    config_send(e->sponsor, body, PR(cust, PR(s_eval, env_e)));
}
/**
LET oper_beh(env_s, form, evar, body) = \msg.[
    LET ((ok, fail), req) = $msg IN
	CASE req OF
	(#comb, opnd, env_d) : [
        CREATE scope WITH scope_beh(dict_new(), env_s)
        SEND ((oper_0, fail), #match, opnd, scope) TO form
        CREATE oper_0 WITH \env_p.[  # (oper_1, evar, env_d)
            CASE evar OF
            $skip_ptrn : [  # optimize "lambda" case
                SEND ((ok, fail), #eval, env_p) TO body
            ]
            _ : [
                SEND ((oper_1, fail), #match, env_d, env_p) TO evar
                CREATE oper_1 WITH \env_e.[  # (cust, body)
                    SEND ((ok, fail), #eval, env_e) TO body
                ]
            ]
        ]
    ]
	_ : value_beh(msg)  # delegate
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
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    if (s_comb == p->h) {  // (#comb, opnd, env_d)
        p = p->t;
        Actor opnd = p->h;
        Actor env_d = p->t;
        Actor scope = actor_new(behavior_new(act_scope, PR(dict_new(), env_s)));
        TRACE(fprintf(stderr, "act_oper: opnd=%p, env_d=%p, scope=%p\n", opnd, env_d, scope));
        if (a_skip_ptrn == evar) {  // optimize "lambda" case
            Actor oper_1 = actor_new(behavior_new(act_oper_1, PR(cust, body)));
            TRACE(fprintf(stderr, "act_oper: ok=%p, fail=%p, oper_1=%p\n", ok, fail, oper_1));
            config_send(e->sponsor, form, PR(PR(oper_1, fail), PR(s_match, PR(opnd, scope))));
        } else {
            Actor oper_1 = actor_new(behavior_new(act_oper_1, PR(cust, body)));
            Actor oper_0 = actor_new(behavior_new(act_oper_0, PR(PR(oper_1, fail), PR(evar, env_d))));
            TRACE(fprintf(stderr, "act_oper: ok=%p, fail=%p, oper_0=%p, oper_1=%p\n", ok, fail, oper_0, oper_1));
            config_send(e->sponsor, form, PR(PR(oper_0, fail), PR(s_match, PR(opnd, scope))));
        }
    } else {
        act_value(e);  // delegate
    }
}

/**
LET vau_beh(form, evar, body) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [
        CREATE oper WITH oper_beh(env, form, evar, body)
        SEND oper TO ok
    ]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
act_vau(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_vau{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    p = p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_vau: (#eval, %p)\n", env));
        p =  e->actor->behavior->context;  // (form, evar, body)
        Actor oper = actor_new(behavior_new(act_oper, PR(env, p)));
        config_send(e->sponsor, ok, oper);
    } else {
        TRACE(fprintf(stderr, "act_vau: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}

/**
LET lambda_beh(form, body) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [
        CREATE oper WITH oper_beh(env, form, _, body)
        CREATE appl WITH appl_beh(oper)
        SEND appl TO ok
    ]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
act_lambda(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_lambda{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    p = p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_lambda: (#eval, %p)\n", env));
        p =  e->actor->behavior->context;  // (form, body)
        Actor form = p->h;
        Actor body = p->t;
        Actor oper = actor_new(behavior_new(act_oper, PR(env, PR(form, PR(a_skip_ptrn, body)))));
        Actor appl = actor_new(behavior_new(act_appl, oper));
        config_send(e->sponsor, ok, appl);
    } else {
        TRACE(fprintf(stderr, "act_lambda: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}

/**
CREATE empty WITH value_beh()
**/
static BEHAVIOR empty_behavior = { act_value, NIL };
ACTOR empty_actor = { &empty_behavior };
/**
LET eq_ptrn_beh(value) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#match, $value, env) : [ SEND env TO ok ]
    (#match, _) : [ SEND msg TO fail ]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
act_eq_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_eq_ptrn{self=%p, msg=%p}\n", e->actor, e->message));
    Any value = e->actor->behavior->context;  // (value)
    p = e->message;  // ((ok, fail), req)
    Actor ok = ((Pair)p->h)->h;
    Actor fail = ((Pair)p->h)->t;
    TRACE(fprintf(stderr, "act_eq_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_match == p->h) {  // (#match, ...)
        p = p->t;
        if (value == p->h) {  // (#match, $value, env)
            Actor env = p->t;
            TRACE(fprintf(stderr, "act_eq_ptrn: (#match, %p, %p)\n", value, env));
            config_send(e->sponsor, ok, env);
        } else {  // (#match, _)
            TRACE(fprintf(stderr, "act_eq_ptrn: (#match, _) -> FAIL!\n"));
            config_send(e->sponsor, fail, e->message);
        }
    } else {
        act_value(e);  // delegate
    }
}
/**
CREATE empty_ptrn WITH eq_ptrn_beh(empty)
**/
static BEHAVIOR empty_ptrn_behavior = { act_eq_ptrn, a_empty };
ACTOR empty_ptrn_actor = { &empty_ptrn_behavior };

/**
LET (pair_beh, pair_ptrn_beh) = $(
	LET brand = NEW value_beh() IN
**/
static BEHAVIOR pair_brand_behavior = { act_value, NIL };
static ACTOR pair_brand_actor = { &pair_brand_behavior };
/**
    LET pair_0_beh((ok, fail), t_ptrn, tail) = \env_0.[
        SEND ((ok, fail), #match, tail, env_0) TO t_ptrn
    ] IN
**/
static void
act_pair_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_pair_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), t_ptrn, tail)
    Pair cust = p->h;
    p = p->t;
    Actor t_ptrn = p->h;
    Actor tail = p->t;
    Actor env_0 = e->message;  // (env_0)
    TRACE(fprintf(stderr, "act_pair_0: ok=%p, fail=%p, tail=%p, env_0=%p, tail_ptrn=%p\n", cust->h, cust->t, tail, env_0, t_ptrn));
    config_send(e->sponsor, t_ptrn, PR(cust, PR(s_match, PR(tail, env_0))));
}
/**
	LET factory(head, tail) = \msg.[
		LET ((ok, fail), req) = $msg IN
		CASE req OF
		($brand, h_ptrn, t_ptrn, env) : [
			CREATE pair_0 WITH \env_0.[
				SEND ((ok, fail), #match, tail, env_0) TO t_ptrn
			]
			SEND ((pair_0, fail), #match, head, env) TO h_ptrn
		]
		_ : value_beh(msg)  # delegate
		END
	] IN
**/
void
act_pair(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_pair{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (head, tail)
    Actor head = p->h;
    Actor tail = p->t;
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "act_pair: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (&pair_brand_actor == p->h) {  // ($brand, h_ptrn, t_ptrn, env)
        p = p->t;
        Actor h_ptrn = p->h;
        p = p->t;
        Actor t_ptrn = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_pair: ($brand, %p, %p, %p)\n", h_ptrn, t_ptrn, env));
        Actor pair_0 = actor_new(behavior_new(act_pair_0, PR(cust, PR(t_ptrn, tail))));
        config_send(e->sponsor, h_ptrn, PR(PR(pair_0, fail), PR(s_match, PR(head, env))));
    } else {
        act_value(e);  // delegate
    }
}
/**
	LET pattern(h_ptrn, t_ptrn) = \msg.[
		LET ((ok, fail), req) = $msg IN
		CASE req OF
		(#match, value, env) : [
			SEND ((ok, fail), $brand, h_ptrn, t_ptrn, env) TO value
		]
		_ : value_beh(msg)  # delegate
		END
	] IN
	(factory, pattern)
)
**/
void
act_pair_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_pair_ptrn{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (h_ptrn, t_ptrn)
    Actor h_ptrn = p->h;
    Actor t_ptrn = p->t;
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "act_pair_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = p->t;
    if (s_match == p->h) {  // (#match, value, env)
        p = p->t;
        Actor value = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_pair_ptrn: (#match, %p, %p)\n", value, env));
        config_send(e->sponsor, value, PR(cust, PR(&pair_brand_actor, PR(h_ptrn, PR(t_ptrn, env)))));
    } else {
        act_value(e);  // delegate
    }
}

/**
LET seq_0_beh((ok, fail), tail_expr, env) = \_.[  # return tail value
	SEND ((ok, fail), #eval, env) TO tail_expr
]
**/
static void
act_seq_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_seq_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), tail_expr, env)
    Pair cust = p->h;
    p = p->t;
    Actor tail_expr = p->h;
    Actor env = p->t;
    TRACE(fprintf(stderr, "act_seq_0: ok=%p, fail=%p, tail_expr=%p, env=%p\n", cust->h, cust->t, tail_expr, env));
    config_send(e->sponsor, tail_expr, PR(cust, PR(s_eval, env)));
}
/**
LET seq_expr_beh(head_expr, tail_expr) = \msg.[  # sequential evaluation
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [
		CREATE seq_0 WITH seq_0_beh((ok, fail), tail_expr, env)
		SEND ((seq_0, fail), #eval, env) TO head_expr
	]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
act_seq_expr(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_seq_expr{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // (head_expr, tail_expr)
    Actor head_expr = p->h;
    Actor tail_expr = p->t;
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_seq_expr: (#eval, %p)\n", env));
        Actor seq_0 = actor_new(behavior_new(act_seq_0, PR(cust, PR(tail_expr, env))));
        config_send(e->sponsor, head_expr, PR(PR(seq_0, fail), PR(s_eval, env)));
    } else {
        TRACE(fprintf(stderr, "act_seq_expr: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}

/**
LET both_0_beh((ok, fail), head, k_tail) = \msg.[
	CASE msg OF
	($k_tail, tail) : [ SEND NEW pair_beh(head, tail) TO ok ]
	_ : [ SEND msg TO fail ]
	END
]
**/
static void
act_both_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_both_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), head, k_tail)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    Actor head = p->h;
    Actor k_tail = p->t;
    p = e->message;
    if (p->h == k_tail) {  // ($k_tail, tail)
        Any tail = p->t;
        Actor pair = actor_new(behavior_new(act_pair, PR(head, tail)));
        config_send(e->sponsor, ok, pair);
    } else {
        TRACE(fprintf(stderr, "act_both_0: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}
/**
LET both_1_beh((ok, fail), k_head, tail) = \msg.[
	CASE msg OF
	($k_head, head) : [ SEND NEW pair_beh(head, tail) TO ok ]
	_ : [ SEND msg TO fail ]
	END
]
**/
static void
act_both_1(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_both_1{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), k_head, tail)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    Actor k_head = p->h;
    Actor tail = p->t;
    p = e->message;
    if (p->h == k_head) {  // ($k_head, head)
        Any head = p->t;
        Actor pair = actor_new(behavior_new(act_pair, PR(head, tail)));
        config_send(e->sponsor, ok, pair);
    } else {
        TRACE(fprintf(stderr, "act_both_1: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}
/**
LET both_beh((ok, fail), k_head, k_tail) = \msg.[
	CASE msg OF
	($k_head, head) : [
		BECOME both_0_beh((ok, fail), head, k_tail)
	]
	($k_tail, tail) : [
		BECOME both_1_beh((ok, fail), k_head, tail)
	]
	_ : [ SEND msg TO fail ]
	END
]
**/
static void
act_both(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_both{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), k_head, k_tail)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    Actor k_head = p->h;
    Actor k_tail = p->t;
    p = e->message;
    if (p->h == k_head) {  // ($k_head, head)
        Any head = p->t;
        actor_become(e->actor, behavior_new(act_both_0, PR(cust, PR(head, k_tail))));
    } else if (p->h == k_tail) {  // ($k_tail, tail)
        Any tail = p->t;
        actor_become(e->actor, behavior_new(act_both_1, PR(cust, PR(k_head, tail))));
    } else {
        TRACE(fprintf(stderr, "act_both: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
    }
}
/**
LET par_0_beh((ok, fail), head_expr, tail_expr) = \req.[
    CREATE k_head WITH tag_beh(SELF)
    CREATE k_tail WITH tag_beh(SELF)
    SEND ((k_head, fail), req) TO head_expr
    SEND ((k_tail, fail), req) TO tail_expr
    BECOME both_beh((ok, fail), k_head, k_tail)  # return pair of values
]
**/
void
act_par_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_par_0{self=%p, msg=%p}\n", e->actor, e->message));
    p = e->actor->behavior->context;  // ((ok, fail), head_expr, tail_expr)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    Actor head_expr = p->h;
    Actor tail_expr = p->t;
    Any req = e->message;  // (#eval, env)
    Actor k_head = actor_new(behavior_new(act_tag, e->actor));
    Actor k_tail = actor_new(behavior_new(act_tag, e->actor));
    config_send(e->sponsor, head_expr, PR(PR(k_head, fail), req));
    config_send(e->sponsor, tail_expr, PR(PR(k_tail, fail), req));
    actor_become(e->actor, behavior_new(act_both, PR(cust, PR(k_head, k_tail))));
}
/**
LET par_expr_beh(head_expr, tail_expr) = \msg.[  # parallel evaluation
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [
        CREATE par_0 WITH par_0_beh((ok, fail), head_expr, tail_expr)
        SEND req TO par_0
	]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
act_par_expr(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "act_par_expr{self=%p, msg=%p}\n", e->actor, e->message));
    Pair exprs = e->actor->behavior->context;  // (head_expr, tail_expr)
    p = e->message;  // ((ok, fail), req)
    Pair cust = p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "act_par_expr: (#eval, %p)\n", env));
        Actor par_0 = actor_new(behavior_new(act_par_0, PR(cust, exprs)));
        config_send(e->sponsor, par_0, p);
    } else {
        TRACE(fprintf(stderr, "act_par_expr: FAIL!\n"));
        config_send(e->sponsor, fail, e->message);
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
    Actor form;
    Actor body;
    Actor comb;
    Actor expr;
    Actor test;

    TRACE(fprintf(stderr, "---- test_universe ----\n"));
    Config cfg = config_new();
    TRACE(fprintf(stderr, "cfg = %p\n", cfg));
    universe_init(cfg);

    // (\x.x)(#t) -> #t
    Actor s_x = symbol_intern("x");
    TRACE(fprintf(stderr, "s_x = %p\n", s_x));
    form = actor_new(behavior_new(act_bind_ptrn, s_x));
    body = s_x;
    Actor I = actor_new(behavior_new(act_lambda, PR(form, body)));
    TRACE(fprintf(stderr, "I = %p\n", I));
    comb = actor_new(behavior_new(act_comb, PR(I, b_true)));
    TRACE(fprintf(stderr, "comb = %p\n", comb));
    test = actor_new(behavior_new(act_expect, b_true));
    TRACE(fprintf(stderr, "test = %p\n", test));
    config_send(cfg, comb, PR(PR(test, a_fail), PR(s_eval, a_empty_env)));
    while (config_dispatch(cfg))
        ;

    // (\(x,y).seq(y,x))(#t,#f) -> #t
    Actor s_y = symbol_intern("y");
    TRACE(fprintf(stderr, "s_y = %p\n", s_y));
    form = actor_new(behavior_new(act_pair_ptrn, PR(
        actor_new(behavior_new(act_bind_ptrn, s_x)), //a_skip_ptrn,
        actor_new(behavior_new(act_bind_ptrn, s_y)))));
//    body = s_y;
//    body = s_x;
//    body = b_true;
//    body = b_false;
//    body = actor_new(behavior_new(act_seq_expr, PR(s_x, s_y)));
    body = actor_new(behavior_new(act_seq_expr, PR(s_y, s_x)));
//    body = actor_new(behavior_new(act_seq_expr, PR(b_true, b_false)));
//    body = actor_new(behavior_new(act_seq_expr, PR(b_false, b_true)));
    expr = actor_new(behavior_new(act_par_expr, PR(b_true, b_false)));
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    comb = actor_new(behavior_new(act_comb, PR(
        actor_new(behavior_new(act_lambda, PR(form, body))), 
        expr)));
    TRACE(fprintf(stderr, "comb = %p\n", comb));
//    test = actor_new(behavior_new(act_expect, b_false));
    test = actor_new(behavior_new(act_expect, b_true));
    TRACE(fprintf(stderr, "test = %p\n", test));
    config_send(cfg, comb, PR(PR(test, a_fail), PR(s_eval, a_empty_env)));
    while (config_dispatch(cfg))
        ;
}
