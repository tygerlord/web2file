// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#ifndef HEADLESS_BROWSER_H_
#define HEADLESS_BROWSER_H_

#include "include/cef_browser.h"

class HeadlessBrowser  
{
 
	public:
		explicit HeadlessBrowser(const CefRefPtr<CefBrowser>& browser);
		~HeadlessBrowser();

		auto IsSame(CefRefPtr<CefBrowser> browser)
		{
			return _browser->IsSame(browser);
		}

		auto GetHost() const
		{
			return _browser->GetHost();
		}

	private:
		CefRefPtr<CefBrowser> _browser;
};

#endif  // HEADLESS_BROWSER_H_
