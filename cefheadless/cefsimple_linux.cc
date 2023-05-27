// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefheadless/simple_app.h"
#include "cefheadless/headless_fuse.h"

#if defined(CEF_X11)
#include <X11/Xlib.h>
#endif

#include "include/base/cef_logging.h"
#include "include/cef_command_line.h"
#include "include/cef_thread.h"
#include "include/cef_task.h"

#if defined(CEF_X11)
namespace {

int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X error received: "
               << "type " << event->type << ", "
               << "serial " << event->serial << ", "
               << "error_code " << static_cast<int>(event->error_code) << ", "
               << "request_code " << static_cast<int>(event->request_code)
               << ", "
               << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}

}  // namespace
#endif  // defined(CEF_X11)

class StartFuse : public CefTask {

public:
	void Execute() {
		int argc = 2;
		char *argv[argc] = {
			const_cast<char*>("web2file"),
			const_cast<char*>("_web2file"),
		};

		LOG(WARNING) << "Start fuse process";

		fuse_start(argc, argv);
		
		LOG(WARNING) << "End fuse process";
	}
private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(StartFuse);

};


// Entry point function for all processes.
int main(int argc, char* argv[]) {
	// Provide CEF with command-line arguments.
	CefMainArgs main_args(argc, argv);

	// CEF applications have multiple sub-processes (render, GPU, etc) that share
	// the same executable. This function checks the command-line and, if this is
	// a sub-process, executes the appropriate logic.
	int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
	if (exit_code >= 0) {
		// The sub-process has completed so return here.
		return exit_code;
	}

#if defined(CEF_X11)
	// Install xlib error handlers so that the application won't be terminated
	// on non-fatal errors.
	XSetErrorHandler(XErrorHandlerImpl);
	XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

	// Parse command-line arguments for use in this method.
	CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
	command_line->InitFromArgv(argc, argv);

	// Specify CEF global settings here.
	CefSettings settings;

	if (command_line->HasSwitch("enable-chrome-runtime")) {
	// Enable experimental Chrome runtime. See issue #2969 for details.
		settings.chrome_runtime = true;
	}

	// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
	// automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
	// use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif

	// SimpleApp implements application-level callbacks for the browser process.
	// It will create the first browser instance in OnContextInitialized() after
	// CEF has initialized.
	CefRefPtr<HeadlessApp> app(new HeadlessApp);

	// Initialize CEF for the browser process.
	CefInitialize(main_args, settings, app.get(), nullptr);

	auto mount_fuse_thread = CefThread::CreateThread("mount_fuse_thread");
	auto run_fuse_thread = CefThread::CreateThread("run_fuse_thread");

	auto task_runner = mount_fuse_thread->GetTaskRunner();

	task_runner->PostTask(new StartFuse());

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is
	// called.
	CefRunMessageLoop();

	mount_fuse_thread->Stop();
	run_fuse_thread->Stop();

	// Shut down CEF.
	CefShutdown();

	return 0;
}

