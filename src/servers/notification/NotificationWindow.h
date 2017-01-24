/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2008-2009, Pier Luigi Fiorini. All Rights Reserved.
 * Copyright 2004-2008, Michael Davidson. All Rights Reserved.
 * Copyright 2004-2007, Mikael Eiman. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _NOTIFICATION_WINDOW_H
#define _NOTIFICATION_WINDOW_H

#include <cmath>
#include <vector>
#include <map>

#include <AppFileInfo.h>
#include <GroupView.h>
#include <ScrollView.h>
#include <String.h>
#include <Window.h>

#include "NotificationView.h"
#include "ScrollableGroupView.h"


class AppGroupView;
class AppUsage;

struct property_info;

typedef std::map<BString, AppGroupView*> appview_t;
typedef std::map<BString, AppUsage*> appfilter_t;

extern const float kEdgePadding;
extern const float kSmallPadding;
extern const float kCloseSize;
extern const float kExpandSize;
extern const float kPenSize;

const uint32 kRemoveGroupView = 'RGVi';
const uint32 kSettingsButtonClicked = 'SeCl';

enum {
	NEW_NOTIFICATIONS_WINDOW = 0,
	SHELVED_NOTIFICATIONS_WINDOW
};


class NotificationWindow : public BWindow {
public:
									NotificationWindow(uint32 type =
										NEW_NOTIFICATIONS_WINDOW);
	virtual							~NotificationWindow();

	virtual	bool					QuitRequested();
	virtual	void					MessageReceived(BMessage*);
	virtual	void					WorkspaceActivated(int32, bool);
	virtual	void					FrameResized(float width, float height);
	virtual	void					ScreenChanged(BRect frame, color_space mode);
	virtual	BHandler*				ResolveSpecifier(BMessage*, int32, BMessage*,
										int32, const char*);

			void					LoadSettings(BMessage& settings);
			icon_size				IconSize();
			int32					Timeout();
			uint32					Type();
			float					Width();


private:
	friend class AppGroupView;

			void					_SetPosition();
			void					_ResizeScrollbar();
			void					_ShowHide();
			void					_LoadAppFilters(BMessage& settings);
			void					_LoadGeneralSettings(BMessage& settings);
			void					_LoadDisplaySettings(BMessage& settings);
			void					_DrawDeskbarNewIcon();
			void					_DrawDeskbarStandardIcon();

			appview_t				fAppViews;
			appfilter_t				fAppFilters;

			BString					fStatusText;
			BString					fMessageText;

			float					fWidth;
			icon_size				fIconSize;
			int32					fTimeout;
			uint32					fType;
			ScrollableGroupView*	fContainerView;
		//	BGroupView*				fContainerView;
			BScrollBar*				fScrollBar;

			BMessenger				fDeskbarViewMessenger;
};

extern property_info main_prop_list[];

#endif	// _NOTIFICATION_WINDOW_H
