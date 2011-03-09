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
#include "fd_traversal.h"
#include "scanner.h"
#include "scannable.h"
#include "fd_scannable.h"
#include "fake_fdopendir.h"


FDTraversal::FDTraversal()
	:
	root_path_(""),
	worklist_(),
	decider_(NULL),
	prev_dir_(NULL),
	counter_(NULL),
	scanner_(NULL)
	{}

FDTraversal::~FDTraversal() {}

int FDTraversal::Init(Counter* counter,
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

int FDTraversal::get_next_dirent(int* dfd, struct dirent** dip)
{
	int ret;        /* ret is for recording errors encountered by readdir_r */
	struct dirent* di;  /* di is for restoring *dip after readdir_r sets
	                       *dip = NULL. */

	while(worklist_.size() > 0)
	{
		/* 0. Get a directory */
		struct path_dir_pair p = worklist_.front();

		/* 1. If it's new, make it current. */
		if (prev_dir_ != p.dir)
		{
			*dfd = dirfd(p.dir);
			prev_dir_ = p.dir;
		}

		/* 2. Sanity-check dip and p.dir, save *dip, and reset p.dir. */
		TST(dip == NULL, errno = EINVAL,
			"tried use a null dip", out_error);
		TST(p.dir == NULL, errno = EINVAL,
			"tried to read a null dir", out_error);
		di = *dip;

		/* 3. readdir_r() and handle errors. */
		ret = readdir_r(p.dir, *dip, dip); // alternately, scandir
		TST(ret > 0, errno = ret, "readdir_r failed", out_error);
		TST(ret < 0, errno = EINVAL, "readdir_r went crazy", out_error);

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
	/* 5. Signal that iteration is complete by setting *dip = NULL. */
	*dip = NULL;
	return 0;
out_error:
	return 1;
}

int FDTraversal::go(bool* done)
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
		return go_lnk(dfd, di, false, st);

	if (di.d_type == DT_UNKNOWN)
	{
		CHK(fstatat(dfd, di.d_name, &st, AT_SYMLINK_NOFOLLOW) == -1,
			"unable to stat fd", out_error);

		if (S_ISDIR(st.st_mode))
			return go_dir(dfd, di, fd, st);
		if (S_ISLNK(st.st_mode))
			return go_lnk(dfd, di, true, st);
		if (S_ISREG(st.st_mode))
			return go_reg(dfd, di, fd, st);
	}

	return 0;
out_error:
	return 1;
}

int FDTraversal::go_dir(int dfd,
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
		LET(fd = openat(dfd, di.d_name, O_RDONLY), fd == -1,
			"unable to open dir; skipping", skip,
			"path: ^^ %d\n",
			dbg_print_current_path("eio-go-dir", di.d_name));
		LET(dir = fake_fdopendir(fd), dir == NULL,
			"unable to fake_fdopendir", out_error);
	} else {
		LET(dir = fake_fdopendir(fd), dir == NULL,
			"unable to fake_fdopendir", out_error);
	}

	LET(newfd = dirfd(dir), newfd == -1,
	        "unable to dirfd new dir", out_error);

	CHK(newfd != fd && fd != -1 && close(fd) == -1,
	        "unable to close old fd", out_error);

	p.path = string(di.d_name),
	p.dir = dir;
	worklist_.push_front(p);

skip:
	return 0;

out_error:
	return 1;
}

int FDTraversal::go_reg(int dfd,
	const struct dirent& di,
	int fd,
	const struct stat& st)
{
	int ret = 0;
	bool should_scan;
	struct stat st2 = st;
	FDScannable sc_fd;
	StringPiece frag = di.d_name;

	CHK(decider_->Decide(di, fd != -1, st2, &should_scan) != 0,
		"unable to decide whether to scan di", out_error);

	if ( ! should_scan)
		goto cleanup;

	if (fd == -1)
	{
		LET(fd = openat(dfd, di.d_name, O_RDONLY | O_NOFOLLOW), fd == -1,
			"unable to open reg file", cleanup,
			"Path: ^^ %d\n",
			dbg_print_current_path("eopen-reg-file", di.d_name));
		CHK(fstat(fd, &st2) == -1,
			"unable to stat fd", out_error);
		CHK(decider_->Decide(di, fd != -1, st2, &should_scan) != 0,
			"unable to decide whether to scan di", out_error);
		if ( ! should_scan)
			goto cleanup;
	}

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

int FDTraversal::go_lnk(int dfd,
	const struct dirent& di,
	bool has_st,
	struct stat& st)
{
	bool should_scan;
	StringPiece frag;
	string path2;
	int fd = -1;

	frag = di.d_name;

	if (!has_st)
	{
		CHK(fstatat(dfd, di.d_name, &st, AT_SYMLINK_NOFOLLOW) == -1,
			"unable to lstat link", out_error);
	}

	CHK(decider_->Decide(di, true, st, &should_scan) != 0,
		"unable to decide whether to scan di", out_error);

	if ( ! should_scan)
		goto skip;

	LET(fd = openat(dfd, di.d_name, O_RDONLY),
		fd == -1,
		"unable to open reg file", skip,
		"Path: ^^ %d\n",
		dbg_print_current_path("eopen-reg-file", di.d_name));

	CHK(fstat(fd, &st) == -1,
		"unable to stat fd", out_error);

	if (S_ISREG(st.st_mode))
		return go_reg(dfd, di, fd, st);

	return 0;

out_error:
	if (fd != -1)
	{
		TST(close(fd) == -1, fd = -1,
			"unable to close fd", out_error);
	}
	return 1;

skip:
	return 0;
}

int FDTraversal::dbg_print_current_path(const char* msg, const char* frag) const
{
	string qpath;
	StringPiece spfrag(frag);

	CHK(get_quoted_path(&spfrag, &qpath) != 0,
		"unable to get quoted path", out_error);

	fprintf(stderr, "E %s path %s\n", msg, qpath.c_str());
	return 0;
out_error:
	return 1;
}

int FDTraversal::get_quoted_path(void* traversal_datum, string* qpath) const
{
	StringPiece* frag = reinterpret_cast<StringPiece*>(traversal_datum);
	ostringstream oss;
	list<struct path_dir_pair>::const_reverse_iterator it, ie, first;
	first = worklist_.rbegin();
	for(it = worklist_.rbegin(), ie = worklist_.rend();
		it != ie;
		++it)
	{
		if (it != first)
			oss << "/";
		oss << uri_encode(it->path);
	}
	oss << "/" << uri_encode(*frag);

	*qpath = oss.str();
	return 0;
}
