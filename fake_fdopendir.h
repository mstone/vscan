/* Copyright (c) 2011 Akamai Technologies, Inc. */

#ifdef HAVE_FDOPENDIR
#define fake_fdopendir fdopendir
#else
DIR* fake_fdopendir(int fd);
#endif
