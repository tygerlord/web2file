/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using low-level API
 *
 * Compile with:
 *
 *     gcc -Wall cefheadless_ll.c `pkg-config fuse3 --cflags --libs` -o cefheadless_ll
 *
 * ## Source code ##
 * \include cefheadless_ll.c
 */

#include <iostream>
#include <queue>
#include <map>
#include <ranges>
#include <iomanip>
#include <string_view>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <sys/mount.h>

#define FUSE_USE_VERSION 34
#include <fuse_lowlevel.h>

#include "headless_fuse.h"

static std::queue<std::string> input;
static std::queue<std::string> output;

static const char* mountpoint;
static struct stat mountpoint_stat;
static std::map<std::string, std::string> xattr;

static std::function<void(const std::string)> _write_callback;


static const char *hello_str = "Hello World!\n";
static const char *cefheadless_name = "browse.w2f";

static int headless_stat(fuse_ino_t ino, struct stat *stbuf)
{
	switch (ino) {
	case 1:
		memcpy(stbuf, &mountpoint_stat, sizeof(*stbuf));
		stbuf->st_ino = ino;
		//stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		break;

	case 2:
		stbuf->st_ino = ino;
		stbuf->st_mode = S_IFREG | 0664;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
		stbuf->st_uid = mountpoint_stat.st_uid;
		stbuf->st_gid = mountpoint_stat.st_gid;
		stbuf->st_rdev = mountpoint_stat.st_rdev;
		stbuf->st_atime = mountpoint_stat.st_atime;
		stbuf->st_mtime = mountpoint_stat.st_mtime;
		stbuf->st_ctime = mountpoint_stat.st_ctime;
		break;

	default:
		return -1;
	}
	return 0;
}

static void cefheadless_ll_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
	if (headless_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);

	fuse_reply_attr(req, &stbuf, 1.0);
}

static void cefheadless_ll_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;

	if (parent != 1 || strcmp(name, cefheadless_name) != 0)
		fuse_reply_err(req, ENOENT);
	else {
		memset(&e, 0, sizeof(e));
		e.ino = 2;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		headless_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
	}
}

struct dirbuf {
	char *p;
	size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	auto length = min(bufsize - off, maxsize);
	if(length > 0) {
		return fuse_reply_buf(req, buf + off, length);
	}

	return fuse_reply_buf(req, NULL, 0);
}

static void cefheadless_ll_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	if (ino != 1)
		fuse_reply_err(req, ENOTDIR);
	else {
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", 1);
		dirbuf_add(req, &b, "..", 1);
		dirbuf_add(req, &b, cefheadless_name, 2);
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
}

static void cefheadless_ll_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi)
{
	std::cerr << "open" << " " << ino << "\n";

	if (ino == 1) {
		fuse_reply_err(req, EISDIR);
		return;
	}

	/*
	if ((fi->flags & O_ACCMODE) != O_WRONLY) {
		fuse_reply_err(req, EACCES);
		return;
	}
	*/

	fuse_reply_open(req, fi);
}

static void cefheadless_ll_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	assert(ino == 2);
	reply_buf_limited(req, hello_str, strlen(hello_str), off, size);
}

static void cefheadless_ll_write(fuse_req_t req, fuse_ino_t ino, const char *buf,
		size_t size, off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	//assert(ino == 2);

	if(ino == 2) {
		fuse_reply_write(req, size);

		_write_callback(std::string(buf, size));
	}

}
#if 0
static void cefheadless_ll_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
							  size_t size)
{
	(void)size;
	//assert(ino == 2);
	
	std::cerr << "getxattr" << " " << ino << " " << name << " " << size << "\n";
	
	if(strcmp(name, "security.selinux") == 0) {
	
	}
	
	if(ino == 2) {
		if (strcmp(name, "cefheadless_ll_getxattr_name") == 0)
		{
			const char *buf = "cefheadless_ll_getxattr_value";
			fuse_reply_buf(req, buf, strlen(buf));
		}
	}
	
	//fuse_reply_err(req, ENOTSUP);
	fuse_reply_err(req, ENODATA);
}

static void cefheadless_ll_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
							  const char *value, size_t size, int flags)
{
	(void)flags;
	(void)size;
	assert(ino == 2);
	const char* exp_val = "cefheadless_ll_setxattr_value";
	if (strcmp(name, "cefheadless_ll_setxattr_name") == 0 &&
	    strlen(exp_val) == size &&
	    strncmp(value, exp_val, size) == 0)
	{
		fuse_reply_err(req, 0);
	}
	else
	{
		fuse_reply_err(req, ENOTSUP);
	}
}

static void cefheadless_ll_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name)
{
	assert(ino == 2);
	if (strcmp(name, "cefheadless_ll_removexattr_name") == 0)
	{
		fuse_reply_err(req, 0);
	}
	else
	{
		fuse_reply_err(req, ENOTSUP);
	}
}
#endif

static const struct fuse_lowlevel_ops cefheadless_ll_oper = {
	.lookup = cefheadless_ll_lookup,
	.getattr = cefheadless_ll_getattr,
	.open = cefheadless_ll_open,
	.read = cefheadless_ll_read,
	.write = cefheadless_ll_write,
	.readdir = cefheadless_ll_readdir,
//	.setxattr = cefheadless_ll_setxattr,
//	.getxattr = cefheadless_ll_getxattr,
//	.removexattr = cefheadless_ll_removexattr,
};

int fuse_start(int argc, char *argv[], std::function<void(const std::string)> write_callback)
{
	_write_callback = write_callback;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_session *se;
	struct fuse_cmdline_opts opts;
	struct fuse_loop_config config;
	int ret = -1;

	if (fuse_parse_cmdline(&args, &opts) != 0)
		return 1;
	if (opts.show_help) {
		printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
		fuse_cmdline_help();
		fuse_lowlevel_help();
		ret = 0;
		goto err_out1;
	} else if (opts.show_version) {
		printf("FUSE library version %s\n", fuse_pkgversion());
		fuse_lowlevel_version();
		ret = 0;
		goto err_out1;
	}

	if(opts.mountpoint == NULL) {
		printf("usage: %s [options] <mountpoint>\n", argv[0]);
		printf("       %s --help\n", argv[0]);
		ret = 1;
		goto err_out1;
	}

	mountpoint = opts.mountpoint;
	stat(mountpoint, &mountpoint_stat);

	se = fuse_session_new(&args, &cefheadless_ll_oper,
			      sizeof(cefheadless_ll_oper), NULL);
	if (se == NULL)
	    goto err_out1;

	if (fuse_set_signal_handlers(se) != 0)
	    goto err_out2;

	if (fuse_session_mount(se, opts.mountpoint) != 0)
	    goto err_out3;

	//fuse_daemonize(opts.foreground);

	/* Block until ctrl+c or fusermount -u */
	if (opts.singlethread)
		ret = fuse_session_loop(se);
	else {
		config.clone_fd = opts.clone_fd;
		config.max_idle_threads = opts.max_idle_threads;
		ret = fuse_session_loop_mt(se, &config);
	}

	fuse_session_unmount(se);
err_out3:
	fuse_remove_signal_handlers(se);
err_out2:
	fuse_session_destroy(se);
err_out1:
	free(opts.mountpoint);
	fuse_opt_free_args(&args);

	return ret ? 1 : 0;
}

void fuse_stop() {
	auto rc = umount(mountpoint);
	
	if(rc != 0) {
		throw std::string("Stop fuse error");
	}
}

#if 0
HeadlessFuse::HeadlessFuse() :
	se(nullptr) {

}

void HeadlessFuse::start(int argc, char *argv[]) {

	using namespace std::string_literals;

	close();

	args = FUSE_ARGS_INIT(argc, argv);
	args.argc = argc;
	args.argv = argv;
	args.allocated = 0;

	if (fuse_parse_cmdline(&args, &opts) != 0) {
		throw std::string("Bad command line");
	}

	if (opts.show_help) {
		fuse_cmdline_help();
		fuse_lowlevel_help();
		fuse_opt_free_args(&args);
		throw std::string("usage: [options] <mountpoint>");
	}

	if (opts.show_version) {
		fuse_lowlevel_version();
		fuse_opt_free_args(&args);
		std::string version = fuse_pkgversion();
		throw std::string("FUSE library version "s  + version);
	}

	if(opts.mountpoint == NULL) {
		fuse_opt_free_args(&args);
		throw std::string("Missing <mountpoint>");
	}

	se = fuse_session_new(&args, &cefheadless_ll_oper,
			      sizeof(cefheadless_ll_oper), NULL);

	if (se == NULL) {
		fuse_opt_free_args(&args);
		throw std::string("Error session fuse");
	}

	if (fuse_set_signal_handlers(se) != 0) {
	    fuse_opt_free_args(&args);
		throw std::string("Error setting signal handler");
	}

	if (fuse_session_mount(se, opts.mountpoint) != 0) {
	    fuse_opt_free_args(&args);
		throw std::string("Error mounting point");
	}

	//fuse_daemonize(opts.foreground);

	//int ret = 0;
	
	/* Block until ctrl+c or fusermount -u */
	if (opts.singlethread) {
		//ret = fuse_session_loop(se);
		fuse_session_loop(se);
	}
	else {
		config.clone_fd = opts.clone_fd;
		config.max_idle_threads = opts.max_idle_threads;
		//ret = fuse_session_loop_mt(se, &config);
		fuse_session_loop_mt(se, &config);
	}


	/*
	if(ret != 0)
	{
	    fuse_opt_free_args(&args);
		throw std::string("");
	}
	*/
}

HeadlessFuse::~HeadlessFuse() {
	close();
	//fuse_loop.join();
}

void HeadlessFuse::exit() {
	fuse_session_exit(se);
}

void HeadlessFuse::close() {
	if(se != nullptr) {
		fuse_session_unmount(se);

		fuse_remove_signal_handlers(se);

		fuse_session_destroy(se);

		free(opts.mountpoint);

		fuse_opt_free_args(&args);

		se = nullptr;
	}
}
#endif

