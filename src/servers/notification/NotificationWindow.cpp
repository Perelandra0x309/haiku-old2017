/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2008-2009, Pier Luigi Fiorini. All Rights Reserved.
 * Copyright 2004-2008, Michael Davidson. All Rights Reserved.
 * Copyright 2004-2007, Mikael Eiman. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Mikael Eiman, mikael@eiman.tv
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */
#include "NotificationWindow.h"

#include <algorithm>

#include <Alert.h>
#include <Button.h>
#include <Application.h>
#include <Catalog.h>
#include <Deskbar.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <NodeMonitor.h>
#include <Notifications.h>
#include <Path.h>
#include <PropertyInfo.h>
#include <Screen.h>
#include <ScrollBar.h>
#include <SeparatorView.h>

#include "AppGroupView.h"
#include "AppUsage.h"
#include "DeskbarShelfView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotificationWindow"


property_info main_prop_list[] = {
	{"message", {B_GET_PROPERTY, 0}, {B_INDEX_SPECIFIER, 0},
		"get a message"},
	{"message", {B_COUNT_PROPERTIES, 0}, {B_DIRECT_SPECIFIER, 0},
		"count messages"},
	{"message", {B_CREATE_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0},
		"create a message"},
	{"message", {B_SET_PROPERTY, 0}, {B_INDEX_SPECIFIER, 0},
		"modify a message"},
	{0}
};


const float kCloseSize				= 6;
const float kExpandSize				= 8;
const float kPenSize				= 1;
const float kEdgePadding			= 2;
const float kSmallPadding			= 2;
const float kClosePadding			= 3 * kEdgePadding;


CenterLabelView::CenterLabelView(const char *name, const char *text)
	:
	BStringView(name, text)
{
	BFont labelFont(be_plain_font);
	labelFont.SetFace(B_BOLD_FACE);
	SetFont(&labelFont, B_FONT_FACE);
	SetExplicitAlignment(BAlignment(B_ALIGN_LEFT,
		B_ALIGN_VERTICAL_UNSET));
	SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}


void
CenterLabelView::MouseDown(BPoint /*point*/)
{
	Window()->Hide();
}


NotificationWindow::NotificationWindow(uint32 type)
	:
	BWindow(BRect(0, 0, -1, -1), B_TRANSLATE_MARK("Notification"),
		B_BORDERED_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL, B_AVOID_FRONT
		/*| B_AVOID_FOCUS*/ | B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE
		| B_NOT_RESIZABLE | B_NOT_MOVABLE /*| B_AUTO_UPDATE_SIZE_LIMITS*/ ,
		B_ALL_WORKSPACES),
	fType(type),
	fContainerView(NULL),
	fScrollBar(NULL)
{
	if (fType == NEW_NOTIFICATIONS_WINDOW) {
		SetFlags(Flags() | B_AVOID_FOCUS | B_AUTO_UPDATE_SIZE_LIMITS);
		SetLayout(new BGroupLayout(B_VERTICAL, 0));
	}
	else {
		AddShortcut('w', B_COMMAND_KEY, new BMessage(kHideButtonClicked));
		AddShortcut('q', B_COMMAND_KEY, new BMessage(kHideButtonClicked));
			// prevent quitting the server- hide window instead
		
		// Notification Center header label and buttons
		CenterLabelView* label = new CenterLabelView("label",
			B_TRANSLATE("Notification Center"));
		
		// TODO use bitmap buttons instead of text
		BButton* settingsButton = new BButton(B_TRANSLATE("Settings"),
			new BMessage(kSettingsButtonClicked));
		settingsButton->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT,
			B_ALIGN_VERTICAL_UNSET));
		settingsButton->SetFlat(true);
		
/*		BButton* hideButton = new BButton(">", new BMessage(kHideButtonClicked));
		float buttonHeight;
		settingsButton->GetPreferredSize(NULL, &buttonHeight);
			// make hide button square with the same height as settings button
		BSize btnSize(buttonHeight, buttonHeight);
		hideButton->SetExplicitSize(btnSize);
		hideButton->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT,
			B_ALIGN_VERTICAL_UNSET));
		hideButton->SetFlat(true);*/
		
		// Container view for notifications and the scrollbar
//		fContainerView = new BGroupView(B_VERTICAL, 0); // testing
		fContainerView = new GroupContainerView();
		//fContainerView->GetLayout()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, 200));
		//fContainerView->InvalidateLayout();

		fScrollBar = new BScrollBar("scrollbar", fContainerView,
			0, 0, B_VERTICAL);
		fScrollBar->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT,
			B_ALIGN_VERTICAL_UNSET));
		fScrollBar->Hide();

		BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
			.AddGroup(B_HORIZONTAL, 0)
				.SetInsets(B_USE_DEFAULT_SPACING, 0, 0, 0)
				.Add(label)
				.Add(new BSeparatorView(B_VERTICAL))
				.Add(settingsButton)
			//	.Add(hideButton)
			.End()
			.Add(new BSeparatorView(B_HORIZONTAL))
			.AddGroup(B_HORIZONTAL, 0)
				.Add(fContainerView)
				.Add(fScrollBar)
			.End()
			.AddGlue()
		.End();

		//GetLayout()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, 200));
//	UpdateSizeLimits();
//	InvalidateLayout();
//	Layout(true);
	//ResizeTo(Bounds().Width(), 200);
		//SetSizeLimits(B_SIZE_UNSET, B_SIZE_UNSET, B_SIZE_UNSET, 200);
	}

	// Start the message loop
	Hide();
	Show();
}


NotificationWindow::~NotificationWindow()
{
	appfilter_t::iterator aIt;
	for (aIt = fAppFilters.begin(); aIt != fAppFilters.end(); aIt++)
		delete aIt->second;
}


bool
NotificationWindow::QuitRequested()
{
	appview_t::iterator aIt;
	for (aIt = fAppViews.begin(); aIt != fAppViews.end(); aIt++) {
		aIt->second->RemoveSelf();
		delete aIt->second;
	}

	BMessenger(be_app).SendMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

/*
void
NotificationWindow::WindowActivated(bool active)
{
	if (fType == SHELVED_NOTIFICATIONS_WINDOW && !active)
		Hide();
}*/


void
NotificationWindow::WorkspaceActivated(int32 /*workspace*/, bool active)
{
	// Ensure window is in the correct position
	if (active)
		_SetPosition();
}


void
NotificationWindow::FrameResized(float width, float height)
{
	_SetPosition();
}


void
NotificationWindow::ScreenChanged(BRect frame, color_space mode)
{
	_SetPosition();
}


void
NotificationWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_COUNT_PROPERTIES:
		{
			BMessage reply(B_REPLY);
			BMessage specifier;
			const char* property = NULL;
			bool messageOkay = true;

			if (message->FindMessage("specifiers", 0, &specifier) != B_OK)
				messageOkay = false;
			if (specifier.FindString("property", &property) != B_OK)
				messageOkay = false;
			if (strcmp(property, "message") != 0)
				messageOkay = false;

			if (messageOkay)
				reply.AddInt32("result", fAppViews.size());
			else {
				reply.what = B_MESSAGE_NOT_UNDERSTOOD;
				reply.AddInt32("error", B_ERROR);
			}

			message->SendReply(&reply);
			break;
		}

		case B_CREATE_PROPERTY:
		case kNotificationMessage:
		{
			BMessage reply(B_REPLY);
			BNotification* notification = new BNotification(message);

			if (notification->InitCheck() == B_OK) {
				bigtime_t timeout;
				if (message->FindInt64("timeout", &timeout) != B_OK)
					timeout = -1;
				BMessenger messenger = message->ReturnAddress();
				app_info info;

				if (messenger.IsValid())
					be_roster->GetRunningAppInfo(messenger.Team(), &info);
				else
					be_roster->GetAppInfo("application/x-vnd.Be-SHEL", &info);

				bool allow = false;
				appfilter_t::iterator it = fAppFilters.find(info.signature);

				if (it == fAppFilters.end()) {
					AppUsage* appUsage = new AppUsage(notification->Group(),
						true);

					appUsage->Allowed(notification->Title(),
							notification->Type());
					fAppFilters[info.signature] = appUsage;
					allow = true;
				} else {
					allow = it->second->Allowed(notification->Title(),
						notification->Type());
				}

				if (allow) {
					BString groupName(notification->Group());
					appview_t::iterator aIt = fAppViews.find(groupName);
					AppGroupView* group = NULL;
					if (aIt == fAppViews.end()) {
						group = new AppGroupView(this,
							groupName == "" ? NULL : groupName.String());
						fAppViews[groupName] = group;
						GetLayout()->AddView(group);
					} else
						group = aIt->second;

					NotificationView* view = new NotificationView(this,
						notification, timeout);

					group->AddInfo(view);

					_ShowHide();

					reply.AddInt32("error", B_OK);
				} else
					reply.AddInt32("error", B_NOT_ALLOWED);
			} else {
				reply.what = B_MESSAGE_NOT_UNDERSTOOD;
				reply.AddInt32("error", B_ERROR);
			}

			message->SendReply(&reply);
			break;
		}

		case kAddViewToCenter:
		{
			NotificationView* view = NULL;
			if (message->FindPointer("view", (void**)&view) != B_OK)
				return;

		// TODO can we assume it has already gone through filtering?
/*			bool allow = false;
		appfilter_t::iterator it = fAppFilters.find(info.signature);

		if (it == fAppFilters.end()) {
			AppUsage* appUsage = new AppUsage(notification->Group(),
				true);

			appUsage->Allowed(notification->Title(),
					notification->Type());
			fAppFilters[info.signature] = appUsage;
			allow = true;
		} else {
			allow = it->second->Allowed(notification->Title(),
				notification->Type());
		}

		if (allow) {*/
			BString groupName(view->Group());
			appview_t::iterator aIt = fAppViews.find(groupName);
			AppGroupView* group = NULL;
			if (aIt == fAppViews.end()) {
				group = new AppGroupView(this,
					groupName == "" ? NULL : groupName.String());
				fAppViews[groupName] = group;
				fContainerView->AddGroup(group);
	//			fContainerView->InvalidateLayout();
			} else
				group = aIt->second;

			view->SetType(SHELVED_NOTIFICATION);
			group->AddInfo(view);
			view->Invalidate();
			_DrawDeskbarNewIcon();

	//		fContainerView->GetLayout()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, 200));
	//		fContainerView->InvalidateLayout();
	//		_ResizeScrollbar();
			//_ShowHide(); // TODO for testing
		}

		case kRemoveGroupView:
		{
			AppGroupView* view = NULL;
			if (message->FindPointer("view", (void**)&view) != B_OK)
				return;

			// It's possible that between sending this message, and us receiving
			// it, the view has become used again, in which case we shouldn't
			// delete it.
			if (view->HasChildren())
				return;

			// this shouldn't happen
			if (fAppViews.erase(view->Group()) < 1)
				break;

			view->RemoveSelf();
			delete view;

			_ShowHide();
			break;
		}

		case kSettingsButtonClicked:
			be_roster->Launch("application/x-vnd.Haiku-Notifications");
			break;
		
		case kHideButtonClicked:
			Hide();
			break;
			
		case B_KEY_DOWN:
		{
			char byte;
			if (message->FindInt8("byte", (int8*)&byte) == B_OK) {
				// Hide notification center window when ESC key pressed
				if (fType == SHELVED_NOTIFICATIONS_WINDOW && byte == B_ESCAPE)
					Hide();
			}
			break;
		}
		
		case kDeskbarReplicantClicked:
			if (IsHidden()) {
				_SetPosition();
				Show();
			}
			else
				Hide();
			break;

		case kDeskbarRegistration:
		{
			status_t result = message->FindMessenger(kKeyMessenger,
				&fDeskbarViewMessenger);
			if (result != B_OK) {
				BAlert* alert = new BAlert("error",
					B_TRANSLATE("The Notification server could not "
					"successfully connect to its deskbar replicant.  The "
					"replicant will not indicate when new notifications are "
					"in the Notification Center"), B_TRANSLATE("OK"),
					NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
				alert->SetShortcut(0, B_ESCAPE);
				alert->Go(NULL);
			}
			break;
		}

		default:
			BWindow::MessageReceived(message);
	}
}


BHandler*
NotificationWindow::ResolveSpecifier(BMessage* msg, int32 index,
	BMessage* spec, int32 form, const char* prop)
{
	BPropertyInfo prop_info(main_prop_list);
	BHandler* handler = NULL;

	if (strcmp(prop,"message") == 0) {
		switch (msg->what) {
			case B_CREATE_PROPERTY:
			{
				msg->PopSpecifier();
				handler = this;
				break;
			}
			case B_SET_PROPERTY:
			case B_GET_PROPERTY:
			{
				int32 i;

				if (spec->FindInt32("index", &i) != B_OK)
					i = -1;

				if (i >= 0 && i < (int32)fAppViews.size()) {
					msg->PopSpecifier();
				//	handler = fAppViews[i]; //TODO fix this
				} else
					handler = NULL;
				break;
			}
			case B_COUNT_PROPERTIES:
				msg->PopSpecifier();
				handler = this;
				break;
			default:
				break;
		}
	}

	if (!handler)
		handler = BWindow::ResolveSpecifier(msg, index, spec, form, prop);

	return handler;
}


icon_size
NotificationWindow::IconSize()
{
	return fIconSize;
}


int32
NotificationWindow::Timeout()
{
	return fTimeout;
}


uint32
NotificationWindow::Type()
{
	return fType;
}


float
NotificationWindow::Width()
{
	return fWidth;
}


void
NotificationWindow::_ShowHide()
{
	if (fAppViews.empty() && !IsHidden()) {
		Hide();
		_DrawDeskbarStandardIcon();
		return;
	}

	if (IsHidden()) {
		_SetPosition();
		Show();
	}
}


void
NotificationWindow::_SetPosition()
{
	BDeskbar deskbar;
	BRect frame = deskbar.Frame();
	
	if (fType == SHELVED_NOTIFICATIONS_WINDOW) {
		BScreen screen;
		float centerHeight = screen.Frame().Height() - frame.Height();
		GetLayout()->SetExplicitMaxSize(BSize(fWidth, centerHeight));
		ResizeTo(fWidth, centerHeight);
	}
	
	Layout(true);

	BRect bounds = DecoratorFrame();
	float width = Bounds().Width() + 1;
	float height = Bounds().Height() + 1;

	float leftOffset = Frame().left - bounds.left;
	float topOffset = Frame().top - bounds.top + 1;
	float rightOffset = bounds.right - Frame().right;
	float bottomOffset = bounds.bottom - Frame().bottom;
		// Size of the borders around the window

	float x = Frame().left, y = Frame().top;
		// If we can't guess, don't move...
	
	if (fType == NEW_NOTIFICATIONS_WINDOW) {
		switch (deskbar.Location()) {
			case B_DESKBAR_TOP:
				// Put it just under, top right corner
				y = frame.bottom + topOffset;
				x = frame.right - width + rightOffset;
				break;
			case B_DESKBAR_BOTTOM:
				// Put it just above, lower left corner
				y = frame.top - height - bottomOffset;
				x = frame.right - width + rightOffset;
				break;
			case B_DESKBAR_RIGHT_TOP:
				x = frame.left - width - rightOffset;
				y = frame.top - topOffset + 1;
				break;
			case B_DESKBAR_LEFT_TOP:
				x = frame.right + leftOffset;
				y = frame.top - topOffset + 1;
				break;
			case B_DESKBAR_RIGHT_BOTTOM:
				y = frame.bottom - height + bottomOffset;
				x = frame.left - width - rightOffset;
				break;
			case B_DESKBAR_LEFT_BOTTOM:
				y = frame.bottom - height + bottomOffset;
				x = frame.right + leftOffset;
				break;
			default:
				break;
		}
	}
	else {
		switch (deskbar.Location()) {
			case B_DESKBAR_TOP:
			case B_DESKBAR_RIGHT_TOP:
				// Put it just under, top right corner
				y = frame.bottom + topOffset;
				x = frame.right - width + rightOffset;
				break;
			case B_DESKBAR_BOTTOM:
			case B_DESKBAR_RIGHT_BOTTOM:
				// Put it just above, lower left corner
				y = frame.top - height - bottomOffset;
				x = frame.right - width + rightOffset;
				break;
			case B_DESKBAR_LEFT_TOP:
				x = 0;
				y = frame.bottom + topOffset;
				break;
			case B_DESKBAR_LEFT_BOTTOM:
				y = 0;
				x = 0;
				break;
			default:
				break;
		}
	}

	MoveTo(x, y);
}


void
NotificationWindow::_ResizeScrollbar()
{
	if (fScrollBar == NULL)
		return;

	BRect layoutArea = fContainerView->GroupLayout()->LayoutArea();
	float layoutHeight = layoutArea.Height();

	BLayoutItem* lastItem = fContainerView->GroupLayout()->ItemAt(
		fContainerView->GroupLayout()->CountItems() - 1);
	if (lastItem != NULL)
		layoutHeight = lastItem->Frame().bottom;

	float viewHeight = fContainerView->Bounds().Height();
	float max = layoutHeight- viewHeight;
	fScrollBar->SetRange(0, max);
	if (layoutHeight > 0)
		fScrollBar->SetProportion(viewHeight / layoutHeight);
	else
		fScrollBar->SetProportion(1);
}


void
NotificationWindow::LoadSettings(BMessage& settings)
{
	_LoadGeneralSettings(settings);
	_LoadDisplaySettings(settings);
	_LoadAppFilters(settings);
}


void
NotificationWindow::_LoadAppFilters(BMessage& settings)
{
	type_code type;
	int32 count = 0;

	if (settings.GetInfo("app_usage", &type, &count) != B_OK)
		return;

	for (int32 i = 0; i < count; i++) {
		AppUsage* app = new AppUsage();
		settings.FindFlat("app_usage", i, app);
		fAppFilters[app->Name()] = app;
	}
}


void
NotificationWindow::_LoadGeneralSettings(BMessage& settings)
{
	bool shouldRun;
	if (settings.FindBool(kAutoStartName, &shouldRun) == B_OK) {
		if (shouldRun == false) {
			// We should not start. Quit the app!
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
		}
	}
	if (settings.FindInt32(kTimeoutName, &fTimeout) != B_OK)
		fTimeout = kDefaultTimeout;
}


void
NotificationWindow::_LoadDisplaySettings(BMessage& settings)
{
	int32 setting;

	if (settings.FindFloat(kWidthName, &fWidth) != B_OK)
		fWidth = kDefaultWidth;
	GetLayout()->SetExplicitMaxSize(BSize(fWidth, B_SIZE_UNSET));
	GetLayout()->SetExplicitMinSize(BSize(fWidth, B_SIZE_UNSET));

	if (settings.FindInt32(kIconSizeName, &setting) != B_OK)
		fIconSize = kDefaultIconSize;
	else
		fIconSize = (icon_size)setting;
	
	InvalidateLayout();
	Lock();
	_SetPosition();
	Unlock();
		
	//TODO Notify the group views about the change?
}


void
NotificationWindow::_DrawDeskbarNewIcon()
{
	if (fDeskbarViewMessenger.IsValid())
		fDeskbarViewMessenger.SendMessage(kShowNewIcon);
}


void
NotificationWindow::_DrawDeskbarStandardIcon()
{
	if (fDeskbarViewMessenger.IsValid())
		fDeskbarViewMessenger.SendMessage(kShowStandardIcon);
}
