/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "log.h"
#include "encode.h"
#include "decode.h"
#include "decider.h"
#include "scanner_mode.h"
#include "config.h"
#include "path_dir_pair.h"
#include "counter.h"
#include "traversal.h"
#include "stream_traversal.h"
#include "scanner.h"
#include "scannable.h"
#include "fd_scannable.h"

#include "fake_fdopendir.h"

StreamTraversal::StreamTraversal()
	:
	root_path_(""),
	worklist_(),
	decider_(NULL),
	prev_dir_(NULL),
	counter_(NULL),
	scanner_(NULL)
	{}

StreamTraversal::~StreamTraversal() {}

int StreamTraversal::Init(Counter* counter,
	struct path_dir_pair root,
	const Decider * decider,
	Scanner* scanner)
{
	CHK(worklist_.size() != 0, "non-empty traversal", out_error);

	CHK(counter == NULL, "null counter", out_error);
	CHK(root.dir == NULL, "null root.dir", out_error);
	CHK(decider == NULL, "null decider", out_error);
	CHK(scanner == NULL, "null scanner", out_error);

	counter_ = counter;
	worklist_.push_back(root);
	decider_ = decider;
	scanner_ = scanner;

	root_path_ = root.path;

	counter_->num_dirs = 1; // for the root dir
	counter_->num_files = 0;
	counter_->num_interesting = 0;
	return 0;
out_error:
	return 1;
}

int StreamTraversal::get_next_dirent(int* dfd, struct dirent** dip)
{
	int ret;        /* ret is for recording errors encountered by readdir_r */
	int newdfd;     /* newdfd is for moving up a level in our dirstack */
	long dirpos;    /* dirpos is for resetting p.dir after a call
	                   to fchdir() invalidates it. */
	struct dirent* di;  /* di is for restoring *dip after readdir_r sets
	                       *dip = NULL. */

	while(worklist_.size() > 0)
	{
		/* 0. Get a directory */
		struct path_dir_pair p = worklist_.front();

		/* 1. If it's new, make it current. */
		if (prev_dir_ != p.dir)
		{
			newdfd = dirfd(p.dir);
			CHK(fchdir(newdfd) == -1,
				"unable to fchdir to new dir", out_error);
			prev_dir_ = p.dir;
		}

		/* 2. Sanity-check dip and p.dir, save *dip, and reset p.dir. */
		TST(dip == NULL, errno = EINVAL,
			"tried use a null dip", out_error);
		TST(p.dir == NULL, errno = EINVAL,
			"tried to read a null dir", out_error);
		di = *dip;
		LET(dirpos = telldir(p.dir), dirpos == -1,
			"unable to telldir()", out_error);
		seekdir(p.dir, dirpos);

		/* 3. readdir_r() and handle errors. */
		ret = readdir_r(p.dir, *dip, dip); // alternately, scandir
		if (ret > 0)
		{
			TST(true, errno = ret, "readdir_r failed", out_error);
		}
		else if (ret < 0)
		{
			TST(true, errno = EINVAL,
				"readdir_r went crazy", out_error);
		}
		else
		{
			/* 4. If you finished the current directory, try again;
			      otherwise, return. */
			if (*dip == NULL)
			{
				worklist_.pop_front();
				closedir(p.dir);
				*dip = di;
			}
			else
			{
				*dfd = dirfd(p.dir);
				return 0;
			}
		}
	}
	/* 5. Signal that iteration is complete by setting *dip = NULL. */
	*dip = NULL;
	return 0;
out_error:
	return 1;
}

int StreamTraversal::go(bool* done)
{
	struct dirent di;
	struct dirent* dip = &di;
	int fd, dfd;
	struct stat st;

	CHK(get_next_dirent(&dfd, &dip) != 0,
		"unable to get next dirent", out_error);

	if (dip == NULL)
	{
		ostringstream oss;
		oss << uri_encode(root_path_);
		string qpath = oss.str();
		write_c_line(&cout, qpath, counter_);
		*done = true;
		return 0;
	}
	else
	{
		*done = false;
	}

	// TRACE:
	// dbg_print_current_path("now", di.d_name);

	if (di.d_type != DT_DIR &&
		di.d_type != DT_REG &&
		di.d_type != DT_LNK &&
		di.d_type != DT_UNKNOWN)
		return 0;

	if (strlen(di.d_name) > 0 && di.d_name[0] == '.')
		return 0;

	fd = -1;

	counter_->num_files += 1;

	if (di.d_type == DT_DIR)
		return go_dir(dfd, di, fd, st);
	else if (di.d_type == DT_REG)
		return go_reg(dfd, di, fd, st);
	else if (di.d_type == DT_LNK)
		return go_lnk(dfd, di);

	if (di.d_type == DT_UNKNOWN)
	{

		// fstatat(dfd, path, &st, AT_SYMLINK_NOFOLLOW)
		// openat(dfd, di.d_name, O_RDONLY | O_NOFOLLOW);
		// iopen(),fiopen() -- but what's the mount point?
		fd = open(di.d_name, O_RDONLY | O_NOFOLLOW);
		if (fd == -1 && errno == ELOOP)
			return go_lnk(dfd, di);

		CHK(fd == -1 && errno == ENOENT,
			"non-existent dirent", skip,
			"Path: ^^ %d\n",
			dbg_print_current_path("enoent-dirent", di.d_name));

		CHK(fd == -1, "unable to open dirent", out_error)

		CHK(fstat(fd, &st) == -1,
			"unable to stat fd", out_error);

		if (S_ISDIR(st.st_mode))
			return go_dir(dfd, di, fd, st);
		if (S_ISREG(st.st_mode))
			return go_reg(dfd, di, fd, st);

		return 0;
	}

out_error:
	return 1;
skip:
	return 0;
}

int StreamTraversal::go_dir(int,
	const struct dirent& di,
	int fd,
	const struct stat&)
{
	struct path_dir_pair p;
	DIR* dir;
	int newfd;

	counter_->num_files -= 1;
	counter_->num_dirs += 1;

	if (fd == -1)
	{
		LET(dir = opendir(di.d_name), dir == NULL,
			"unable to open dir; skipping", skip,
			"path: ^^ %d\n",
			dbg_print_current_path("eio-go-dir", di.d_name));
	} else {
		LET(dir = fake_fdopendir(fd), dir == NULL,
			"unable to fake_fdopendir; skipping", skip);
	}
	LET(newfd = dirfd(dir), newfd == -1,
		"unable to dirfd new dir", out_error);
	CHK(newfd != fd && fd != -1 && close(fd) == -1,
		"unable to close old fd", out_error);
	CHK(fchdir(newfd) == -1,
		"unable to fchdir", out_error);

	p.path = string(di.d_name),
	p.dir = dir;
	worklist_.push_front(p);

skip:
	return 0;

out_error:
	return 1;
}

int StreamTraversal::go_reg(int,
	const struct dirent& di,
	int fd,
	const struct stat& st)
{
	int ret = 0;
	bool should_scan;
	struct stat st2 = st;
	FDScannable sc_fd;
	StringPiece frag = di.d_name;

	if (fd == -1)
	{
		LET(fd = open(di.d_name, O_RDONLY | O_NOFOLLOW), fd == -1,
			"unable to open reg file; skipping", cleanup,
			"Path: ^^ %d\n",
			dbg_print_current_path("eio-go-reg", di.d_name));
		CHK(fstat(fd, &st2) == -1,
			"unable to stat fd", out_error);
	}

	CHK(decider_->Decide(di, st2, &should_scan) != 0,
		"unable to decide whether to scan di", out_error);

	if (! should_scan)
		goto cleanup;

	CHK(sc_fd.Init(this, &frag, (uint64_t)st2.st_size, fd) != 0,
		"unable to init fd-scannable", out_error);

	CHK(scanner_->scan(&sc_fd) != 0,
		"unable to scan fd-scannable", out_error);

	CHK(sc_fd.Destroy() != 0,
		"unable to destroy fd-scannable", out_error);

	ret = 0;
	goto cleanup;
out_error:
	ret = 1;
	goto cleanup;
cleanup:
	if (fd != -1)
	{
		TST(close(fd) == -1, fd = -1,
			"unable to close fd", out_error);
	}
	return ret;
}

int StreamTraversal::go_lnk(int dfd, const struct dirent& di)
{
	bool should_scan;
	StringPiece frag;
	string path2;
	int fd = -1, ret = 0;
	struct stat st;

	frag = di.d_name;

	CHK(lstat(di.d_name, &st) == -1,
		"unable to lstat link", out_error);

	CHK(decider_->Decide(di, st, &should_scan) != 0,
		"unable to decide whether to scan di", out_error);

	if (! should_scan)
		goto cleanup;

	LET(fd = open(di.d_name, O_RDONLY),
		fd == -1,
		"unable to open reg file", cleanup,
		"Path: ^^ %d\n",
		dbg_print_current_path("eopen-reg-file", di.d_name));

	CHK(fstat(fd, &st) == -1,
		"unable to stat fd", out_error);
	if (S_ISREG(st.st_mode))
		return go_reg(dfd, di, fd, st);

	ret = 0;
	goto cleanup;

out_error:
	ret = 1;

cleanup:
	if (fd != -1)
	{
		TST(close(fd) == -1, fd = -1,
			"unable to close fd", out_error);
	}
	return ret;
}

int StreamTraversal::dbg_print_current_path(const char* msg, const char* frag) const
{
	char buf[PATH_MAX+1];
	CHK(getcwd(buf, sizeof(buf)) == NULL,
		"unable to getcwd()", out_error);
	fprintf(stderr, "E %s path %s/%s\n", msg, buf, frag);
	return 0;
out_error:
	return 1;
}

int StreamTraversal::get_quoted_path(void* traversal_datum, string* qpath) const
{
	StringPiece* frag = reinterpret_cast<StringPiece*>(traversal_datum);
	ostringstream oss;
	list<struct path_dir_pair>::const_reverse_iterator iter;
	for(iter = worklist_.rbegin();
		iter != worklist_.rend();
		++iter)
	{
		if (iter != worklist_.rbegin())
			oss << "/";
		oss << uri_encode(iter->path);
	}
	oss << "/" << uri_encode(*frag);

	*qpath = oss.str();
	return 0;
}
