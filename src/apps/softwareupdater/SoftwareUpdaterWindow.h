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
#include <StringView.h>
#include <Window.h>

#include "StripeView.h"


class SoftwareUpdaterWindow : public BWindow {
public:
							SoftwareUpdaterWindow();
							~SoftwareUpdaterWindow();
			bool			QuitRequested();

			void			MessageReceived(BMessage* message);
			bool			ConfirmUpdates(const char* text);
			void			FinalUpdate(const char* header,
								const char* detail);

private:
			void			_Error(const char* error);
			
			StripeView*		fStripeView;
			BStringView*	fHeaderView;
			BStringView*	fDetailView;
			BButton*		fUpdateButton;
			BButton*		fCancelButton;
			
			sem_id			fConfirmSem;
			bool			fConfirmed;
};


#endif
