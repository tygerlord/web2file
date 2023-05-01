// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef HEADLESS_CLIENT_H_
#define HEADLESS_CLIENT_H_

#include "include/cef_client.h"

#include <list>

#include "headless_browser.h"

class HeadlessClient : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler {
 
public:
  explicit HeadlessClient();
  ~HeadlessClient();

  // Provide access to the single global instance of this object.
  static HeadlessClient* GetInstance();

  // CefClient methods:
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) override;

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) override;

  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
						 CefRefPtr<CefFrame> frame,
						 int httpStatusCode) override;

  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
						   CefRefPtr<CefFrame> frame,
						   CefLoadHandler::TransitionType transition_type) override;


  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

  // Returns true if the Chrome runtime is enabled.
  static bool IsChromeRuntimeEnabled();

 private:
  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<HeadlessBrowser> BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(HeadlessClient);
};

#endif  // HEADLESS_CLIENT_H_