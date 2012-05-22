#ifndef foomirlistfoo
#define foomirlistfoo

#define MIR_DIM(a)  (sizeof(a)/sizeof((a)[0]))

#define MIR_OFFSET(structure, member)                                   \
    ((int)((void *)((&((structure *)0)->member)) - (void *)0))

#define MIR_LIST_RELOCATE(structure, member, ptr)                       \
    ((structure *)((void *)ptr - MIR_OFFSET(structure, member)))


#define MIR_DLIST_HEAD(name)   mir_dlist name = { &(name), &(name) }

#define MIR_DLIST_INIT(self)                                            \
    do {                                                                \
        (&(self))->prev = &(self);                                      \
        (&(self))->next = &(self);                                      \
    } while(0)

#define MIR_DLIST_EMPTY(name)  ((&(name))->next == &(name))

#define MIR_DLIST_FOR_EACH(structure, member, pos, head)                \
    for (pos = MIR_LIST_RELOCATE(structure, member, (head)->next);      \
         &pos->member != (head);                                        \
         pos = MIR_LIST_RELOCATE(structure, member, pos->member.next))

#define MIR_DLIST_FOR_EACH_SAFE(structure, member, pos, n, head)        \
    for (pos = MIR_LIST_RELOCATE(structure, member, (head)->next),      \
           n = MIR_LIST_RELOCATE(structure, member, pos->member.next);  \
         &pos->member != (head);                                        \
         pos = n,                                                       \
           n = MIR_LIST_RELOCATE(structure, member, pos->member.next))

#define MIR_DLIST_FOR_EACH_BACKWARD(structure, member, pos, head)       \
    for (pos = MIR_LIST_RELOCATE(structure, member, (head)->prev);      \
         &pos->member != (head);                                        \
         pos = MIR_LIST_RELOCATE(structure, member, pos->member.prev))

#define MIR_DLIST_FOR_EACH_NOHEAD(structure, member, pos, start)        \
    for (pos = start;                                                   \
         &(pos)->member != &(start)->member;                            \
         pos = MIR_LIST_RELOCATE(structure, member, pos->member.next))

#define MIR_DLIST_FOR_EACH_NOHEAD_SAFE(structure, member, pos,n, start) \
    for (pos = start,                                                   \
           n = MIR_LIST_RELOCATE(structure, member, pos->member.next);  \
         &pos->member != &(start)->member;                              \
         pos = n,                                                       \
           n = MIR_LIST_RELOCATE(structure, member, pos->member.next))

#define MIR_DLIST_INSERT_BEFORE(structure, member, new, before)         \
    do {                                                                \
        mir_dlist *after = (before)->prev;                              \
        after->next = &(new)->member;                                   \
        (new)->member.next = before;                                    \
        (before)->prev = &(new)->member;                                \
        (new)->member.prev = after;                                     \
    } while(0)
#define MIR_DLIST_APPEND(structure, member, new, head)                  \
    MIR_DLIST_INSERT_BEFORE(structure, member, new, head)

#define MIR_DLIST_INSERT_AFTER(structure, member, new, after)           \
    do {                                                                \
        mir_dlist *before = (after)->next;                              \
        (after)->next = &((new)->member);                               \
        (new)->member.next = before;                                    \
        before->prev = &((new)->member);                                \
        (new)->member.prev = after;                                     \
    } while(0)
#define MIR_DLIST_PREPEND(structure, member, new, head)                 \
    MIR_DLIST_INSERT_AFTER(structure, member, new, head)


#define MIR_DLIST_UNLINK(structure, member, elem)                       \
    do {                                                                \
        mir_dlist *after  = (elem)->member.prev;                        \
        mir_dlist *before = (elem)->member.next;                        \
        after->next = before;                                           \
        before->prev = after;                                           \
        (elem)->member.prev = (elem)->member.next = &(elem)->member;    \
    } while(0)


typedef struct mir_dlist {
    struct mir_dlist *prev;
    struct mir_dlist *next;
} mir_dlist;

#endif /* foomirlistfoo */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 */
