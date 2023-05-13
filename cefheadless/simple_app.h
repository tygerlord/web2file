// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef HEADLESS_APP_H_
#define HEADLESS_APP_H_

#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class HeadlessApp : public CefApp, public CefBrowserProcessHandler {
 public:
  HeadlessApp();

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;

  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(HeadlessApp);
};

#endif  // HEADLESS_APP_H_
