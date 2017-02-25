/*
 * Copyright 2016-2017 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Alexander von Gluck IV <kallisti5@unixzen.com>
 *		Brian Hill <supernova@warpmail.net>
 */

#include "SoftwareUpdaterApp.h"


SoftwareUpdaterApp::SoftwareUpdaterApp()
	:
	BApplication("application/x-vnd.haiku-softwareupdater"),
	fWorker(NULL)
{
}


SoftwareUpdaterApp::~SoftwareUpdaterApp()
{
	if (fWorker) {
		fWorker->Lock();
		fWorker->Quit();
	}
}


void
SoftwareUpdaterApp::ReadyToRun()
{
	fWorker = new WorkingLooper();
}


int
main()
{
	SoftwareUpdaterApp app;
	return app.Run();
}
