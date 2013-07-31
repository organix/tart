/*

expr.c -- Tiny Actor Run-Time

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

#include "expr.h"

void
val_request(Event e)
{
    TRACE(fprintf(stderr, "val_request{event=%p}\n", e));
    Request r = (Request)SELF(e);
    TRACE(fprintf(stderr, "val_request: ok=%p, fail=%p, req=%p\n", r->ok, r->fail, r->req));
    beh_halt(e);
}
static Actor
request_new(Actor ok, Actor fail, Actor req)
{
    Request r = NEW(REQUEST);
    BEH(r) = val_request;
    r->ok = ok;
    r->fail = fail;
    r->req = req;
    return (Actor)r;
}

void
val_req_eval(Event e)
{
    TRACE(fprintf(stderr, "val_req_eval{event=%p}\n", e));
    ReqEval req = (ReqEval)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_eval: env=%p\n", req->env));
    beh_halt(e);
}
Actor
req_eval_new(Actor ok, Actor fail, Actor env)
{
    ReqEval req = NEW(REQ_EVAL);
    BEH(req) = val_req_eval;
    req->env = env;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_match(Event e)
{
    TRACE(fprintf(stderr, "val_req_match{event=%p}\n", e));
    ReqMatch req = (ReqMatch)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_match: value=&p, env=%p\n", req->value, req->env));
    beh_halt(e);
}
Actor
req_match_new(Actor ok, Actor fail, Actor value, Actor env)
{
    ReqMatch req = NEW(REQ_MATCH);
    BEH(req) = val_req_match;
    req->value = value;
    req->env = env;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_bind(Event e)
{
    TRACE(fprintf(stderr, "val_req_bind{event=%p}\n", e));
    ReqBind req = (ReqBind)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_bind: key=%p, value=%p\n", req->key, req->value));
    beh_halt(e);
}
Actor
req_bind_new(Actor ok, Actor fail, Actor key, Actor value)
{
    ReqBind req = NEW(REQ_BIND);
    BEH(req) = val_req_bind;
    req->key = key;
    req->value = value;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_lookup(Event e)
{
    TRACE(fprintf(stderr, "val_req_lookup{event=%p}\n", e));
    ReqLookup req = (ReqLookup)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_lookup: key=%p\n", req->key));
    beh_halt(e);
}
Actor
req_lookup_new(Actor ok, Actor fail, Actor key)
{
    ReqLookup req = NEW(REQ_LOOKUP);
    BEH(req) = val_req_lookup;
    req->key = key;
    return request_new(ok, fail, (Actor)req);
}

void
val_req_combine(Event e)
{
    TRACE(fprintf(stderr, "val_req_combine{event=%p}\n", e));
    ReqCombine req = (ReqCombine)REQ(SELF(e));
    TRACE(fprintf(stderr, "val_req_combine: opnd=%p, env=%p\n", req->opnd, req->env));
    beh_halt(e);
}
Actor
req_combine_new(Actor ok, Actor fail, Actor opnd, Actor env)
{
    ReqCombine req = NEW(REQ_COMBINE);
    BEH(req) = val_req_combine;
    req->opnd = opnd;
    req->env = env;
    return request_new(ok, fail, (Actor)req);
}

/**
LET value_beh = \msg.[
    LET ((ok, fail), req) = $msg IN
    CASE req OF
    (#eval, _) : [ SEND SELF TO ok ]
    _ : [ SEND (SELF, msg) TO fail ]
    END
]
**/
void
expr_value(Event e)
{
    TRACE(fprintf(stderr, "expr_value{self=%p, msg=%p}\n", SELF(e), MSG(e)));
    if (val_request != BEH(MSG(e))) { halt("expr_value: request msg required"); }
    Request r = (Request)MSG(e);
    TRACE(fprintf(stderr, "expr_value: ok=%p, fail=%p\n", r->ok, r->fail));
    if (val_req_eval == BEH(r->req)) {  // (#eval, _)
        TRACE(fprintf(stderr, "expr_value: (#eval, _)\n"));
        config_send(e->sponsor, r->ok, SELF(e));
    } else {
        TRACE(fprintf(stderr, "expr_value: FAIL!\n"));
        config_send(e->sponsor, r->fail, e);
    }
}
