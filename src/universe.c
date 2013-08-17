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
CREATE fail WITH \msg.[
    THROW (#FAIL!, msg)
]
**/
static inline void
beh_fail(Event e)
{
    TRACE(fprintf(stderr, "beh_fail{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    halt("FAIL!");
}
ACTOR fail_actor = { beh_fail };

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
beh_empty_env(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_empty_env{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "beh_empty_env: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_lookup == p->h) {  // (#lookup, _)
        TRACE(fprintf(stderr, "beh_empty_env: (#lookup, _) -> FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
    } else {
        beh_value(e);
    }
}
ACTOR empty_env_actor = { beh_empty_env };

/**
LET value_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
beh_value(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_value{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "beh_value: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, _)
        TRACE(fprintf(stderr, "beh_value: (#eval, _)\n"));
        config_send(SPONSOR(e), ok, SELF(e));
    } else {
        TRACE(fprintf(stderr, "beh_value: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
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
ser_scope(Event e)  // SERIALIZED
{
    Pair p;

    TRACE(fprintf(stderr, "ser_scope{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)STATE(SELF(e));  // (dict, parent)
    Actor dict = p->h;
    Actor parent = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "ser_scope: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_lookup == p->h) {  // (#lookup, name)
        Actor name = p->t;
        Any value = dict_lookup(dict, name);
        TRACE(fprintf(stderr, "ser_scope: (#lookup, %p) -> %p\n", name, value));
        if (value == NULL) {
            config_send(SPONSOR(e), parent, MSG(e));
        } else {
            config_send(SPONSOR(e), ok, value);
        }
    } else if (s_bind == p->h) {  // (#bind, name, value)
        p = (Pair)p->t;
        Actor name = p->h;
        Any value = p->t;
        TRACE(fprintf(stderr, "ser_scope: (#bind, %p, %p)\n", name, value));
        dict = dict_bind(dict, name, value);
//        actor_become(SELF(e), value_new(ser_scope, PR(dict, parent)));  -- see next two lines for equivalent
        p = (Pair)STATE(SELF(e));  // (dict, parent)
        p->h = dict;  // WARNING! this directly manipulates the data in this behavior
        config_send(SPONSOR(e), ok, SELF(e));
    } else {
        beh_value(e);  // delegate
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
beh_skip_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_skip_ptrn{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "beh_skip_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_match == p->h) {  // (#match, _, env)
        p = (Pair)p->t;
        Actor env = p->t;
        TRACE(fprintf(stderr, "beh_skip_ptrn: (#match, _, %p)\n", env));
        config_send(SPONSOR(e), ok, env);
    } else {
        beh_value(e);  // delegate
    }
}
/**
CREATE skip_ptrn WITH skip_ptrn_beh
**/
ACTOR skip_ptrn_actor = { beh_skip_ptrn };

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
val_bind_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_bind_ptrn{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor name = DATA(SELF(e));  // (name)
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "val_bind_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_match == p->h) {  // (#match, value, env)
        p = (Pair)p->t;
        Actor value = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_bind_ptrn: (#match, %p, %p)\n", value, env));
        config_send(SPONSOR(e), env, PR(PR(ok, fail), PR(s_bind, PR(name, value))));
    } else {
        beh_value(e);  // delegate
    }
}

/**
LET name_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, env) : [ SEND ((ok, fail), #lookup, SELF) TO env ]
    _ : [ SEND msg TO fail ]
    END
]
**/
void
beh_name(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_name{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "beh_name: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "beh_name: (#eval, %p)\n", env));
        config_send(SPONSOR(e), env, PR((Actor)cust, PR(s_lookup, SELF(e))));
    } else {
        TRACE(fprintf(stderr, "beh_name: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
    }
}

/**
LET comb_0_beh((ok, fail), opnd, env) = \comb.[
    SEND ((ok, fail), #comb, opnd, env) TO comb
]
**/
static void
val_comb_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_comb_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), opnd, env)
    Pair cust = (Pair)p->h;
    p = (Pair)p->t;
    Actor opnd = p->h;
    Actor env = p->t;
    Actor comb = MSG(e);  // (comb)
    TRACE(fprintf(stderr, "val_comb_0: ok=%p, fail=%p, comb=%p, opnd=%p, env=%p\n", cust->h, cust->t, comb, opnd, env));
    config_send(SPONSOR(e), comb, PR((Actor)cust, PR(s_comb, PR(opnd, env))));
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
val_comb(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_comb{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // (oper, opnd)
    Actor oper = p->h;
    Actor opnd = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_comb: (#eval, %p)\n", env));
        Actor comb_0 = value_new(val_comb_0, PR((Actor)cust, PR(opnd, env)));
        config_send(SPONSOR(e), oper, PR(PR(comb_0, fail), PR(s_eval, env)));
    } else {
        TRACE(fprintf(stderr, "val_comb: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
    }
}

/**
LET appl_0_beh((ok, fail), comb, env) = \args.[
    SEND ((ok, fail), #comb, args, env) TO comb
]
**/
static void
val_appl_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_appl_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), comb, env)
    Pair cust = (Pair)p->h;
    p = (Pair)p->t;
    Actor comb = p->h;
    Actor env = p->t;
    Actor args = MSG(e);  // (args)
    TRACE(fprintf(stderr, "val_appl_0: ok=%p, fail=%p, comb=%p, args=%p, env=%p\n", cust->h, cust->t, comb, args, env));
    config_send(SPONSOR(e), comb, PR((Actor)cust, PR(s_comb, PR(args, env))));
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
val_appl(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_appl{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor comb = DATA(SELF(e));  // (comb)
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_comb == p->h) {  // (#comb, opnd, env)
        p = (Pair)p->t;
        Actor opnd = p->h;
        Actor env = p->t;
        Actor appl_0 = value_new(val_appl_0, PR((Actor)cust, PR(comb, env)));
        TRACE(fprintf(stderr, "val_appl: ok=%p, fail=%p, appl_0=%p, comb=%p, opnd=%p, env=%p\n", cust->h, cust->t, appl_0, comb, opnd, env));
        config_send(SPONSOR(e), opnd, PR(PR(appl_0, fail), PR(s_eval, env)));
    } else {
        beh_value(e);  // delegate
    }
}

/**
LET oper_0_beh((ok, fail), evar, env_d) = \env_p.[
    SEND ((ok, fail), #match, env_d, env_p) TO evar
]
**/
static void
val_oper_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_oper_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), evar, env_d)
    Pair cust = (Pair)p->h;
    p = (Pair)p->t;
    Actor evar = p->h;
    Actor env_d = p->t;
    Actor env_p = MSG(e);  // (env_p)
    TRACE(fprintf(stderr, "val_oper_0: ok=%p, fail=%p, evar=%p, env_p=%p, env_d=%p\n", cust->h, cust->t, evar, env_p, env_d));
    config_send(SPONSOR(e), evar, PR((Actor)cust, PR(s_match, PR(env_d, env_p))));
}
/**
LET oper_1_beh((ok, fail), body) = \env_e.[
    SEND ((ok, fail), #eval, env_e) TO body
]
**/
static void
val_oper_1(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_oper_1{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), body)
    Pair cust = (Pair)p->h;
    Actor body = p->t;
    Actor env_e = MSG(e);  // (env_e)
    TRACE(fprintf(stderr, "val_oper_1: ok=%p, fail=%p, body=%p, env_e=%p\n", cust->h, cust->t, body, env_e));
    config_send(SPONSOR(e), body, PR((Actor)cust, PR(s_eval, env_e)));
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
val_oper(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_oper{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // (env_s, form, evar, body)
    Actor env_s = p->h;
    p = (Pair)p->t;
    Actor form = p->h;
    p = (Pair)p->t;
    Actor evar = p->h;
    Actor body = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_comb == p->h) {  // (#comb, opnd, env_d)
        p = (Pair)p->t;
        Actor opnd = p->h;
        Actor env_d = p->t;
        Actor scope = serial_new(ser_scope, PR(dict_new(), env_s));
        TRACE(fprintf(stderr, "val_oper: opnd=%p, env_d=%p, scope=%p\n", opnd, env_d, scope));
        if (a_skip_ptrn == evar) {  // optimize "lambda" case
            Actor oper_1 = value_new(val_oper_1, PR((Actor)cust, body));
            TRACE(fprintf(stderr, "val_oper: ok=%p, fail=%p, oper_1=%p\n", ok, fail, oper_1));
            config_send(SPONSOR(e), form, PR(PR(oper_1, fail), PR(s_match, PR(opnd, scope))));
        } else {
            Actor oper_1 = value_new(val_oper_1, PR((Actor)cust, body));
            Actor oper_0 = value_new(val_oper_0, PR(PR(oper_1, fail), PR(evar, env_d)));
            TRACE(fprintf(stderr, "val_oper: ok=%p, fail=%p, oper_0=%p, oper_1=%p\n", ok, fail, oper_0, oper_1));
            config_send(SPONSOR(e), form, PR(PR(oper_0, fail), PR(s_match, PR(opnd, scope))));
        }
    } else {
        beh_value(e);  // delegate
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
val_vau(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_vau{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_vau: (#eval, %p)\n", env));
        p = (Pair)DATA(SELF(e));  // (form, evar, body)
        Actor oper = value_new(val_oper, PR(env, (Actor)p));
        config_send(SPONSOR(e), ok, oper);
    } else {
        TRACE(fprintf(stderr, "val_vau: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
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
val_lambda(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_lambda{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_lambda: (#eval, %p)\n", env));
        p = (Pair)DATA(SELF(e));  // (form, body)
        Actor form = p->h;
        Actor body = p->t;
        Actor oper = value_new(val_oper, PR(env, PR(form, PR(a_skip_ptrn, body))));
        Actor appl = value_new(val_appl, oper);
        config_send(SPONSOR(e), ok, appl);
    } else {
        TRACE(fprintf(stderr, "val_lambda: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
    }
}

/**
CREATE s_x WITH name_beh
CREATE s_e WITH name_beh
CREATE oper_eval_form WITH pair_ptrn_beh(
	NEW bind_ptrn_beh(s_x), 
	NEW bind_ptrn_beh(s_e))
LET oper_eval_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
	CASE req OF
	(#comb, opnd, env_d) : [
        CREATE scope WITH scope_beh(dict_new(), env_d)  # NOTE: env_d not used
        SEND ((oper_0, fail), #match, opnd, scope) TO oper_eval_form
        CREATE oper_0 WITH \env_p.[
			SEND ((oper_1, fail), #lookup, s_x) TO env_p;
			CREATE oper_1 WITH \x.[
				SEND ((oper_2, fail), #lookup, s_e) TO env_p;
				CREATE oper_2 WITH \e.[
                    SEND ((ok, fail), #eval, e) TO x
				]
			]
        ]
    ]
	_ : value_beh(msg)  # delegate
	END
]
**/
static void
beh_oper_eval(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_oper_eval{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_comb == p->h) {  // (#comb, opnd, _)
        p = (Pair)p->t;
        Actor opnd = p->h;
        if (val_pair == CODE(opnd)) {
            // WARNING!  TIGHT-COUPLING TO THE IMPLEMENTATION OF ENCAPSULATED PAIRS
            p = (Pair)DATA(opnd);  // (head, tail)
            Actor expr = p->h;
            Actor env = p->t;
            TRACE(fprintf(stderr, "beh_oper_eval: ok=%p, fail=%p, expr=%p, env=%p\n", ok, fail, expr, env));
            config_send(SPONSOR(e), expr, PR((Actor)cust, PR(s_eval, env)));
        } else {
            TRACE(fprintf(stderr, "beh_oper_eval: FAIL! (non-pair arg)\n"));
            config_send(SPONSOR(e), fail, MSG(e));
        }
    } else {
        beh_value(e);  // delegate
    }
}
/**
CREATE oper_eval WITH oper_eval_beh
CREATE appl_eval WITH appl_beh(oper_eval);
**/
static ACTOR oper_eval_actor = { beh_oper_eval };
SERIAL appl_eval_actor = { { val_appl }, &oper_eval_actor };

/**
CREATE empty WITH value_beh
**/
ACTOR empty_actor = { beh_value };
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
val_eq_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_eq_ptrn{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor value = DATA(SELF(e));  // (value)
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "val_eq_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_match == p->h) {  // (#match, ...)
        p = (Pair)p->t;
        if (value == p->h) {  // (#match, $value, env)
            Actor env = p->t;
            TRACE(fprintf(stderr, "val_eq_ptrn: (#match, %p, %p)\n", value, env));
            config_send(SPONSOR(e), ok, env);
        } else {  // (#match, _)
            TRACE(fprintf(stderr, "val_eq_ptrn: (#match, _) -> FAIL!\n"));
            config_send(SPONSOR(e), fail, MSG(e));
        }
    } else {
        beh_value(e);  // delegate
    }
}
/**
CREATE empty_ptrn WITH eq_ptrn_beh(empty)
**/
VALUE empty_ptrn_actor = { { val_eq_ptrn }, a_empty };

/**
LET choice_ptrn_0_beh((ok, fail), value, env, ptrn) = \_.[
	CREATE scope WITH scope_beh(dict_new(), env)
	SEND ((ok, fail), #match, value, scope) TO ptrn
]
**/
static void
val_choice_ptrn_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_choice_ptrn_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), value, env, ptrn)
    Pair cust = (Pair)p->h;
    p = (Pair)p->t;
    Actor value = p->h;
    p = (Pair)p->t;
    Actor env = p->h;
    Actor ptrn = p->t;
    Actor scope = serial_new(ser_scope, PR(dict_new(), env));
    TRACE(fprintf(stderr, "val_choice_ptrn_0: ok=%p, fail=%p, value=%p, scope=%p, ptrn=%p\n", cust->h, cust->t, value, scope, ptrn));
    config_send(SPONSOR(e), ptrn, PR((Actor)cust, PR(s_match, PR(value, scope))));
}
/**
LET choice_ptrn_beh(h_ptrn, t_ptrn) = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#match, value, env) : [
		CREATE t_choice WITH choice_ptrn_0_beh((ok, fail), value, env, t_ptrn)
		CREATE h_choice WITH choice_ptrn_0_beh((ok, t_choice), value, env, h_ptrn)
		SEND () TO h_choice
	]
    _ : value_beh(msg)  # delegate
    END
]
**/
void
val_choice_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_choice_ptrn{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // (h_ptrn, t_ptrn)
    Actor h_ptrn = p->h;
    Actor t_ptrn = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "val_choice_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_match == p->h) {  // (#match, value, env)
        p = (Pair)p->t;
        Actor value = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_choice_ptrn: (#match, %p, %p)\n", value, env));
        Actor t_choice = value_new(val_choice_ptrn_0, PR((Actor)cust, PR(value, PR(env, t_ptrn))));
        Actor h_choice = value_new(val_choice_ptrn_0, PR(PR(ok, t_choice), PR(value, PR(env, h_ptrn))));
        config_send(SPONSOR(e), h_choice, a_fail);
    } else {
        beh_value(e);  // delegate
    }
}

/**
LET (pair_beh, pair_ptrn_beh) = $(
	LET brand = $(NEW value_beh) IN
**/
static ACTOR pair_brand_actor = { beh_value };
/**
    LET pair_0_beh((ok, fail), t_ptrn, tail) = \env_0.[
        SEND ((ok, fail), #match, tail, env_0) TO t_ptrn
    ] IN
**/
static void
val_pair_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_pair_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), t_ptrn, tail)
    Pair cust = (Pair)p->h;
    p = (Pair)p->t;
    Actor t_ptrn = p->h;
    Actor tail = p->t;
    Actor env_0 = MSG(e);  // (env_0)
    TRACE(fprintf(stderr, "val_pair_0: ok=%p, fail=%p, tail=%p, env_0=%p, tail_ptrn=%p\n", cust->h, cust->t, tail, env_0, t_ptrn));
    config_send(SPONSOR(e), t_ptrn, PR((Actor)cust, PR(s_match, PR(tail, env_0))));
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
val_pair(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_pair{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // (head, tail)
    Actor head = p->h;
    Actor tail = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "val_pair: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (&pair_brand_actor == p->h) {  // ($brand, h_ptrn, t_ptrn, env)
        p = (Pair)p->t;
        Actor h_ptrn = p->h;
        p = (Pair)p->t;
        Actor t_ptrn = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_pair: ($brand, %p, %p, %p)\n", h_ptrn, t_ptrn, env));
        Actor pair_0 = value_new(val_pair_0, PR((Actor)cust, PR(t_ptrn, tail)));
        config_send(SPONSOR(e), h_ptrn, PR(PR(pair_0, fail), PR(s_match, PR(head, env))));
    } else {
        beh_value(e);  // delegate
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
val_pair_ptrn(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_pair_ptrn{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // (h_ptrn, t_ptrn)
    Actor h_ptrn = p->h;
    Actor t_ptrn = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    TRACE(fprintf(stderr, "val_pair_ptrn: ok=%p, fail=%p\n", ok, fail));
    p = (Pair)p->t;
    if (s_match == p->h) {  // (#match, value, env)
        p = (Pair)p->t;
        Actor value = p->h;
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_pair_ptrn: (#match, %p, %p)\n", value, env));
        config_send(SPONSOR(e), value, PR((Actor)cust, PR(&pair_brand_actor, PR(h_ptrn, PR(t_ptrn, env)))));
    } else {
        beh_value(e);  // delegate
    }
}

/**
LET seq_0_beh((ok, fail), tail_expr, env) = \_.[  # return tail value
	SEND ((ok, fail), #eval, env) TO tail_expr
]
**/
static void
val_seq_0(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_seq_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // ((ok, fail), tail_expr, env)
    Pair cust = (Pair)p->h;
    p = (Pair)p->t;
    Actor tail_expr = p->h;
    Actor env = p->t;
    TRACE(fprintf(stderr, "val_seq_0: ok=%p, fail=%p, tail_expr=%p, env=%p\n", cust->h, cust->t, tail_expr, env));
    config_send(SPONSOR(e), tail_expr, PR((Actor)cust, PR(s_eval, env)));
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
val_seq_expr(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_seq_expr{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)DATA(SELF(e));  // (head_expr, tail_expr)
    Actor head_expr = p->h;
    Actor tail_expr = p->t;
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_seq_expr: (#eval, %p)\n", env));
        Actor seq_0 = value_new(val_seq_0, PR((Actor)cust, PR(tail_expr, env)));
        config_send(SPONSOR(e), head_expr, PR(PR(seq_0, fail), PR(s_eval, env)));
    } else {
        TRACE(fprintf(stderr, "val_seq_expr: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
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

    TRACE(fprintf(stderr, "act_both_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)STATE(SELF(e));  // ((ok, fail), head, k_tail)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    Actor head = p->h;
    Actor k_tail = p->t;
    p = (Pair)MSG(e);
    if (p->h == k_tail) {  // ($k_tail, tail)
        Actor tail = p->t;
        Actor pair = value_new(val_pair, PR(head, tail));
        config_send(SPONSOR(e), ok, pair);
    } else {
        TRACE(fprintf(stderr, "act_both_0: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
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

    TRACE(fprintf(stderr, "act_both_1{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)STATE(SELF(e));  // ((ok, fail), k_head, tail)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    Actor k_head = p->h;
    Actor tail = p->t;
    p = (Pair)MSG(e);
    if (p->h == k_head) {  // ($k_head, head)
        Actor head = p->t;
        Actor pair = value_new(val_pair, PR(head, tail));
        config_send(SPONSOR(e), ok, pair);
    } else {
        TRACE(fprintf(stderr, "act_both_1: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
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
act_both(Event e)  // SERIALIZED
{
    Pair p;

    TRACE(fprintf(stderr, "act_both{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)STATE(SELF(e));  // ((ok, fail), k_head, k_tail)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    Actor k_head = p->h;
    Actor k_tail = p->t;
    p = (Pair)MSG(e);
    if (p->h == k_head) {  // ($k_head, head)
        Actor head = p->t;
        actor_become(SELF(e), value_new(act_both_0, PR((Actor)cust, PR(head, k_tail))));
    } else if (p->h == k_tail) {  // ($k_tail, tail)
        Actor tail = p->t;
        actor_become(SELF(e), value_new(act_both_1, PR((Actor)cust, PR(k_head, tail))));
    } else {
        TRACE(fprintf(stderr, "act_both: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
    }
}

/**
LET tag_beh(cust) = \msg.[ SEND (SELF, msg) TO cust ]
**/
static void
val_tag(Event e)
{
    TRACE(fprintf(stderr, "val_tag{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor cust = DATA(SELF(e));  // cust
    Actor msg = MSG(e);  // msg
    config_send(SPONSOR(e), cust, PR(SELF(e), msg));
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
act_par_0(Event e)  // SERIALIZED
{
    Pair p;

    TRACE(fprintf(stderr, "act_par_0{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)STATE(SELF(e));  // ((ok, fail), head_expr, tail_expr)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    Actor head_expr = p->h;
    Actor tail_expr = p->t;
    Actor req = MSG(e);  // (#eval, env)
    Actor k_head = value_new(val_tag, SELF(e));
    Actor k_tail = value_new(val_tag, SELF(e));
    config_send(SPONSOR(e), head_expr, PR(PR(k_head, fail), req));
    config_send(SPONSOR(e), tail_expr, PR(PR(k_tail, fail), req));
    actor_become(SELF(e), value_new(act_both, PR((Actor)cust, PR(k_head, k_tail))));
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
val_par_expr(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "val_par_expr{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor exprs = DATA(SELF(e));  // (head_expr, tail_expr)
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_eval == p->h) {  // (#eval, env)
        Actor env = p->t;
        TRACE(fprintf(stderr, "val_par_expr: (#eval, %p)\n", env));
        Actor par_0 = serial_new(act_par_0, PR((Actor)cust, exprs));
        config_send(SPONSOR(e), par_0, (Actor)p);
    } else {
        TRACE(fprintf(stderr, "val_par_expr: FAIL!\n"));
        config_send(SPONSOR(e), fail, MSG(e));
    }
}

/**
LET oper_true_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
	CASE req OF
	(#comb, opnd, env) : [
        (expr, _) = $(opnd->behavior->context)
        SEND ((ok, fail), #eval, env) TO expr
    ]
	_ : value_beh(msg)  # delegate
	END
]
**/
static void
beh_oper_true(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_oper_true{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_comb == p->h) {  // (#comb, opnd, env)
        p = (Pair)p->t;
        Actor opnd = p->h;
        Actor env = p->t;
        if (val_pair == CODE(opnd)) {
            // WARNING!  TIGHT-COUPLING TO THE IMPLEMENTATION OF ENCAPSULATED PAIRS
            p = (Pair)DATA(opnd);  // (expr, _)
            Actor expr = p->h;
            TRACE(fprintf(stderr, "beh_oper_true: ok=%p, fail=%p, expr=%p, env=%p\n", ok, fail, expr, env));
            config_send(SPONSOR(e), expr, PR((Actor)cust, PR(s_eval, env)));
        } else {
            TRACE(fprintf(stderr, "beh_oper_true: FAIL! (non-pair arg)\n"));
            config_send(SPONSOR(e), fail, MSG(e));
        }
    } else {
        beh_value(e);  // delegate
    }
}
/**
LET oper_false_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
	CASE req OF
	(#comb, opnd, env) : [
        (_, expr) = $(opnd->behavior->context)
        SEND ((ok, fail), #eval, env) TO expr
    ]
	_ : value_beh(msg)  # delegate
	END
]
**/
static void
beh_oper_false(Event e)
{
    Pair p;

    TRACE(fprintf(stderr, "beh_oper_false{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    p = (Pair)MSG(e);  // ((ok, fail), req)
    Pair cust = (Pair)p->h;
    Actor ok = cust->h;
    Actor fail = cust->t;
    p = (Pair)p->t;
    if (s_comb == p->h) {  // (#comb, opnd, env)
        p = (Pair)p->t;
        Actor opnd = p->h;
        Actor env = p->t;
        if (val_pair == CODE(opnd)) {
            // WARNING!  TIGHT-COUPLING TO THE IMPLEMENTATION OF ENCAPSULATED PAIRS
            p = (Pair)DATA(opnd);  // (_, expr)
            Actor expr = p->t;
            TRACE(fprintf(stderr, "beh_oper_false: ok=%p, fail=%p, expr=%p, env=%p\n", ok, fail, expr, env));
            config_send(SPONSOR(e), expr, PR((Actor)cust, PR(s_eval, env)));
        } else {
            TRACE(fprintf(stderr, "beh_oper_false: FAIL! (non-pair arg)\n"));
            config_send(SPONSOR(e), fail, MSG(e));
        }
    } else {
        beh_value(e);  // delegate
    }
}

/*
 *  NOTE: These global references are initialized by universe_init().
 */
Actor b_true;
Actor b_false;
static Actor symbol_table = a_empty_dict;
Actor s_eval;
Actor s_match;
Actor s_lookup;
Actor s_bind;
Actor s_comb;

static Actor
symbol_intern(char * name)
{
    Actor a_symbol = dict_lookup(symbol_table, (Actor)name);  // [FIXME: 'name' IS NOT AN ACTOR!]
    if (a_symbol == NULL) {
        a_symbol = actor_new(beh_name);
        symbol_table = dict_bind(symbol_table, (Actor)name, a_symbol);
    }
    return a_symbol;
}

/*
0 = \(f, x).x
1 = \(f, x).f(x)
true = \(a, b).a
false = \(a, b).b

($define! #t ($vau (x) #ignore ($vau (y) e (eval x e))))  ; usage: ((#t cnsq) altn) ==> cnsq
($define! #f ($vau (x) #ignore ($vau (y) e (eval y e))))  ; usage: ((#f cnsq) altn) ==> altn

($define! #t ($vau (x . #ignore) e (eval x . e)))  ; usage: ((#t cnsq . altn) ==> cnsq
($define! #f ($vau (#ignore . x) e (eval x . e)))  ; usage: ((#f cnsq . altn) ==> altn
*/
static void
boolean_init()
{
    Actor s_e = actor_new(beh_name); //symbol_intern("e");
    Actor s_x = actor_new(beh_name); //symbol_intern("x");
    Actor T_form = value_new(val_pair_ptrn, PR(
        value_new(val_bind_ptrn, s_x),
        a_skip_ptrn));
    Actor evar = value_new(val_bind_ptrn, s_e);
    Actor body = value_new(val_comb, PR(
        a_appl_eval, 
        value_new(val_par_expr, PR(s_x, s_e))));
    b_true = value_new(val_oper, 
        PR(a_empty_env, PR(T_form, PR(evar, body))));
    Actor F_form = value_new(val_pair_ptrn, PR(
        a_skip_ptrn,
        value_new(val_bind_ptrn, s_x)));
    b_false = value_new(val_oper, 
        PR(a_empty_env, PR(F_form, PR(evar, body))));
}

void
universe_init(Config cfg)
{
    b_true = actor_new(beh_oper_true);
    b_false = actor_new(beh_oper_false);
//    boolean_init();
    TRACE(fprintf(stderr, "b_true = %p\n", b_true));
    TRACE(fprintf(stderr, "b_false = %p\n", b_false));
    s_comb = symbol_intern("comb");
    s_bind = symbol_intern("bind");
    s_lookup = symbol_intern("lookup");
    s_match = symbol_intern("match");
    s_eval = symbol_intern("eval");
}

static inline void
val_expect(Event e)
{
    TRACE(fprintf(stderr, "val_expect{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    Actor expect = DATA(SELF(e));
    Actor actual = MSG(e);
    if (expect != actual) {
        TRACE(fprintf(stderr, "val_expect: %p != %p\n", expect, actual));
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
    form = value_new(val_bind_ptrn, s_x);
    body = s_x;
    Actor I = value_new(val_lambda, PR(form, body));
    TRACE(fprintf(stderr, "I = %p\n", I));
    comb = value_new(val_comb, PR(I, b_true));
    TRACE(fprintf(stderr, "comb = %p\n", comb));
    test = value_new(val_expect, b_true);
    TRACE(fprintf(stderr, "test = %p\n", test));
    config_send(cfg, comb, PR(PR(test, a_fail), PR(s_eval, a_empty_env)));
    while (config_dispatch(cfg) == a_true)
        ;

    // (\(x,y).seq(y,x))(#t,#f) -> #t
    Actor s_y = symbol_intern("y");
    TRACE(fprintf(stderr, "s_y = %p\n", s_y));
    form = value_new(val_pair_ptrn, PR(
        value_new(val_bind_ptrn, s_x), //a_skip_ptrn,
        value_new(val_bind_ptrn, s_y)));
//    body = s_y;
//    body = s_x;
//    body = b_true;
//    body = b_false;
//    body = value_new(val_seq_expr, PR(s_x, s_y));
    body = value_new(val_seq_expr, PR(s_y, s_x));
//    body = value_new(val_seq_expr, PR(b_true, b_false));
//    body = value_new(val_seq_expr, PR(b_false, b_true));
    expr = value_new(val_par_expr, PR(b_true, b_false));
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    comb = value_new(val_comb, PR(
        value_new(val_lambda, PR(form, body)), 
        expr));
    TRACE(fprintf(stderr, "comb = %p\n", comb));
//    test = value_new(val_expect, b_false);
    test = value_new(val_expect, b_true);
    TRACE(fprintf(stderr, "test = %p\n", test));
    config_send(cfg, comb, PR(PR(test, a_fail), PR(s_eval, a_empty_env)));
    while (config_dispatch(cfg) == a_true)
        ;

    // (#t #f . #t) -> #f
    expr = value_new(val_pair, PR(b_false, b_true));
//    expr = value_new(val_pair, PR(b_false, a_fail));
//    expr = value_new(val_pair, PR(a_fail, b_true));
//    expr = value_new(val_pair, PR(b_false, s_y));
//    expr = value_new(val_pair, PR(s_x, b_true));
    TRACE(fprintf(stderr, "expr = %p\n", expr));
    comb = value_new(val_comb, PR(b_true, expr));
    TRACE(fprintf(stderr, "comb = %p\n", comb));
    test = value_new(val_expect, b_false);
    TRACE(fprintf(stderr, "test = %p\n", test));
    config_send(cfg, comb, PR(PR(test, a_fail), PR(s_eval, a_empty_env)));
    while (config_dispatch(cfg) == a_true)
        ;
}
