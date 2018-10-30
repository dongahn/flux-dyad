#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "../../libtap/tap.h"

int dyad_sync_health ()
{
    char *e = NULL;
    if ((e = getenv ("DYAD_SYNC_HEALTH"))) {
        if (!strcmp ("ok", e))
            return 0;
    }
    return -1;
}

void test_open_valid_path ()
{
    int fd = -1;

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    fd = open ("./consumer_path/data1.txt", O_RDONLY);
    ok (!(fd < 0) && (dyad_sync_health () == 0), "consumer: open sync worked");

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    close (fd);
    ok ((dyad_sync_health () == 0), "consumer: close sync worked");
}

void test_fopen_valid_path ()
{
    int rc = -1;
    FILE *fptr = NULL;

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    fptr = fopen ("./consumer_path/data2.txt", "r");
    ok ((fptr != NULL) && (dyad_sync_health () == 0), "consumer: fopen sync worked");

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    rc = fclose (fptr);
    ok ((rc == 0) && (dyad_sync_health () == 0), "consumer: fclose sync worked");
}

int main (int argc, char *argv[])
{
    plan (4);

    setenv ("DYAD_KIND_CONSUMER", "1", 1);
    setenv ("DYAD_PATH_CONSUMER", "./consumer_path", 1);
    setenv ("DYAD_SYNC_CHECK", "1", 1);

    test_open_valid_path ();

    test_fopen_valid_path ();

    done_testing ();
}

/*
 * vi: ts=4 sw=4 expandtab
 */
