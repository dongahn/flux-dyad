#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <wrapper.h>
// #include <flux.h>

#define DYAD_PATH_ENV "DYAD_PATH"
#define DYAD_CHECK_ENV "DYAD_SYNC_HEALTH"

static int debug = 0;

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

static int open_sync (const char *path)
{
    char cp[PATH_MAX];
    char *dir = NULL;
    char *dyad_path = NULL;

    if (!(dyad_path = getenv (DYAD_PATH_ENV))) {
        DPRINTF ("DYAD_SYNC: %s envVar is not set.\n", DYAD_PATH_ENV);
        return 0;
    }
    DPRINTF ("%s=%s.\n", DYAD_PATH_ENV, dyad_path);

    dir = dirname (strncpy (cp, path, PATH_MAX));
    if (!cannonical_pathcmp (dyad_path, dir))
        DPRINTF ("Do flux synchronization here \n");

#ifdef DYAD_CHECK
    setenv (DYAD_CHECK_ENV, "ok", 1);
#endif
    return 0;
}

static int close_sync (int fd)
{
    char *dir = NULL;
    char *dyad_path = NULL;
    char cp[PATH_MAX];
    char path[PATH_MAX];
    char proclink[PATH_MAX];

    if (!(dyad_path = getenv (DYAD_PATH_ENV))) {
        DPRINTF ("DYAD_SYNC:%s envVar is not set.\n", DYAD_PATH_ENV);
        return 0;
    }
    DPRINTF ("DYAD_SYNC: %s=%s.\n", DYAD_PATH_ENV, dyad_path);

    sprintf (proclink, "/proc/self/fd/%d", fd);
    if (readlink (proclink, path, PATH_MAX) < 0) {
        DPRINTF ("DYAD_SYNC: error reading the file link: %s\n", proclink);
        return 0;
    }
    DPRINTF ("DYAD_SYNC: file path: %s.\n", path);

    dir = dirname (strncpy (cp, path, PATH_MAX));
    if (!cannonical_pathcmp (dyad_path, dir))
        DPRINTF ("Do flux synchronization here \n");

#ifdef DYAD_CHECK
    setenv (DYAD_CHECK_ENV, "ok", 1);
#endif
    return 0;
}

void dyad_sync_init ()
{
    char *e = NULL;
    if ((e = getenv ("DYAD_SYNC_DEBUG")))
        debug = 1;
}

int open (const char *path, int oflag)
{
    char *error = NULL;
    int (*func_ptr) (const char *, int) = NULL;

    DPRINTF ("DYAD_SYNC: open sync begins.\n");

    func_ptr = dlsym (RTLD_NEXT, "open");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return -1;
    }

    DPRINTF ("DYAD_SYNC: dlsym succeeded.\n");

    if (!path)
        goto real_call;
    if (open_sync (path) < 0)
        return -1;

real_call:
    return (func_ptr (path, oflag));
}

FILE *fopen (const char *path, const char *mode)
{
    char *error = NULL;
    FILE *(*func_ptr) (const char *, const char *) = NULL;

    DPRINTF ("DYAD_SYNC: fopen sync begins.\n");

    func_ptr = dlsym (RTLD_NEXT, "fopen");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return NULL;
    }
    DPRINTF ("DYAD_SYNC: dlsym succeeded.\n");

    if (!path)
        goto real_call;
    if (open_sync (path) < 0)
        return NULL;

real_call:
    return (func_ptr (path, mode));
}

int close (int fd)
{
    char *error = NULL;
    int (*func_ptr) (int) = NULL;

    DPRINTF ("DYAD_SYNC: close sync begins.\n");

    func_ptr = dlsym (RTLD_NEXT, "close");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return -1;
    }
    DPRINTF ("DYAD_SYNC: dlsym succeeded.\n");

    if (fd < 0)
        goto real_call;
    if (close_sync (fd) < 0)
        return -1;

real_call:
    return (func_ptr (fd));
}

int fclose (FILE *fp)
{
    char *error = NULL;
    int (*func_ptr) (FILE *) = NULL;

    DPRINTF ("DYAD_SYNC: fclose sync begins.\n");

    func_ptr = dlsym (RTLD_NEXT, "fclose");
    if ((error = dlerror ())) {
        DPRINTF ("DYAD_SYNC: error in dlsym: %s\n", error);
        return -1;
    }
    DPRINTF ("DYAD_SYNC: dlsym succeeded.\n");

    if (!fp)
        goto real_call;
    if (close_sync (fileno (fp)) < 0)
        return -1;

real_call:
    return (func_ptr (fp));
}

/*
 * vi: ts=4 sw=4 expandtab
 */
