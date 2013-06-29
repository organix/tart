/*

tart.c -- Tiny Actor Run-Time

To compile:
    cc -o tart tart.c

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

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define TRACE(x)    x   /* enable/disable trace statements */
#define DEBUG(x)        /* enable/disable debug statements */

#define ALLOC(S)    (calloc((S), 1))
#define NEW(T)      ((T *)calloc(sizeof(T), 1))
#define FREE(p)     ((p) = (free(p), NULL))

struct actor;
struct message;
struct event;

typedef void (*ACTION)(struct event * e);

struct link {
    struct link *   next;       // singly-link structure
};

struct link nil = { &nil };     // list terminator (empty)
#define NIL (&nil)

struct list {
    struct link     link;
    void *          item;
};

#define EMPTY_LIST ((struct list *)NIL)

inline int
list_empty_p(struct list * list)
{
    return (list == EMPTY_LIST);
}

inline struct list *
list_pop(struct list * list)
{
    return list;
}

inline struct list *
list_push(struct list * list, void * item)
{
    struct list * result = NEW(struct list);
    result->item = item;
    result->link.next = (struct link *)list;
    return result;
}

struct queue {
    struct link *   head;
    struct link *   tail;
};

inline int
queue_empty_p(struct queue * q)
{
    return (q->head == NIL);
}

inline void
queue_give(struct queue * q, void * item)
{
    struct list * list = NEW(struct list);
    list->item = item;
    list->link.next = NIL;
    if (queue_empty_p(q)) {
        q->head = (struct link *)list;
    } else {
        q->tail->next = (struct link *)list;
    }
    q->tail = (struct link *)list;
}

inline void *
queue_take(struct queue * q)
{
    // if (queue_empty_p(q)) ERROR("can't take from empty queue");
    struct list * entry = (struct list *)q->head;
    void * item = entry->item;
    q->head = entry->link.next;
    entry = FREE(entry);
    return item;
}

struct config {
    struct queue    event_q;    // messages in-transit
    struct list *   actors;     // actors created
};

inline struct config *
config_new()
{
    struct config * cfg = NEW(struct config);
    cfg->event_q.head = NIL;
    cfg->event_q.tail = NIL;
    cfg->actors = EMPTY_LIST;
    return cfg;
}

struct actor {  // FIXME: refactor to extract "struct behavior" so "struct actor" becomes a single pointer
    ACTION          action;     // behavior
    void *          data;       // state
};

struct event {
    struct config * sponsor;    // sponsor configuration
    struct actor *  actor;      // target actor
    void *          message;    // message to deliver
};

struct effect {
    struct actor *  self;       // currently active actor
    struct list *   actors;     // actors created
    struct list *   events;     // messages sent
    ACTION          behavior;   // replacement behavior
    void *          data;       // replacement state
};

void
act_begin(struct event * e)
{
    TRACE(fprintf(stderr, "act_begin{self=%p, msg=%p}\n", e->actor, e->message));
    struct actor * cust = (struct actor *)e->actor->data;  // cust
    // initialize effects
    struct actor * self = e->actor;
    struct effect * fx = NEW(struct effect);
    fx->self = self;
    fx->actors = EMPTY_LIST;
    fx->events = EMPTY_LIST;
    fx->behavior = self->action;
    fx->data = self->data;
    // trigger continuation
    struct event * cont = NEW(struct event);
    cont->sponsor = e->sponsor;
    cont->actor = cust;
    cont->message = fx;
    queue_give(&(e->sponsor->event_q), cont);
}

void
act_send(struct event * e)
{
    TRACE(fprintf(stderr, "act_send{self=%p, msg=%p}\n", e->actor, e->message));
    struct list * list = (struct list *)e->actor->data;  // [cust, target, message]
    struct actor * cust = (struct actor *)list->item;
    list = (struct list *)list->link.next;
    struct actor * target = (struct actor *)list->item;
    list = (struct list *)list->link.next;
    void * message = list->item;
    // store new event in effects
    struct effect * fx = (struct effect *)e->message;
    struct event * event = NEW(struct event);
    event->sponsor = e->sponsor;
    event->actor = target;
    event->message = message;
    fx->events = list_push(fx->events, event);
    // trigger continuation
    struct event * cont = NEW(struct event);
    cont->sponsor = e->sponsor;
    cont->actor = cust;
    cont->message = e->message;
    queue_give(&(e->sponsor->event_q), cont);
}

void
act_create(struct event * e)
{
    TRACE(fprintf(stderr, "act_create{self=%p, msg=%p}\n", e->actor, e->message));
    struct list * list = (struct list *)e->actor->data;  // [cust, action, data]
    struct actor * cust = (struct actor *)list->item;
    list = (struct list *)list->link.next;
    ACTION beh = (ACTION)list->item;
    list = (struct list *)list->link.next;
    void * data = list->item;
    // store new actor in effects
    struct effect * fx = (struct effect *)e->message;
    struct actor * actor = NEW(struct actor);
    actor->action = beh;
    actor->data = data;
    fx->actors = list_push(fx->actors, actor);
    // trigger continuation
    struct event * cont = NEW(struct event);
    cont->sponsor = e->sponsor;
    cont->actor = cust;
    cont->message = e->message;
    queue_give(&(e->sponsor->event_q), cont);
}

void
act_become(struct event * e)
{
    TRACE(fprintf(stderr, "act_become{self=%p, msg=%p}\n", e->actor, e->message));
    struct list * list = (struct list *)e->actor->data;  // [cust, action, data]
    struct actor * cust = (struct actor *)list->item;
    list = (struct list *)list->link.next;
    ACTION beh = (ACTION)list->item;
    list = (struct list *)list->link.next;
    void * data = list->item;
    // store new behavior/state in effects
    struct effect * fx = (struct effect *)e->message;
    fx->behavior = beh;
    fx->data = data;
    // trigger continuation
    struct event * cont = NEW(struct event);
    cont->sponsor = e->sponsor;
    cont->actor = cust;
    cont->message = e->message;
    queue_give(&(e->sponsor->event_q), cont);
}

void
act_commit(struct event * e)
{
    struct list * list;
    struct list * entry;

    TRACE(fprintf(stderr, "act_commit{self=%p, msg=%p}\n", e->actor, e->message));
    struct effect * fx = (struct effect *)e->message;
    // update actor behavior
    fx->self->action = fx->behavior;
    fx->self->data = fx->data;
    // add new actors to configuration
    for (list = fx->actors; !list_empty_p(list); list = (struct list *)entry->link.next) {
        entry = list_pop(list);
        e->sponsor->actors = list_push(e->sponsor->actors, entry->item);
    }
    // add new events to dispatch queue
    for (list = fx->events; !list_empty_p(list); list = (struct list *)entry->link.next) {
        entry = list_pop(list);
        queue_give(&(e->sponsor->event_q), entry->item);
    }
}

int
config_dispatch(struct config * cfg)
{
    if (queue_empty_p(&(cfg->event_q))) {
        TRACE(fprintf(stderr, "config_dispatch: --EMPTY--\n"));
        return 0;
    }
    struct event * e = (struct event *)queue_take(&(cfg->event_q));
    TRACE(fprintf(stderr, "config_dispatch: actor=%p, event=%p\n", e->actor, e));
    (e->actor->action)(e);
    return 1;
}

/*
 *  Unit tests
 */
void
run_tests()
{
    struct actor * actor;
    struct list * list;
    struct link * next;

    TRACE(fprintf(stderr, "NIL = %p\n", NIL));

/*
CREATE sink WITH \_.[]
CREATE doit WITH \_.[ SEND [] TO sink ]
fwd_beh(target) = \msg.[ SEND msg TO target ]
oneshot_beh(target) = \msg.[
    SEND msg TO target
    BECOME sink_beh
]
*/
    struct actor * a_commit = NEW(struct actor);
    a_commit->action = act_commit;
    a_commit->data = NIL;
    TRACE(fprintf(stderr, "a_commit = %p\n", a_commit));
    struct actor * a_sink = NEW(struct actor);
    a_sink->action = act_begin;
    a_sink->data = a_commit;
    TRACE(fprintf(stderr, "a_sink = %p\n", a_sink));

    // [cust, target, message]
    next = NIL;
    list = NEW(struct list);
    list->link.next = next;
    list->item = NIL;
    next = (struct link *)list;
    list = NEW(struct list);
    list->link.next = next;
    list->item = a_sink;
    next = (struct link *)list;
    list = NEW(struct list);
    list->link.next = next;
    list->item = a_commit;
    next = (struct link *)list;
    actor = NEW(struct actor);
    actor->action = act_send;
    actor->data = list;

    struct actor * a_doit = NEW(struct actor);
    a_doit->action = act_begin;
    a_doit->data = actor;
    TRACE(fprintf(stderr, "a_doit = %p\n", a_doit));
    
    struct config * config = config_new();
    struct event * e = NEW(struct event);
    e->sponsor = config;
    e->actor = a_doit;
    e->message = NIL;
    queue_give(&(config->event_q), e);
    while (config_dispatch(config))
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
