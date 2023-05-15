// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#ifndef HEADLESS_FUSE_H_
#define HEADLESS_FUSE_H_

#define FUSE_USE_VERSION 34

#include <fuse_lowlevel.h>

class HeadlessFuse {

public:
	HeadlessFuse(const int argc, const char *argv[]);
	~HeadlessFuse();

private:
	struct fuse_args args;
	struct fuse_session *se;
	struct fuse_cmdline_opts opts;
	struct fuse_loop_config config;

};

#endif // HEADLESS_FUSE_H_
	
