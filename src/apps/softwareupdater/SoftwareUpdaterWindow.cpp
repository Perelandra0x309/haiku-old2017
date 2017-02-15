/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Alexander von Gluck IV <kallisti5@unixzen.com>
 *		Brian Hill <supernova@warpmail.net>
 */


#include "SoftwareUpdaterWindow.h"

#include <Alert.h>
#include <AppDefs.h>
#include <Application.h>
#include <Catalog.h>
#include <NodeInfo.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <String.h>
#include <package/manager/Exceptions.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SoftwareUpdater"


SoftwareUpdaterWindow::SoftwareUpdaterWindow()
	:
	BWindow(BRect(0, 0, 0, 300), "Software Update",
		B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE
		| B_NOT_CLOSABLE | B_NOT_RESIZABLE),
	fStripeView(NULL),
	fHeaderView(NULL),
	fDetailView(NULL),
	fUpdateButton(NULL),
	fCancelButton(NULL),
	fConfirmSem(-1),
	fConfirmed(false)
{
	BBitmap* icon = new BBitmap(BRect(0, 0, 31, 31), 0, B_RGBA32);
	team_info teamInfo;
	get_team_info(B_CURRENT_TEAM, &teamInfo);
	app_info appInfo;
	be_roster->GetRunningAppInfo(teamInfo.team, &appInfo);
	BNodeInfo::GetTrackerIcon(&appInfo.ref, icon, B_LARGE_ICON);

	fStripeView = new StripeView(icon);

	fUpdateButton = new BButton(B_TRANSLATE("Update now"),
		new BMessage(kMsgConfirm));
	fUpdateButton->Hide();

	fCancelButton = new BButton(B_TRANSLATE("Cancel"),
		new BMessage(kMsgExit));

	fHeaderView = new BStringView("header",
		"Checking for updates...", B_WILL_DRAW);
//	fHeaderView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fDetailView = new BStringView("detail", "Contacting software repositories"
		" to check for package updates.", B_WILL_DRAW);
//	fDetailView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));

	BFont font;
	fHeaderView->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	font.SetSize(font.Size() * 1.5);
	fHeaderView->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(fStripeView)
		.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING)
			.SetInsets(0, B_USE_DEFAULT_SPACING,
				B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
			.AddGroup(B_HORIZONTAL, 0)
				.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
					.Add(fHeaderView)
					.Add(fDetailView)
				.End()
				.AddGlue()
			.End()
			.AddStrut(B_USE_DEFAULT_SPACING)
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
				.AddGlue()
				.Add(fCancelButton)
				.Add(fUpdateButton)
			.End()
		.End();
	CenterOnScreen();
	Show();
}


SoftwareUpdaterWindow::~SoftwareUpdaterWindow()
{
}


bool
SoftwareUpdaterWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
SoftwareUpdaterWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgExit:
		{
			// Check if we are waiting for update confirmation
			if (fConfirmSem > 0) {
				fConfirmed = false;
				delete_sem(fConfirmSem);
				fConfirmSem = -1;
			}
			else
				QuitRequested();
				// TODO quit gracefully
			//	throw BAbortedByUserException();
			break;
		}
		
		case kMsgUpdate:
		{
			BString header;
			BString detail;
			status_t result = message->FindString(kKeyHeader, &header);
			Lock();
			if (result == B_OK && header != fHeaderView->Text())
				fHeaderView->SetText(header.String());
			result = message->FindString(kKeyDetail, &detail);
			if (result == B_OK)
				fDetailView->SetText(detail.String());
			Unlock();
			break;
		}
		
		case kMsgConfirm:
			fConfirmed = true;
			delete_sem(fConfirmSem);
			fConfirmSem = -1;
			break;
		
		default:
			BWindow::MessageReceived(message);
	}
}


bool
SoftwareUpdaterWindow::ConfirmUpdates(const char* text)
{
	fConfirmSem = create_sem(0, "AlertSem");
	if (fConfirmSem < 0) {
		// Alert backup method
		Lock();
		fDetailView->SetText("");
		Unlock();
		BAlert* alert = new BAlert("Continue", text, "Cancel", "Update Now");
		int32 choice = alert->Go();
		return choice == 1;
	}
	Lock();
	fHeaderView->SetText("Updates found:");
	fDetailView->SetText(text);
	fUpdateButton->Show();
	fCancelButton->Show();
	Unlock();
	
	while (acquire_sem(fConfirmSem) == B_INTERRUPTED) {
	}
	return fConfirmed;
}


void
SoftwareUpdaterWindow::FinalUpdate(const char* header, const char* detail)
{
	Lock();
	fHeaderView->SetText(header);
	fDetailView->SetText(detail);
	fUpdateButton->Hide();
	fCancelButton->SetLabel("Quit");
	fCancelButton->Show();
	Unlock();
}


void
SoftwareUpdaterWindow::_Error(const char* error)
{
	Lock();
	fHeaderView->SetText("Error encountered!");
	fDetailView->SetText(error);
	fUpdateButton->Hide();
	fCancelButton->Show();
	Unlock();
}
