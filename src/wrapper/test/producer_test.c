#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
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

int copy (int fd, const char *path)
{
    int ofd = 0;
    size_t b = 0;
    struct stat sbuf;
    char *buffer = NULL;

    if (fstat (fd, &sbuf) < 0) {
        fprintf (stderr, "error in fstat\n");
        goto done;
    }
    if ((buffer = (char *)malloc (sbuf.st_size)) == NULL)
        exit (1);
    if ((ofd = open (path, O_RDWR|O_CREAT, 0666)) < 0)
        return -1;
    if ((b = read (fd, buffer, sbuf.st_size)) != sbuf.st_size)
        return -1;
    if ((b = write (ofd, buffer, sbuf.st_size)) != sbuf.st_size)
        return -1;

done:
    free (buffer);
    if (ofd)
        close (ofd);
    return 0;
}

void test_open_valid_path ()
{
    int fd = -1;
    int rc = 0;

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    fd = open ("./data1.txt", O_RDONLY);
    ok (!(fd < 0) && (dyad_sync_health () == 0), "producer: open sync worked");

    rc = copy (fd, "./consumer_path/data1.txt");
    ok (rc == 0, "producer: file copy to the consumer location");

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    close (fd);
    ok ((dyad_sync_health () == 0), "producer: close sync worked");
}

void test_fopen_valid_path ()
{
    int rc = -1;
    FILE *fptr = NULL;

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    fptr = fopen ("./data2.txt", "r");
    ok ((fptr != NULL) && (dyad_sync_health () == 0), "producer: fopen sync worked");

    rc = copy (fileno (fptr), "./consumer_path/data2.txt");
    ok (rc == 0, "producer: file copy to the consumer location");

    setenv ("DYAD_SYNC_HEALTH", "test", 1);
    rc = fclose (fptr);
    ok ((rc == 0) && (dyad_sync_health () == 0), "producer: fclose sync worked");
}

int main (int argc, char *argv[])
{
    plan (6);

    setenv ("DYAD_KIND_PRODUCER", "1", 1);
    setenv ("DYAD_PATH_PRODUCER", "./", 1);

    test_open_valid_path ();

    test_fopen_valid_path ();

    done_testing ();
}

/*
 * vi: ts=4 sw=4 expandtab
 */
