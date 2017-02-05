/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Alexander von Gluck IV <kallisti5@unixzen.com>
 *		Brian Hill <supernova@warpmail.net>
 */
#ifndef _SOFTWARE_UPDATER_APP_H
#define _SOFTWARE_UPDATER_APP_H


#include <Application.h>

#include "SoftwareUpdaterWindow.h"
#include "WorkingLooper.h"


class SoftwareUpdaterApp : public BApplication {
public:
							SoftwareUpdaterApp();
							~SoftwareUpdaterApp();

			void			ReadyToRun();
			void			MessageReceived(BMessage* message);

private:
			SoftwareUpdaterWindow*	fWindow;
			WorkingLooper*			fWorker;
};


#endif
