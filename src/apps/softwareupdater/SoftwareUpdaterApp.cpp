/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Alexander von Gluck IV <kallisti5@unixzen.com>
 *		Brian Hill <supernova@warpmail.net>
 */

#include "SoftwareUpdaterApp.h"

#include "SoftwareUpdaterWindow.h"


//#undef B_TRANSLATION_CONTEXT
//#define B_TRANSLATION_CONTEXT "SoftwareUpdater"


SoftwareUpdaterApp::SoftwareUpdaterApp()
	:
	BApplication("application/x-vnd.haiku-softwareupdater"),
	fWindow(NULL),
	fWorker(NULL)
{
}


SoftwareUpdaterApp::~SoftwareUpdaterApp()
{
	delete fWindow;
	if (fWorker) {
		fWorker->Lock();
		fWorker->Quit();
	}
}


void
SoftwareUpdaterApp::ReadyToRun()
{
	fWindow = new SoftwareUpdaterWindow();
	
	fWorker = new WorkingLooper();
	fWorker->PostMessage(kMsgStart);
}


int
main()
{
	SoftwareUpdaterApp app;
	return app.Run();
}
