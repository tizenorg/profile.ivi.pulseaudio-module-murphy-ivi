#ifndef footrackerfoo
#define footrackerfoo

#include "userdata.h"


struct pa_card_hooks {
    pa_hook_slot    *put;
    pa_hook_slot    *unlink;
    pa_hook_slot    *profchg;
};

struct pa_sink_hooks {
    pa_hook_slot    *put;
    pa_hook_slot    *unlink;
    pa_hook_slot    *portchg;
    pa_hook_slot    *portavail;
};

struct pa_source_hooks {
    pa_hook_slot    *put;
    pa_hook_slot    *unlink;
    pa_hook_slot    *portchg;
    pa_hook_slot    *portavail;
};

struct pa_sink_input_hooks {
    pa_hook_slot    *neew;
    pa_hook_slot    *put;
    pa_hook_slot    *unlink;
};


struct pa_tracker {
    pa_card_hooks       card;
    pa_sink_hooks       sink;
    pa_source_hooks     source;
    pa_sink_input_hooks sink_input;
};

pa_tracker *pa_tracker_init(struct userdata *);
void pa_tracker_done(struct userdata *);

void pa_tracker_synchronize(struct userdata *);



#endif /* footrackerfoo */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 */