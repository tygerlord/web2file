// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#ifndef HEADLESS_FUSE_H_
#define HEADLESS_FUSE_H_

#include <string>
#include <functional>

int fuse_start(int argc, char *argv[], std::function<void(const std::string)> write_callback);
void fuse_stop();

#endif // HEADLESS_FUSE_H_
	
