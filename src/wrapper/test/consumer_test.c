#include "../../libtap/tap.h"

void test_valid_paths ()
{
    int rc = -1;
    FILE *fptr = NULL;
    FILE *fopen_sync = NULL;
    FILE *fclose_sync = NULL;

    fptr = fopen ("./data1.txt", "r");
    ok ((fptr != NULL), "fopen worked");
    rc = access ("./fopen_sync", R_OK);
    ok (!rc, "dyad fopen wrapper worked");
    rc = fclose (fptr);
    ok (!rc, "fclose worked");
    rc = access ("./fclose_sync", R_OK);
    ok (!rc, "dyad fclose wrapper worked");
}

int main (int argc, char *argv)
{
    test_valid_paths ();
}
