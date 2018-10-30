#ifndef WRAPPER_H
#define WRAPPER_H

#define DPRINTF(fmt,...) do { \
    if (ctx && ctx->debug) fprintf (stderr, fmt, ##__VA_ARGS__); \
} while (0)

#endif /* WRAPPER_H */

/*
 * vi: ts=4 sw=4 expandtab
 */
