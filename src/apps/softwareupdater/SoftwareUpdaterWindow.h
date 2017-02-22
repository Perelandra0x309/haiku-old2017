/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Alexander von Gluck IV <kallisti5@unixzen.com>
 *		Brian Hill <supernova@warpmail.net>
 */
#ifndef _SOFTWARE_UPDATER_WINDOW_H
#define _SOFTWARE_UPDATER_WINDOW_H


#include <Button.h>
#include <GroupView.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextView.h>
#include <Window.h>

#include "StripeView.h"

class DetailsWindow : public BWindow {
public:
							DetailsWindow(const char* details);
			void			MessageReceived(BMessage* message);

private:
			BStringView*	fLabelView;
			BTextView*		fTextView;
			BScrollView*	fScrollView;
			BButton*		fCloseButton;
};


class SoftwareUpdaterWindow : public BWindow {
public:
							SoftwareUpdaterWindow();
							~SoftwareUpdaterWindow();
			bool			QuitRequested();

			void			MessageReceived(BMessage* message);
			bool			ConfirmUpdates(const char* text,
								const char* packageDetails);
			void			UpdatesApplying(const char* header,
								const char* detail);
			void			FinalUpdate(const char* header,
								const char* detail, bool success = false);
			bool			UserCancelRequested();

private:
//			void			_Error(const char* error);
			uint32			_WaitForButtonClick();
			void			_SetState(uint32 state);
			uint32			_GetState();
			
			StripeView*		fStripeView;
			BStringView*	fHeaderView;
			BStringView*	fDetailView;
			BGroupView*		fInfoView;
			BButton*		fUpdateButton;
			BButton*		fCancelButton;
			BButton*		fViewDetailsButton;
			
			uint32			fCurrentState;
			sem_id			fWaitingSem;
			bool			fWaitingForButton;
			uint32			fButtonResult;
			bool			fUserCancelRequested;
			const char*		fPackageDetails;
			
};


#endif
