/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill <supernova@warpmail.net>
 */


#include "WorkingLooper.h"

#include <Application.h>

//#undef B_TRANSLATION_CONTEXT
//#define B_TRANSLATION_CONTEXT "WorkingLooper"


WorkingLooper::WorkingLooper()
	:
	BLooper("WorkingLooper")
{
	Run();
	PostMessage(kMsgStart);
}

/*
WorkingLooper::~WorkingLooper()
{
}*/


bool
WorkingLooper::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
WorkingLooper::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgStart:
		{
			fAction.Perform();
		//	be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		
		default:
			BLooper::MessageReceived(message);
	}
}
