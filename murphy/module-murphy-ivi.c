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
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <pulsecore/pulsecore-config.h>

#include <pulse/timeval.h>
#include <pulse/xmalloc.h>

#include <pulsecore/macro.h>
#include <pulsecore/module.h>
#include <pulsecore/idxset.h>
#include <pulsecore/client.h>
#include <pulsecore/core-util.h>
#include <pulsecore/core-error.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>

#include "module-murphy-ivi-symdef.h"
#include "userdata.h"
#include "node.h"
#include "tracker.h"
#include "discover.h"
#include "router.h"
#include "constrain.h"
#include "multiplex.h"
#include "loopback.h"
#include "fader.h"
#include "volume.h"
#include "audiomgr.h"
#include "routerif.h"
#include "config.h"
#include "utils.h"

#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "murphy-ivi.conf"
#endif


PA_MODULE_AUTHOR("Janos Kovacs");
PA_MODULE_DESCRIPTION("Murphy and GenIVI compliant audio policy module");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(TRUE);
PA_MODULE_USAGE(
    "config_dir=<configuration directory>"
    "config_file=<policy configuration file> "
    "fade_out=<stream fade-out time in msec> "
    "fade_in=<stream fade-in time in msec> "
#ifdef WITH_DBUS
    "dbus_bus_type=<system|session> "
    "dbus_if_name=<policy dbus interface> "
    "dbus_murphy_path=<policy daemon's path> "
    "dbus_murphy_name=<policy daemon's name> "
    "dbus_audiomgr_path=<GenIVI audio manager's path> " 
    "dbus_audiomgr_name=<GenIVI audio manager's name> " 
#else
    "audiomgr_socktype=<tcp|unix> "
    "audiomgr_address=<audiomgr socket address> "
    "audiomgr_port=<audiomgr tcp port> "
#endif
    "null_sink_name=<name of the null sink> "
);

static const char* const valid_modargs[] = {
    "config_dir",
    "config_file",
    "fade_out",
    "fade_in",
#ifdef WITH_DBUS
    "dbus_bus_type",
    "dbus_if_name",
    "dbus_murphy_path",
    "dbus_murphy_name",
    "dbus_audiomgr_path",
    "dbus_audiomgr_name",
#else
    "audiomgr_socktype",
    "audiomgr_address",
    "audiomgr_port",
#endif
    "null_sink_name",
    NULL
};


int pa__init(pa_module *m) {
    struct userdata *u = NULL;
    pa_modargs      *ma = NULL;
    const char      *cfgdir;
    const char      *cfgfile;
    const char      *fadeout;
    const char      *fadein;
#ifdef WITH_DBUS
    const char      *dbustype;
    const char      *ifnam;
    const char      *mrppath;
    const char      *mrpnam;
    const char      *ampath;
    const char      *amnam;
#else
    const char      *socktype;
    const char      *amaddr;
    const char      *amport;
#endif
    const char      *nsnam;
    const char      *cfgpath;
    char             buf[4096];
    
    pa_assert(m);
    
    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        pa_log("Failed to parse module arguments.");
        goto fail;
    }

    cfgdir   = pa_modargs_get_value(ma, "config_dir", NULL);
    cfgfile  = pa_modargs_get_value(ma, "config_file", DEFAULT_CONFIG_FILE);
    fadeout  = pa_modargs_get_value(ma, "fade_out", NULL);
    fadein   = pa_modargs_get_value(ma, "fade_in", NULL);
#ifdef WITH_DBUS
    dbustype = pa_modargs_get_value(ma, "dbus_bus_type", NULL);
    ifnam    = pa_modargs_get_value(ma, "dbus_if_name", NULL);
    mrppath  = pa_modargs_get_value(ma, "dbus_murphy_path", NULL);
    mrpnam   = pa_modargs_get_value(ma, "dbus_murphy_name", NULL);
    ampath   = pa_modargs_get_value(ma, "dbus_audiomgr_path", NULL);
    amnam    = pa_modargs_get_value(ma, "dbus_audiomgr_name", NULL);
#else
    socktype = pa_modargs_get_value(ma, "audiomgr_socktype", NULL);
    amaddr   = pa_modargs_get_value(ma, "audiomgr_address", NULL);
    amport   = pa_modargs_get_value(ma, "audiomgr_port", NULL);
#endif
    nsnam    = pa_modargs_get_value(ma, "null_sink_name", NULL);
    
    u = pa_xnew0(struct userdata, 1);
    u->core      = m->core;
    u->module    = m;
    u->nullsink  = pa_utils_create_null_sink(u, nsnam);
    u->nodeset   = pa_nodeset_init(u);
    u->audiomgr  = pa_audiomgr_init(u);
#ifdef WITH_DBUS
    u->routerif  = pa_routerif_init(u, dbustype, ifnam, mrppath, mrpnam,
                                    ampath, amnam);
#else
    u->routerif  = pa_routerif_init(u, socktype, amaddr, amport);
#endif
    u->discover  = pa_discover_init(u);
    u->tracker   = pa_tracker_init(u);
    u->router    = pa_router_init(u);
    u->constrain = pa_constrain_init(u);
    u->multiplex = pa_multiplex_init();
    u->loopback  = pa_loopback_init();
    u->fader     = pa_fader_init(fadeout, fadein);
    u->volume    = pa_mir_volume_init(u);
    u->config    = pa_mir_config_init(u);

    u->state.sink   = PA_IDXSET_INVALID;
    u->state.source = PA_IDXSET_INVALID;

    if (u->nullsink == NULL || u->routerif == NULL  ||
        u->audiomgr == NULL || u->discover == NULL)
        goto fail;

    m->userdata = u;

    //cfgpath = pa_utils_file_path(cfgfile, buf, sizeof(buf));
    cfgpath = cfgfile;
    pa_mir_config_parse_file(u, cfgpath);

    pa_tracker_synchronize(u);

    mir_router_print_rtgroups(u, buf, sizeof(buf));
    pa_log_debug("%s", buf);
    
    pa_modargs_free(ma);
    
    return 0;
    
 fail:
    
    if (ma)
        pa_modargs_free(ma);
    
    pa__done(m);
    
    return -1;
}

void pa__done(pa_module *m) {
    struct userdata *u;

    pa_assert(m);
    
    if ((u = m->userdata)) {
    
        pa_tracker_done(u);
        pa_discover_done(u);
        pa_constrain_done(u);
        pa_router_done(u);
        pa_audiomgr_done(u);
        pa_routerif_done(u);
        pa_fader_done(u);
        pa_mir_volume_done(u);
        pa_mir_config_done(u);
        pa_nodeset_done(u);
        pa_utils_destroy_null_sink(u);

        pa_loopback_done(u->loopback, u->core);
        pa_multiplex_done(u->multiplex, u->core);

        pa_xfree(u);
    }
}



/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 */


