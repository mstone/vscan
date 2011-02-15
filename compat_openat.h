/* Copyright (c) 2011 Akamai Technologies, Inc. */

#ifdef HAVE_OPENAT
#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#else
#define openat(dfd, path, flags) -1
#endif
