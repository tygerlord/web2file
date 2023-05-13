// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#ifndef HEADLESS_BROWSER_H_
#define HEADLESS_BROWSER_H_

#include "include/cef_browser.h"

class HeadlessBrowser : public CefBaseRefCounted {
 
public:
	explicit HeadlessBrowser(const CefRefPtr<CefBrowser>& browser);
	~HeadlessBrowser();

	auto IsSame(CefRefPtr<CefBrowser> browser) {
		return _browser->IsSame(browser);
	}

	auto GetHost() const {
		return _browser->GetHost();
	}

	auto get_namefifo() const {
		return namefifo;
	}

	void write(const std::string& text);

private:
	CefRefPtr<CefBrowser> _browser;

	std::string namefifo;

	int output, input;

private:
	IMPLEMENT_REFCOUNTING(HeadlessBrowser);

};

#endif  // HEADLESS_BROWSER_H_
