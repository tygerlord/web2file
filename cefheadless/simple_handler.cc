// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefheadless/simple_handler.h"
#include "cefheadless/headless_browser.h"

#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

class FrameVisitor : public CefStringVisitor {
public:
	FrameVisitor(const CefRefPtr<HeadlessBrowser>& headless_browser) :
		_headless_browser(headless_browser)
	{
	}


	void Visit(const CefString& html) override {
		auto page_html = html.ToString();
		_headless_browser->write(page_html);
	}

private:
	CefRefPtr<HeadlessBrowser> _headless_browser;

private:
  IMPLEMENT_REFCOUNTING(FrameVisitor);
};

//namespace {

HeadlessClient* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

//}  // namespace

HeadlessClient::HeadlessClient() : 
	is_closing_(false) {
	DCHECK(!g_instance);
	g_instance = this;

}

HeadlessClient::~HeadlessClient() {
  g_instance = nullptr;
}

// static
HeadlessClient* HeadlessClient::GetInstance() {
  return g_instance;
}

void HeadlessClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (!IsChromeRuntimeEnabled()) {
    // Set the title of the window using platform APIs.
    //PlatformTitleChange(browser, title);
  }
}

void HeadlessClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<HeadlessBrowser> headless_browser = new HeadlessBrowser(browser);
  // Add to the list of existing browsers.
  browser_list_.push_back(headless_browser);
}

bool HeadlessClient::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void HeadlessClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }


	if (browser_list_.empty()) {

		// All browser windows have closed. Quit the application message loop.
		CefQuitMessageLoop();
	}
}

void HeadlessClient::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Allow Chrome to show the error page.
  if (IsChromeRuntimeEnabled())
    return;

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void HeadlessClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefFrame> frame,
							   int httpStatusCode) {
	std::cerr << "OnLoadEnd" << std::endl;

    //CefPostTask(TID_UI, base::BindOnce(&HeadlessClient::CloseAllBrowsers, this,
    //                                   force_close));

	auto it = std::find_if(browser_list_.begin(), browser_list_.end(), [browser](auto headless_browser) {
			return headless_browser->IsSame(browser);
		});

	
	if(it != browser_list_.end()) {
	
		CefRefPtr<FrameVisitor> visitor = new FrameVisitor(*it);
		frame->GetSource(visitor);
	}

	/*
	while(on_visit)
	{
		usleep(1000);
		std::cout << "on_visit" << std::endl;
		CefDoMessageLoopWork();
	}
	*/

}

void HeadlessClient::OnLoadStart(CefRefPtr<CefBrowser> browser,
								 CefRefPtr<CefFrame> frame,
								 CefLoadHandler::TransitionType transition_type) {
	std::cerr << "OnLoadStart" << " " << transition_type << std::endl;
}


void HeadlessClient::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&HeadlessClient::CloseAllBrowsers, this,
                                       force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}

// static
bool HeadlessClient::IsChromeRuntimeEnabled() {
  static int value = -1;
  if (value == -1) {
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    value = command_line->HasSwitch("enable-chrome-runtime") ? 1 : 0;
  }
  return value == 1;
}

void HeadlessClient::GetViewRect(CefRefPtr<CefBrowser> browser,CefRect &rect) {
//CefRect(0, 0, browser->GetHost()->GetWindowlessFrameRate(), browser->GetHost()->GetWindowlessFrameRate());
	rect = CefRect(0, 0, browser->GetHost()->GetWindowlessFrameRate(), browser->GetHost()->GetWindowlessFrameRate());
}

void HeadlessClient::OnPaint (CefRefPtr< CefBrowser > browser, PaintElementType type, 
		const RectList &dirtyRects, const void *buffer, int width, int height) {

}



