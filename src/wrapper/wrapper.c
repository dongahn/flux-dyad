/*****************************************************************************\
 *  Copyright (c) 2018 Lawrence Livermore National Security, LLC.  Produced at
 *  the Lawrence Livermore National Laboratory (cf, AUTHORS, DISCLAIMER.LLNS).
 *  LLNL-CODE-658032 All rights reserved.
 *
 *  This file is part of the Flux resource manager framework.
 *  For details, see https://github.com/flux-framework.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license, or (at your option)
 *  any later version.
 *
 *  Flux is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <wrapper.h>
#include <flux/core.h>


/*****************************************************************************
 *                                                                           *
 *                   DYAD Sync Data Types and Declaration                    *
 *                                                                           *
 *****************************************************************************/

#define DYAD_KIND_PROD_ENV "DYAD_KIND_PRODUCER"
#define DYAD_KIND_CONS_ENV "DYAD_KIND_CONSUMER"
#define DYAD_PATH_PROD_ENV "DYAD_PATH_PRODUCER"
#define DYAD_PATH_CONS_ENV "DYAD_PATH_CONSUMER"
#define DYAD_CHECK_ENV     "DYAD_SYNC_HEALTH"

typedef struct {
    flux_t *h;
    bool debug;
    bool check;
    bool reenter;
} dyad_sync_ctx_t;

typedef enum {
    DYAD_PRODUCER,
    DYAD_CONSUMER
} dyad_kind_t;

static dyad_sync_ctx_t *ctx = NULL;
static void dyad_sync_init (void) __attribute__((constructor));
static void dyad_sync_fini (void) __attribute__((destructor));


/*****************************************************************************
 *                                                                           *
 *                   DYAD Sync Internal API                                  *
 *                                                                           *
 *****************************************************************************/

static inline int cannonical_pathcmp (const char *p1, const char *p2)
{
    // Only works when there are no multiple absolute paths via hardlinks
    char cann_path1[PATH_MAX];
    char cann_path2[PATH_MAX];
    if (!realpath (p1, cann_path1)) {
        DPRINTF ("DYAD_SYNC: error in realpath for %s\n", p1);
        DPRINTF ("DYAD_SYNC: %s\n", strerror (errno));
        return -1;
    }
    if (!realpath (p2, cann_path2)) {
        DPRINTF ("DYAD_SYNC: error in realpath for %s\n", p2);
        DPRINTF ("DYAD_SYNC: %s\n", strerror (errno));
        return -1;
    }
    return strcmp (cann_path1, cann_path2);
}

static inline bool is_dyad_producer ()
{
    char *e = NULL;
    if ((e = getenv (DYAD_KIND_PROD_ENV)))
        return true;
    return false;
}

static inline bool is_dyad_consumer ()
{
    char *e = NULL;
    if ((e = getenv (DYAD_KIND_CONS_ENV)))
        return true;
    return false;
}

static const char *get_exec_name (char *exe, size_t len)
{
    int rc;
    char fullpath[PATH_MAX] = {'\0'};
    if ((rc = readlink("/proc/self/exe", fullpath, len)) < 0) {
        DPRINTF ("DYAD_SYNC: error fetching executable name\n");
        exit(1);
    }
    strncpy (exe, basename (fullpath), len);
    return exe;
}

static int subscribe_via_flux (const char *fn)
{
    int rc = -1;
    flux_msg_t *msg = NULL;
    const char *srcfn = NULL;
    struct flux_match match = FLUX_MATCH_EVENT;

    if (!ctx->h)
        goto done;
    if (flux_event_subscribe (ctx->h, "dyad.sync.producer") != 0)
        goto done;

    while (1) {
        if (!(msg = flux_recv (ctx->h, match, 0)))
            goto done;
        if (flux_event_unpack (msg, NULL, "{ s:s }", "fn", &srcfn) < 0)
            goto done;
        if (!strcmp (fn, srcfn)) {
            flux_msg_destroy (msg);
            break;
         }
         flux_msg_destroy (msg);
    }
    rc = 0;

done:
    return rc;
}

static int publish_via_flux (const char *fn)
{
    int rc = -1;
    int seq = -1;
    char topic[PATH_MAX];
    char exe[PATH_MAX];
    flux_future_t *f = NULL;
    snprintf (topic, PATH_MAX, "%s.%s", "dyad.sync.producer",
              get_exec_name (exe, PATH_MAX));
    if (!ctx->h)
        goto done;
    if (!(f = flux_event_publish_pack (ctx->h, topic, 0, "{ s:s }", "fn", fn)))
        goto done;
    if (flux_event_publish_get_seq (f, &seq) < 0)
        goto done;
    rc = 0;
done:
    flux_future_destroy (f);
    return rc;
}

static int dyad_open_sync (const char *path)
{
    int rc = 0;
    char *fn = NULL;
    if (is_dyad_consumer ()) {
        char cp[PATH_MAX] = {'\0'};
        strcpy (cp, path);
        fn = basename (cp);
        ctx->reenter = false;
        rc = subscribe_via_flux (fn);
        ctx->reenter = true;
    }
    return rc;
}

static int dyad_close_sync (const char *path)
{
    int rc = 0;
    char *fn = NULL;
    if (is_dyad_producer ()) {
        char cp[PATH_MAX] = {'\0'};
        strcpy (cp, path);
        fn = basename (cp);
        ctx->reenter = false;
        rc = publish_via_flux (fn);
        ctx->reenter = true;
    }
    return rc;
}

static int open_sync (const char *path)
{
    int rc = 0;
    char cp[PATH_MAX];
    char *dir = NULL;
    char *dyad_path = NULL;

    if (!(dyad_path = getenv (DYAD_PATH_CONS_ENV))) {
        DPRINTF ("DYAD_SYNC: %s envVar is not set.\n", DYAD_PATH_CONS_ENV);
        goto done;
    }
    DPRINTF ("%s=%s.\n", DYAD_PATH_CONS_ENV, dyad_path);

    dir = dirname (strncpy (cp, path, PATH_MAX));
    if (!cannonical_pathcmp (dyad_path, dir))
        rc = dyad_open_sync (path); // annotate

done:
    if (rc == 0 && (ctx && ctx->check))
        setenv (DYAD_CHECK_ENV, "ok", 1);
    return rc;
}

static int close_sync (int fd)
{
    int rc = 0;
    char *dir = NULL;
    char *dyad_path = NULL;
    char cp[PATH_MAX] = {'\0'};
    char path[PATH_MAX] = {'\0'};
    char proclink[PATH_MAX] ={'\0'};

    if (!(dyad_path = getenv (DYAD_PATH_PROD_ENV))) {
        DPRINTF ("DYAD_SYNC: %s envVar is not set.\n", DYAD_PATH_PROD_ENV);
        goto done;
    }
    DPRINTF ("DYAD_SYNC: %s=%s.\n", DYAD_PATH_PROD_ENV, dyad_path);

    sprintf (proclink, "/proc/self/fd/%d", fd);
    if (readlink (proclink, path, PATH_MAX) < 0) {
        DPRINTF ("DYAD_SYNC: error reading the file link: %s\n", proclink);
        return 0;
    }
    DPRINTF ("DYAD_SYNC: file path: %s.\n", path);

    dir = dirname (strncpy (cp, path, PATH_MAX));
    if (!cannonical_pathcmp (dyad_path, dir))
        rc = dyad_close_sync (path);

done:
    if (rc == 0 && (ctx && ctx->check))
        setenv (DYAD_CHECK_ENV, "ok", 1);
    return rc;
}


/*****************************************************************************
 *                                                                           *
 *         DYAD Sync Constructor, Destructor and Wrapper API                 *
 *                                                                           *
 *****************************************************************************/

void dyad_sync_init (void)
{
    char *e = NULL;

    if (!(ctx = (dyad_sync_ctx_t *)malloc (sizeof (dyad_sync_ctx_t))))
        exit (1);
    if ((e = getenv ("DYAD_SYNC_DEBUG")))
        ctx->debug = true;
    else
        ctx->debug = false;
    if ((e = getenv ("DYAD_SYNC_CHECK")))
        ctx->check = true;
    else
        ctx->check = false;
    if (!(ctx->h = flux_open (NULL, 0)))
        DPRINTF ("DYAD_SYNC: can't open flux\n");
    ctx->reenter = true;
}

void dyad_sync_fini ()
{
    // flux_close (ctx->h) cannot be called in this destructor
    free (ctx);
}

int open (const char *path, int oflag, mode_t mode)
{
    char *error = NULL;
    int (*func_ptr) (const char *, int, mode_t) = NULL;

    func_ptr = dlsym (RTLD_NEXT, "open");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return -1;
    }

    if (!(ctx && ctx->h) || (ctx && !ctx->reenter) || !path)
        goto real_call;

    DPRINTF ("DYAD_SYNC: open sync begins (%s).\n", path);
    if (open_sync (path) < 0)
        goto real_call;

real_call:
    return (func_ptr (path, oflag, mode));
}

FILE *fopen (const char *path, const char *mode)
{
    char *error = NULL;
    FILE *(*func_ptr) (const char *, const char *) = NULL;

    func_ptr = dlsym (RTLD_NEXT, "fopen");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return NULL;
    }

    if (!(ctx && ctx->h) || (ctx && !ctx->reenter) || !path)
        goto real_call;

    DPRINTF ("DYAD_SYNC: fopen sync begins (%s).\n", path);
    if (open_sync (path) < 0)
        goto real_call;

real_call:
    return (func_ptr (path, mode));
}

int close (int fd)
{
    char *error = NULL;
    int (*func_ptr) (int) = NULL;

    func_ptr = dlsym (RTLD_NEXT, "close");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return -1;
    }

    if (!(ctx && ctx->h) || (ctx && !ctx->reenter) || fd < 0)
        goto real_call;

    DPRINTF ("DYAD_SYNC: close sync begins.\n");
    if (close_sync (fd) < 0)
        goto real_call;

real_call:
    return (func_ptr (fd));
}

int fclose (FILE *fp)
{
    char *error = NULL;
    int (*func_ptr) (FILE *) = NULL;

    func_ptr = dlsym (RTLD_NEXT, "fclose");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return -1;
    }

    if (!(ctx && ctx->h) || (ctx && !ctx->reenter) || !fp)
        goto real_call;

    DPRINTF ("DYAD_SYNC: fclose sync begins.\n");
    if (close_sync (fileno (fp)) < 0)
        goto real_call;

real_call:
    return (func_ptr (fp));
}

/*
 * vi: ts=4 sw=4 expandtab
 */
