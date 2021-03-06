/*
 * module-murphy-ivi -- PulseAudio module for providing audio routing support
 * Copyright (c) 2012, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301 USA.
 *
 */
#ifndef foomirnodefoo
#define foomirnodefoo

#include <sys/types.h>

#include "userdata.h"
#include "list.h"
#include "multiplex.h"
#include "loopback.h"
#include "volume.h"

#define AM_ID_INVALID  65535


enum mir_direction {
    mir_direction_unknown,
    mir_input,
    mir_output
};

enum mir_implement {
    mir_implementation_unknown = 0,
    mir_device,
    mir_stream
};

enum mir_location {
    mir_location_unknown = 0,
    mir_internal,
    mir_external
};

enum mir_node_type {
    mir_node_type_unknown = 0,

    /* application classes */
    mir_application_class_begin,
    mir_radio = mir_application_class_begin,
    mir_player,
    mir_navigator,
    mir_game,
    mir_browser,
    mir_phone,
    mir_event,
    mir_application_class_end,

    /* device types */
    mir_device_class_begin = 128,
    mir_null = mir_device_class_begin,
    mir_speakers,
    mir_front_speakers,
    mir_rear_speakers,
    mir_microphone,
    mir_jack,
    mir_spdif,
    mir_hdmi,
    mir_wired_headset,
    mir_wired_headphone,
    mir_usb_headset,
    mir_usb_headphone,
    mir_bluetooth_sco,
    mir_bluetooth_a2dp,
    mir_bluetooth_carkit,
    mir_bluetooth_source,
    mir_bluetooth_sink,
    mir_device_class_end,

    /* extensions */
    mir_user_defined_start = 256
};

enum mir_privacy {
    mir_privacy_unknown = 0,
    mir_public,
    mir_private
};

struct pa_node_card {
    uint32_t  index;
    char     *profile;
};


/**
 * @brief routing endpoint
 *
 * @details node is a routing endpoint in the GenIVI audio model.
 *          In pulseaudio terminology a routing endpoint is one of
 *          the following
 * @li      node is a pulseaudio sink or source. Such node is a
 *          combination of pulseudio card/profile + sink/port
 * @li      node is a pulseaudio stream. Such node in pulseaudio
 *          is either a sink_input or a source_output
 */
struct mir_node {
    uint32_t       index;     /**< index into nodeset->idxset */
    char          *key;       /**< hash key for discover lookups */
    mir_direction  direction; /**< mir_input | mir_output */
    mir_implement  implement; /**< mir_device | mir_stream */
    uint32_t       channels;  /**< number of channels (eg. 1=mono, 2=stereo) */
    mir_location   location;  /**< mir_internal | mir_external */
    mir_privacy    privacy;   /**< mir_public | mir_private */
    mir_node_type  type;      /**< mir_speakers | mir_headset | ...  */
    pa_bool_t      visible;   /**< internal or can appear on UI  */
    pa_bool_t      available; /**< eg. is the headset connected?  */
    pa_bool_t      ignore;    /**< do not consider it while routing  */
    char          *amname;    /**< audiomanager name */
    char          *amdescr;   /**< UI description */
    uint16_t       amid;      /**< handle to audiomanager, if any */
    char          *paname;    /**< sink|source|sink_input|source_output name */
    uint32_t       paidx;     /**< sink|source|sink_input|source_output index*/
    pa_node_card   pacard;    /**< pulse card related data, if any  */
    char          *paport;    /**< sink or source port if applies */
    pa_muxnode    *mux;       /**< for multiplexable input streams only */
    pa_loopnode   *loop;      /**< for looped back sources only */
    mir_dlist      rtentries; /**< for devices: listhead of nodchain,
                                   for streams: priority link (head is in
                                                               pa_router )
                               */
    mir_dlist      constrains;/**< listhead of constrains */
    mir_vlim       vlim;      /**< volume limit */
    uint32_t       stamp;
};


pa_nodeset *pa_nodeset_init(struct userdata *);
void pa_nodeset_done(struct userdata *);


mir_node *mir_node_create(struct userdata *, mir_node *);
void mir_node_destroy(struct userdata *, mir_node *);

mir_node *mir_node_find_by_index(struct userdata *, uint32_t);

int mir_node_print(mir_node *, char *, int);

const char *mir_direction_str(mir_direction);
const char *mir_implement_str(mir_implement);
const char *mir_location_str(mir_location);
const char *mir_node_type_str(mir_node_type);
const char *mir_node_type_str(mir_node_type);
const char *mir_privacy_str(mir_privacy);

#endif


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 */
