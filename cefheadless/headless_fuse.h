// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#ifndef HEADLESS_FUSE_H_
#define HEADLESS_FUSE_H_

#include <string>
#include <string_view>
#include <functional>

int fuse_start(int argc, char *argv[], std::function<void(const std::string)> write_callback);
void fuse_stop();
void fuse_write(const std::string& file_datas);

#endif // HEADLESS_FUSE_H_
	
