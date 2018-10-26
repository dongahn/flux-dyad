#define DPRINTF(fmt,...) do { \
    fprintf (stderr, fmt, ##__VA_ARGS__); \
} while (0)

/*
 * vi: ts=4 sw=4 expandtab
 */