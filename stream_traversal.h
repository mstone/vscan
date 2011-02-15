/* Copyright (c) 2011 Akamai Technologies, Inc. */

class StreamTraversal : public Traversal
{
public:
	StreamTraversal();
	virtual ~StreamTraversal();

	int Init(Counter* counter,
		struct path_dir_pair root,
		const Decider* decider,
		Scanner* scanner) WARN_RET;

	int go(bool* done) WARN_RET;

	int dbg_print_current_path(const char* msg, const char* frag) const;

	int get_quoted_path(void* traversal_datum, string* qpath) const WARN_RET;

protected:
	virtual int get_next_dirent(int* dfd, struct dirent** dip) WARN_RET;

	virtual int go_dir(int dfd,
		const struct dirent& di,
		int fd,
		const struct stat& st) WARN_RET;

	virtual int go_reg(int dfd,
		const struct dirent& di,
		int fd,
		const struct stat& st) WARN_RET;

	virtual int go_lnk(int dfd, const struct dirent& di) WARN_RET;

	/* The path we were given in Init(); used for printing summary stats. */
	string root_path_;

	/* A stack of DIR*s and path fragments which we're currently
	   traversing. The front of the list is the top of the stack.
	   Our traversal will be done when worklist_ is empty. */
	list<struct path_dir_pair> worklist_;

	/* sensors_ lets us figure out which sensor(s) caused a given hit.
	   Presently, we act based on the first sensor able to explain a given
	   hit. sensors_ also records per-sensor hit frequencies during our
	   traversal. */
	list<struct sensor>* sensors_;

	/* decider_ is used to figure out which paths we actually want to scan
	   based on a dirent's name and its inode's stat data. */
	const Decider* decider_;

	/* prev_dir_ is used by get_next_dirent() to figure out when we need to
	   fchdir() to a new directory.
	   TODO(mistone): remove prev_dir_ in favor of iopen() or openat(). */
	DIR* prev_dir_;

	/* counter_ records statistics about how many directories and files
	   we've visited and/or scanned. */
	Counter* counter_;

	/* We hand scanner_ over to the Scannables we construc in order to
	   perform whatever actual scanning or capture was requested. */
	Scanner* scanner_;

private:
	DISALLOW_COPY_AND_ASSIGN(StreamTraversal);
};
