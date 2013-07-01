/* "Tiny Actor Run-Time" compilation target (see LICENSE file for Copyright) */

#include <stddef.h>
#include <stdlib.h>

#undef inline /*inline*/

#define ALLOC(S)    (calloc((S), 1))
#define NEW(T)      ((T *)calloc(sizeof(T), 1))
#define FREE(p)     ((p) = (free(p), NULL))

typedef void *Any;

typedef struct pair PAIR, *Pair;
typedef struct config CONFIG, *Config;
typedef struct behavior BEHAVIOR, *Behavior;
typedef struct actor ACTOR, *Actor;
typedef struct event EVENT, *Event;

typedef void (*Action)(Event e);

struct pair {
    Any         h;
    Any         t;
};

#define NIL ((Pair)0)
#define PR(h,t) pair_new((h),(t))

extern Pair     pair_new(Any h, Any t);

extern Pair     list_new();
extern int      list_empty_p(Pair list);
extern Pair     list_pop(Pair list);
extern Pair     list_push(Pair list, Any item);

extern Pair     queue_new();
extern int      queue_empty_p(Pair q);
extern void     queue_give(Pair q, Any item);
extern Any      queue_take(Pair q);

struct actor {
    Behavior    behavior;   // current behavior
};

struct behavior {
    Action      action;     // code
    Any         context;    // data
};

struct event {
    Config      sponsor;    // sponsor configuration
    Actor       actor;      // target actor
    Any         message;    // message to deliver
};

struct config {
    Pair        event_q;    // queue of messages in-transit
    Pair        actors;     // list of actors created
};

extern Actor    actor_new(Behavior beh);
extern void     actor_become(Actor a, Behavior beh);

extern Behavior behavior_new(Action act, Any data);

extern Event    event_new(Config cfg, Actor a, Any msg);

extern Config   config_new();
extern void     config_enqueue(Config cfg, Event e);
extern void     config_enlist(Config cfg, Actor a);
extern void     config_send(Config cfg, Actor target, Any msg);
extern void     config_create(Config cfg, Behavior beh);
extern int      config_dispatch(Config cfg);

extern BEHAVIOR sink_behavior;
extern ACTOR sink_actor;
