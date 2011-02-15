/* Copyright (c) 2011 Akamai Technologies, Inc. */

#ifdef HAVE_FSTATAT
#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <sys/stat.h>
#else
#define fstatat(dfd, path, stat, flags) -1
#endif
