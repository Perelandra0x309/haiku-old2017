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
#include <ColumnListView.h>
#include <GroupView.h>
#include <StatusBar.h>
#include <StringView.h>
#include <TextView.h>
#include <Window.h>

#include "StripeView.h"



class PackageRow : public BRow {
public:
								PackageRow(const char* package_name,
									const char* cur_ver,
									const char* new_ver,
									const char* repo_name);
};


class DetailsWindow : public BWindow {
public:
							DetailsWindow(const BMessenger& target);
							~DetailsWindow();
			bool			QuitRequested();
			void			CustomQuit();
			void			MessageReceived(BMessage* message);
			void			AddRow(const char* package_name,
								const char* cur_ver,
								const char* new_ver,
								const char* repo_name);

private:
			BStringView*	fLabelView;
			BColumnListView*	fListView;
			BButton*		fUpdateButton;
			BButton*		fCloseButton;
			bool			fCustomQuitFlag;
			float			fPackageNameWidth;
			float			fCurVerWidth;
			float			fNewVerWidth;
			float			fRepoNameWidth;
			BMessenger		fStatusWindowMessenger;
};


class SoftwareUpdaterWindow : public BWindow {
public:
							SoftwareUpdaterWindow();

			void			MessageReceived(BMessage* message);
			bool			ConfirmUpdates(const char* text,
								const BMessenger& target);
			void			UpdatesApplying(const char* header,
								const char* detail);
			void			FinalUpdate(const char* header,
								const char* detail);
			bool			UserCancelRequested();

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
			
			uint32			fCurrentState;
			sem_id			fWaitingSem;
			bool			fWaitingForButton;
			uint32			fButtonResult;
			bool			fUserCancelRequested;
			BMessenger		fWindowTarget;
			
};


#endif // _SOFTWARE_UPDATER_WINDOW_H
