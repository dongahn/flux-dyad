#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <wrapper.h>
// #include <flux.h>

#define DYAD_PATH_ENV "DYAD_PATH"


static inline int cannonical_pathcmp (const char *p1, const char *p2)
{
    // Only works when there are no multiple absolute paths via hardlinks
    char cann_path1[PATH_MAX];
    char cann_path2[PATH_MAX];
    if (!realpath (p1, cann_path1)) {
        DPRINTF ("error in realpath for %s (%s)\n", p1, strerror (errno));
        return -1;
    }
    if (!realpath (p2, cann_path2)) {
        DPRINTF ("error in realpath for %s\n", p2, strerror (errno));
        return -1;
    }
    return strcmp (cann_path1, cann_path2);
}


FILE *fopen (const char *path, const char *mode)
{
    char cp[PATH_MAX];
    char *dir = NULL;
    char *error = NULL;
    char *dyad_path = NULL;
    FILE *(*func_ptr) (const char *path, const char *mode) = NULL;

    func_ptr = dlsym (RTLD_NEXT, "fopen");
    if (!(error = dlerror ())) {
        DPRINTF ("error in dlsym: %s\n", error);
        return NULL;
    }
    if (!(dyad_path = getenv (DYAD_PATH_ENV))) {
        DPRINTF ("%s environment variable is not set.\n");
        goto real_call;
    }

    dir = dirname (strncpy (cp, path, PATH_MAX));
    if (!cannonical_pathcmp (dyad_path, dir)) {
        DPRINTF ("Do flux synchronization here \n");
#ifdef DYAD_CHECK
        FILE *tp = NULL;
        remove ("./fopen_sync");
        tp = fopen ("./fopen_sync", "rw");
        fclose (tp);
#endif
    }

real_call:
    return (func_ptr (path, mode));
}


int fclose (FILE *fp)
{
    int fno = -1;
    char *dir = NULL;
    char *error = NULL;
    char *dyad_path = NULL;
    char cp[PATH_MAX];
    char path[PATH_MAX];
    char proclink[PATH_MAX];
    int (*func_ptr) (FILE *fp) = NULL;

    func_ptr = dlsym (RTLD_NEXT, "fclose");
    if (!(error = dlerror ())) {
        DPRINTF ("error in dlsym: %s\n", error);
        return -1;
    }
    if (!fp) {
        DPRINTF ("Invalid FILE object\n");
        goto real_call;
    }
    if (!(dyad_path = getenv (DYAD_PATH_ENV))) {
        DPRINTF ("%s environment variable is not set.\n");
        goto real_call;
    }

    fno = fileno (fp);
    sprintf (proclink, "/proc/self/fd/%d", fno);
    if (readlink (proclink, path, PATH_MAX) < 0) {
        DPRINTF ("error reading the file link: %s\n", proclink);
        goto real_call;
    }
    dir = dirname (strncpy (cp, path, PATH_MAX));
    if (!cannonical_pathcmp (dyad_path, dir)) {
        DPRINTF ("Do flux synchronization here \n");
#ifdef DYAD_CHECK
        FILE *tp = NULL;
        remove ("./fclose_sync");
        tp = fopen ("./fclose_sync", "rw");
        fclose (tp);
#endif
    }

real_call:
    return (func_ptr (fp));
}

/*
 * vi: ts=4 sw=4 expandtab
 */
