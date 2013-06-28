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

/*
 * RE-ENVISION CORE MECHANISM AS SIMPLE EVENT QUEUING AND EXECUTION
 */

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

struct actor {
    ACTION          action;     // behavior
    void *          data;       // state
};

struct message {  // DEPRECATED -- MESSAGES NEED NOT BE LISTS, AND IF THEY WERE WE SHOULD USE LIST TYPE
    struct link     link;
    void *          data;       // data payload component
};

struct event {
    struct config * sponsor;    // sponsor configuration
    struct actor *  actor;      // target actor
    struct message *message;    // message to deliver
};

struct effect {
    struct list *   actors;     // actors created
    struct list *   events;     // messages sent
    ACTION          behavior;   // replacement behavior
};

void
act_send(struct event * e)
{
    struct list * list = (struct list *)e->actor->data;
    struct actor * cust = (struct actor *)list->item;
    list = (struct list *)list->link.next;
    struct actor * target = (struct actor *)list->item;
    list = (struct list *)list->link.next;
    struct message * message = (struct message *)list->item;
    // store the pending send in the event effects
    struct effect * fx = (struct effect *)e->message->data;
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
act_commit(struct event * e)
{
    struct list * list;
    struct list * entry;
    struct effect * fx = (struct effect *)e->message->data;
    // update actor behavior
    e->actor->action = fx->behavior;
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

/*
 *  Unit tests
 */
void
run_tests()
{
    TRACE(fprintf(stderr, "NIL = %p\n", NIL));
/*
    TRACE(fprintf(stderr, "o_undef = %p\n", o_undef));
    TRACE(fprintf(stderr, "o_fail = %p\n", o_fail));
    TRACE(fprintf(stderr, "o_nil = %p\n", o_nil));

    TRACE(fprintf(stderr, "o_true = %p\n", o_true));
    TRACE(fprintf(stderr, "o_false = %p\n", o_false));
    TRACE(fprintf(stderr, "s_eq_p = %p\n", s_eq_p));
    TRACE(fprintf(stderr, "&eq_p_symbol = %p\n", &eq_p_symbol));
    TRACE(fprintf(stderr, "symbol_kind = %p\n", (void*)symbol_kind));
    TRACE(fprintf(stderr, "eq_p_symbol.o.kind = %p\n", (void*)eq_p_symbol.o.kind));
    TRACE(fprintf(stderr, "eq_p_symbol.s = \"%s\"\n", eq_p_symbol.s));

    TRACE(fprintf(stderr, "dict_kind = %p\n", (void*)dict_kind));
    TRACE(fprintf(stderr, "s_lookup = %p\n", s_lookup));
    TRACE(fprintf(stderr, "s_bind = %p\n", s_bind));

    OOP s_x = symbol_new("x");
    TRACE(fprintf(stderr, "s_x = %p\n", s_x));
    TRACE(fprintf(stderr, "as_symbol(s_x)->s = \"%s\"\n", as_symbol(s_x)->s));
    OOP result = object_call(o_empty_dict, s_lookup, s_x);
    TRACE(fprintf(stderr, "result = %p\n", result));
    assert(o_fail == result);
    
    OOP n_42 = integer_new(42);
    TRACE(fprintf(stderr, "n_42 = %p\n", n_42));
    TRACE(fprintf(stderr, "as_integer(n_42)->n = %d\n", as_integer(n_42)->n));
    OOP d_env = object_call(o_empty_dict, s_bind, s_x, n_42);
    TRACE(fprintf(stderr, "d_env = %p\n", d_env));
    result = object_call(d_env, s_lookup, s_x);
    TRACE(fprintf(stderr, "result = %p\n", result));
    assert(n_42 == result);

    OOP s_y = symbol_new("y");
    TRACE(fprintf(stderr, "s_y = %p\n", s_y));
    TRACE(fprintf(stderr, "as_symbol(s_y)->s = \"%s\"\n", as_symbol(s_y)->s));
    d_env = object_call(d_env, s_bind, s_y, n_minus_1);
    TRACE(fprintf(stderr, "d_env = %p\n", d_env));

    OOP s_z = symbol_new("z");
    TRACE(fprintf(stderr, "s_z = %p\n", s_z));
    TRACE(fprintf(stderr, "as_symbol(s_z)->s = \"%s\"\n", as_symbol(s_z)->s));
    result = object_call(d_env, s_lookup, s_z);
    TRACE(fprintf(stderr, "result = %p\n", result));
    assert(o_fail == result);

    OOP ch_A = integer_new('A');
    OOP ch_Z = integer_new('Z');
    OOP q_oop = queue_new();
    TRACE(fprintf(stderr, "q_oop = %p\n", q_oop));
    result = object_call(q_oop, s_empty_p);
    assert(o_true == result);
    OOP n_ch = ch_A;
    while (object_call(n_ch, s_eq_p, ch_Z) != o_true) {
        q_oop = object_call(q_oop, s_give_x, n_ch);
        TRACE(fprintf(stderr, "q_oop = %p ^ [%c]\n", q_oop, as_integer(n_ch)->n));
        n_ch = object_call(n_ch, s_add, n_1);
    }
    result = object_call(q_oop, s_empty_p);
    assert(o_false == result);
    n_ch = ch_A;
    while (object_call(n_ch, s_eq_p, ch_Z) != o_true) {
        OOP n_i = object_call(q_oop, s_take_x);
        TRACE(fprintf(stderr, "q_oop = [%c] ^ %p\n", as_integer(n_i)->n, q_oop));
        result = object_call(n_i, s_eq_p, n_ch);
        assert(o_true == result);
        n_ch = object_call(n_ch, s_add, n_1);
    }
    result = object_call(q_oop, s_empty_p);
    assert(o_true == result);
*/
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
