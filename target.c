/* "Tiny Actor Run-Time" compilation target (see LICENSE file for Copyright) */
#include "target.h"

inline Pair     pair_new(Any h, Any t) {
    Pair q = NEW(PAIR);
    q->h = h;
    q->t = t;
    return q;
}

inline Pair     list_new() { return NIL; }
inline int      list_empty_p(Pair list) { return (list == NIL); }
inline Pair     list_pop(Pair list) { return list; }
inline Pair     list_push(Pair list, Any item) { return PR(item, list); }

inline Pair     queue_new() { return PR(NIL, NIL); }
inline int      queue_empty_p(Pair q) { return (q->h == NIL); }
inline void     queue_give(Pair q, Any item) {
    Pair p = PR(item, NIL);
    if (queue_empty_p(q)) {
        q->h = p;
    } else {
        Pair t = q->t;
        t->t = p;
    }
    q->t = p;
}
inline Any      queue_take(Pair q) {
    // if (queue_empty_p(q)) ERROR("can't take from empty queue");
    Pair p = q->h;
    Any item = p->h;
    q->h = p->t;
    p = FREE(p);
    return item;
}

inline Actor    actor_new(Behavior beh) {
    Actor a = NEW(ACTOR);
    a->behavior = beh;
    return a;
}
inline void     actor_become(Actor a, Behavior beh) { a->behavior = beh; }

inline Behavior behavior_new(Action act, Any data) {
    Behavior beh = NEW(BEHAVIOR);
    beh->action = act;
    beh->context = data;
    return beh;
}

inline Event    event_new(Config cfg, Actor a, Any msg) {
    Event e = NEW(EVENT);
    e->sponsor = cfg;
    e->actor = a;
    e->message = msg;
    return e;
}

inline Config   config_new() {
    Config cfg = NEW(CONFIG);
    cfg->event_q = queue_new();
    cfg->actors = NIL;
    return cfg;
}
inline void     config_enqueue(Config cfg, Event e) { queue_give(cfg->event_q, e); }
inline void     config_enlist(Config cfg, Actor a) { cfg->actors = list_push(cfg->actors, a); }
inline void     config_send(Config cfg, Actor target, Any msg) { config_enqueue(cfg, event_new(cfg, target, msg)); }
inline void     config_create(Config cfg, Behavior beh) { config_enlist(cfg, actor_new(beh)); }
int             config_dispatch(Config cfg) {
    if (queue_empty_p(cfg->event_q)) {
        return 0;
    }
    Event e = queue_take(cfg->event_q);
    (e->actor->behavior->action)(e);  // INVOKE ACTOR BEHAVIOR
    e = FREE(e); // FIXME: KEEP HISTORY HERE?
    return 1;
}

static void     act_sink(Event e) { /* no action */ }

BEHAVIOR sink_behavior = { act_sink, NIL };
ACTOR sink_actor = { &sink_behavior };
