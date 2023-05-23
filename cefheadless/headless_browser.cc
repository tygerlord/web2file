// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "include/base/cef_logging.h"
#include "cefheadless/headless_browser.h"
#include "cefheadless/headless_fuse.h"

HeadlessBrowser::HeadlessBrowser(const CefRefPtr<CefBrowser>& browser) :
	_browser(browser) {
}


HeadlessBrowser::~HeadlessBrowser() {
}

void HeadlessBrowser::write(const std::string& text) {

	std::cout << text << std::endl;
}



