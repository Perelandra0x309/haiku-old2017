/*
 * Copyright 2017 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Brian Hill <supernova@warpmail.net>
 */
#ifndef _DETAILS_WINDOW_H
#define _DETAILS_WINDOW_H


#include <Button.h>
#include <ColumnListView.h>
#include <StringView.h>
#include <Window.h>


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


#endif // _DETAILS_WINDOW_H
