/*

expr.h -- Tiny Actor Run-Time

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
#ifndef _EXPR_H_
#define _EXPR_H_

#include "tart.h"
#include "actor.h"

/**
Request [*|*|*|*]
         | | | +------> [*|*|*] ReqCall
         | | +-> fail    | | +--> parameter
         | +-> ok        | +--> selector
         V               V
      val_request     val_req_call
**/

typedef struct request REQUEST, *Request;
typedef struct req_eval REQ_EVAL, *ReqEval;
typedef struct req_match REQ_MATCH, *ReqMatch;
typedef struct req_bind REQ_BIND, *ReqBind;
typedef struct req_lookup REQ_LOOKUP, *ReqLookup;
typedef struct req_combine REQ_COMBINE, *ReqCombine;
typedef struct req_read REQ_READ, *ReqRead;
typedef struct req_write REQ_WRITE, *ReqWrite;
typedef struct req_call REQ_CALL, *ReqCall;

#define REQ(r)      (((Request)(r))->req)

#define a_ptrn_skip ((Actor)(&the_ptrn_skip_actor))

struct request {
    ACTOR       _act;
    Actor       ok;         // "success" customer
    Actor       fail;       // "failure" customer
    Actor       req;        // request parameters
};
extern void     val_request(Event e);
extern Actor    request_new(Actor ok, Actor fail, Actor req);

struct req_eval {
    ACTOR       _act;
    Actor       env;        // evaluation environment
};
extern void     val_req_eval(Event e);
extern Actor    req_eval_new(Actor ok, Actor fail, Actor env);

struct req_match {
    ACTOR       _act;
    Actor       value;      // value to match
    Actor       env;        // binding environment
};
extern void     val_req_match(Event e);
extern Actor    req_match_new(Actor ok, Actor fail, Actor value, Actor env);

struct req_bind {
    ACTOR       _act;
    Actor       key;        // key used to lookup
    Actor       value;      // value bound to key
};
extern void     val_req_bind(Event e);
extern Actor    req_bind_new(Actor ok, Actor fail, Actor key, Actor value);

struct req_lookup {
    ACTOR       _act;
    Actor       key;        // key used to lookup
};
extern void     val_req_lookup(Event e);
extern Actor    req_lookup_new(Actor ok, Actor fail, Actor key);

struct req_combine {
    ACTOR       _act;
    Actor       opnd;       // operand value
    Actor       env;        // dynamic environment
};
extern void     val_req_combine(Event e);
extern Actor    req_combine_new(Actor ok, Actor fail, Actor opnd, Actor env);

struct req_read {
    ACTOR       _act;
};
extern void     val_req_read(Event e);
extern Actor    req_read_new(Actor ok, Actor fail);

struct req_write {
    ACTOR       _act;
    Actor       value;      // value to transmit
};
extern void     val_req_write(Event e);
extern Actor    req_write_new(Actor ok, Actor fail, Actor value);

struct req_call {
    ACTOR       _act;
    Actor       selector;   // request identification
    Actor       parameter;  // parameter data
};
extern void     val_req_call(Event e);
extern Actor    req_call_new(Actor ok, Actor fail, Actor selector, Actor parameter);

extern void     expr_value(Event e);
extern void     expr_env_empty(Event e);
extern void     expr_env(Event e);
extern void     ptrn_bind(Event e);
extern void     expr_name(Event e);

extern void     test_expr();  // unit-test method

extern ACTOR the_skip_ptrn_actor;

#endif /* _EXPR_H_ */
