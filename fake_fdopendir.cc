/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"
#include "fake_fdopendir.h"

#include "cgen.h"

DIR* fake_fdopendir(int fd)
{
	int cdfd, ret;
	DIR* dir = NULL;

	LET(cdfd = ret = open(".", O_RDONLY), cdfd == -1,
		"unable to open(\".\")", out_ret);
	LET(ret = fchdir(fd), ret == -1,
		"unable to fchdir to fd", out_closecdfd);
	LET(dir = opendir("."), dir == NULL,
		"unable to opendir(\".\")", out_fchdir);
	CHK(fchdir(cdfd) == -1,
		"unable to fchdir to cdfd", out_closedir);

	goto out_closecdfd;

out_closedir:
	closedir(dir);
	dir = NULL;

out_fchdir:
	// XXX: No point checking the return code here since we're already
	// in the process of signalling failure. <MS>
	ret = fchdir(cdfd);

out_closecdfd:
	close(cdfd);

out_ret:
	return dir;
}
