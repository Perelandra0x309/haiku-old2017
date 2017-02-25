/*
 * Copyright 2016-2017 Haiku, Inc. All rights reserved.
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
#include <StatusBar.h>
#include <StringView.h>
#include <Window.h>

#include "StripeView.h"


class SoftwareUpdaterWindow : public BWindow {
public:
							SoftwareUpdaterWindow();
							~SoftwareUpdaterWindow();

			void			MessageReceived(BMessage* message);
			bool			ConfirmUpdates(const char* text,
								const BMessenger& target);
			void			UpdatesApplying(const char* header,
								const char* detail);
			void			FinalUpdate(const char* header,
								const char* detail);
			bool			UserCancelRequested();
			const BBitmap*	GetIcon() { return fIcon; };

private:
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
			BStatusBar*		fStatusBar;
			BBitmap*		fIcon;
			
			uint32			fCurrentState;
			sem_id			fWaitingSem;
			bool			fWaitingForButton;
			uint32			fButtonResult;
			bool			fUserCancelRequested;
			BMessenger		fWindowTarget;
			
};


#endif // _SOFTWARE_UPDATER_WINDOW_H
