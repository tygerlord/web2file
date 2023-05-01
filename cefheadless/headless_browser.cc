// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#include "cefheadless/headless_browser.h"

HeadlessBrowser::HeadlessBrowser(const CefRefPtr<CefBrowser>& browser) :
	_browser(browser) 
{
}

HeadlessBrowser::~HeadlessBrowser()
{
}


