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
#include <OutlineListView.h>
#include <Point.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <StringView.h>
#include <Window.h>

#include "StripeView.h"

#define USE_PANE_SWITCH 0

namespace BPrivate {
	class PaneSwitch;
};
using namespace BPrivate;

enum {
	PACKAGE_UPDATE,
	PACKAGE_INSTALL,
	PACKAGE_UNINSTALL
};


class SuperItem : public BListItem {
public:
							SuperItem(const char* label);
							~SuperItem();
	virtual void			DrawItem(BView*, BRect, bool);
	virtual void			Update(BView *owner, const BFont *font);
	font_height				GetFontHeight() { return fFontHeight; };
	float					GetPackageItemHeight()
								{ return fPackageItemHeight; };
	BBitmap*				GetIcon() { return fPackageIcon; };
	float					GetIconSize() { return fIconSize; };

private:
			void			_GetPackageIcon();
			
			BString			fLabel;
			BFont			fRegularFont;
			BFont			fBoldFont;
			font_height		fFontHeight;
			float			fPackageItemHeight;
			BBitmap*		fPackageIcon;
			int16			fIconSize;
};


class PackageItem : public BListItem {
public:
							PackageItem(const char* name,
								const char* version,
								const char* summary,
								SuperItem* super);
//							~PackageItem();
	virtual void			DrawItem(BView*, BRect, bool);
	virtual void			Update(BView *owner, const BFont *font);
	void					SetItemHeight(const BFont* font);
	int						ICompare(PackageItem* item);
	
private:
			BString			fName;
			BString			fVersion;
			BString			fSummary;
			BFont			fRegularFont;
			BFont			fSmallFont;
			font_height		fSmallFontHeight;
			float			fSmallTotalHeight;
			float			fLabelOffset;
			SuperItem*		fSuperItem;
};


class PackageListView : public BOutlineListView {
public:
							PackageListView();
			virtual	void	FrameResized(float newWidth, float newHeight);
//			virtual BSize	PreferredSize();
//			virtual	void	GetPreferredSize(float* _width, float* _height);
//			virtual	BSize	MaxSize();
			void			AddPackage(uint32 install_type,
								const char* name,
								const char* version,
								const char* summary);
			void			SortItems();

private:
			SuperItem*		fSuperUpdateItem;
			SuperItem*		fSuperInstallItem;
			SuperItem*		fSuperUninstallItem;
};


class SoftwareUpdaterWindow : public BWindow {
public:
							SoftwareUpdaterWindow();
							~SoftwareUpdaterWindow();

			void			MessageReceived(BMessage* message);
			bool			ConfirmUpdates(const char* text /*,
								const BMessenger& target*/);
			void			UpdatesApplying(const char* header,
								const char* detail);
//			void			FinalUpdate(const char* header,
//								const char* detail);
			bool			UserCancelRequested();
			void			AddPackageInfo(uint32 install_type,
								const char* package_name,
								const char* cur_ver,
								const char* new_ver,
								const char* summary);
			const BBitmap*	GetIcon() { return fIcon; };
			BRect			GetDefaultRect() { return fDefaultRect; };
			BPoint			GetLocation() { return Frame().LeftTop(); };
			BLayoutItem*	layout_item_for(BView* view);

private:
			uint32			_WaitForButtonClick();
			void			_SetState(uint32 state);
			uint32			_GetState();
			
			BRect			fDefaultRect;
			StripeView*		fStripeView;
			BStringView*	fHeaderView;
			BStringView*	fDetailView;
//			BGroupView*		fInfoView;
			BButton*		fUpdateButton;
			BButton*		fCancelButton;
//			BButton*		fViewDetailsButton;
			BStatusBar*		fStatusBar;
#if USE_PANE_SWITCH
			PaneSwitch*		fPackagesSwitch;
			BLayoutItem*	fPkgSwitchLayoutItem;
#endif
			PackageListView*	fListView;
			BScrollView*	fScrollView;
			BLayoutItem*	fDetailsLayoutItem;
			BLayoutItem*	fPackagesLayoutItem;
			BLayoutItem*	fProgressLayoutItem;
			BLayoutItem*	fUpdateButtonLayoutItem;
//			BLayoutItem*	fDetailsButtonLayoutItem;
			BBitmap*		fIcon;
			
			uint32			fCurrentState;
			sem_id			fWaitingSem;
			bool			fWaitingForButton;
			uint32			fButtonResult;
			bool			fUserCancelRequested;
//			BMessenger		fWindowTarget;
			
};


int SortPackageItems(const BListItem* item1, const BListItem* item2);


class FinalWindow : public BWindow {
public:
							FinalWindow(BRect rect, BPoint location,
								const char* header, const char* detail);
							~FinalWindow();

			void			MessageReceived(BMessage* message);
private:
			
			StripeView*		fStripeView;
			BStringView*	fHeaderView;
			BStringView*	fDetailView;
			BButton*		fCancelButton;
			BBitmap*		fIcon;
};


#endif // _SOFTWARE_UPDATER_WINDOW_H
