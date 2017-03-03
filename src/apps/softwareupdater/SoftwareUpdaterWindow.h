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
#include <ListView.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <StringView.h>
#include <Window.h>

#include "StripeView.h"


namespace BPrivate {
	class PaneSwitch;
};
using namespace BPrivate;


class PackageItem : public BListItem {
public:
							PackageItem(const char* text,
								const char* version,
								const char* repository);
//							~PackageItem();
	virtual void			DrawItem(BView*, BRect, bool);
	virtual void			Update(BView *owner, const BFont *font);
	void					SetItemHeight(const BFont* font);
	int						ICompare(PackageItem* item);
	
private:
			BString			fName;
			BString			fVersion;
			BString			fRepository;
			font_height		fFontHeight;
			font_height		fSmallFontHeight;
			float			fSmallTotalHeight;
			float			fNameOffset;
};


class PackageListView : public BListView {
public:
							PackageListView();
			virtual	void	FrameResized(float newWidth, float newHeight);
			virtual	void	GetPreferredSize(float* _width, float* _height);
			virtual	BSize	MaxSize();
};


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
			void			AddPackageInfo(const char* package_name,
								const char* cur_ver,
								const char* new_ver,
								const char* repo_name);
			const BBitmap*	GetIcon() { return fIcon; };
			BLayoutItem*	layout_item_for(BView* view);

private:
			uint32			_WaitForButtonClick();
			void			_SetState(uint32 state);
			uint32			_GetState();
			
			StripeView*		fStripeView;
			BStringView*	fHeaderView;
			BStringView*	fDetailView;
//			BGroupView*		fInfoView;
			BButton*		fUpdateButton;
			BButton*		fCancelButton;
			BButton*		fViewDetailsButton;
			BStatusBar*		fStatusBar;
			PaneSwitch*		fPackagesSwitch;
			PackageListView*	fListView;
			BScrollView*	fScrollView;
			BLayoutItem*	fPkgSwitchLayoutItem;
			BLayoutItem*	fPackagesLayoutItem;
			BBitmap*		fIcon;
			
			uint32			fCurrentState;
			sem_id			fWaitingSem;
			bool			fWaitingForButton;
			uint32			fButtonResult;
			bool			fUserCancelRequested;
			BMessenger		fWindowTarget;
			
};


int SortPackageItems(const void* item1, const void* item2);


#endif // _SOFTWARE_UPDATER_WINDOW_H
