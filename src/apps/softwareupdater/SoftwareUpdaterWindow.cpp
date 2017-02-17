/*
 * Copyright 2016 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Alexander von Gluck IV <kallisti5@unixzen.com>
 *		Brian Hill <supernova@warpmail.net>
 */


#include "SoftwareUpdaterWindow.h"

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
	BWindow(BRect(0, 0, 0, 300), B_TRANSLATE_SYSTEM_NAME("Software Update"),
		B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE
		| B_NOT_CLOSABLE | B_NOT_RESIZABLE),
	fStripeView(NULL),
	fHeaderView(NULL),
	fDetailView(NULL),
	fUpdateButton(NULL),
	fCancelButton(NULL),
	fWaitingSem(-1),
	fWaitingForButton(false),
	fUserCancelRequested(false)
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

	fCancelButton = new BButton("", new BMessage(kMsgCancel));

	fHeaderView = new BStringView("header",
		B_TRANSLATE("Checking for updates"), B_WILL_DRAW);
//	fHeaderView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fDetailView = new BStringView("detail", B_TRANSLATE("Contacting software "
		"repositories to check for package updates."), B_WILL_DRAW);
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
	
	_SetState(STATE_DISPLAY_STATUS);
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
		
		case kMsgUpdate:
		{
			if (fCurrentState != STATE_DISPLAY_STATUS)
				break;
			
			BString header;
			BString detail;
			Lock();
			status_t result = message->FindString(kKeyHeader, &header);
			if (result == B_OK && header != fHeaderView->Text())
				fHeaderView->SetText(header.String());
			result = message->FindString(kKeyDetail, &detail);
			if (result == B_OK)
				fDetailView->SetText(detail.String());
			Unlock();
			break;
		}
		
		case kMsgCancel:
		{
			if (fWaitingForButton) {
				fButtonResult = message->what;
				delete_sem(fWaitingSem);
				fWaitingSem = -1;
			}
			else if (fCurrentState == STATE_FINAL_MSG)
				QuitRequested();
			else {
				Lock();
				fHeaderView->SetText(B_TRANSLATE("Cancelling updates"));
				fDetailView->SetText(
					B_TRANSLATE("Attempting to cancel the updates..."));
				Unlock();
				fUserCancelRequested = true;
			}
			break;
		}
		
		case kMsgConfirm:
		{
			if (fWaitingForButton) {
				fButtonResult = message->what;
				delete_sem(fWaitingSem);
				fWaitingSem = -1;
			}
			break;
		}
		
		default:
			BWindow::MessageReceived(message);
	}
}


bool
SoftwareUpdaterWindow::ConfirmUpdates(const char* text)
{
	uint32 priorState = _GetState();
	_SetState(STATE_GET_CONFIRMATION);
	Lock();
	fUpdateButton->Show();
	 // TODO why isn't the button showing in _SetState?
	fHeaderView->SetText(B_TRANSLATE("Updates found"));
	fDetailView->SetText(text);
	Unlock();
	
	_WaitForButtonClick();
	_SetState(priorState);
	return fButtonResult == kMsgConfirm;
}


void
SoftwareUpdaterWindow::FinalUpdate(const char* header, const char* detail)
{
	Lock();
	fHeaderView->SetText(header);
	fDetailView->SetText(detail);
	Unlock();
	_SetState(STATE_FINAL_MSG);
}


bool
SoftwareUpdaterWindow::UserCancelRequested()
{
	if (fUserCancelRequested) {
		FinalUpdate(B_TRANSLATE("Updates cancelled"),
			B_TRANSLATE("No packages have been updated."));
		_WaitForButtonClick();
	}
	
	return fUserCancelRequested;
}

/*
void
SoftwareUpdaterWindow::_Error(const char* error)
{
	Lock();
	fHeaderView->SetText("Error encountered!");
	fDetailView->SetText(error);
	Unlock();
}
*/

uint32
SoftwareUpdaterWindow::_WaitForButtonClick()
{
	fButtonResult = 0;
	fWaitingForButton = true;
	fWaitingSem = create_sem(0, "WaitingSem");
	while (acquire_sem(fWaitingSem) == B_INTERRUPTED) {
	}
	fWaitingForButton = false;
	return fButtonResult;
}


void
SoftwareUpdaterWindow::_SetState(uint32 state)
{
	if (state <= STATE_HEAD || state >= STATE_TAIL)
		return;
	fCurrentState = state;
	
	Lock();
	// Update confirmation button
	if (state == STATE_GET_CONFIRMATION) {
		// TODO this isn't working, button doesn't show
		fUpdateButton->Show();
		//fUpdateButton->Invalidate();
	}
	else
		fUpdateButton->Hide();
	
	// Cancel button
	if (fCurrentState == STATE_FINAL_MSG)
		fCancelButton->SetLabel(B_TRANSLATE("Quit"));
	else
		fCancelButton->SetLabel(B_TRANSLATE("Cancel"));
	Unlock();
}


uint32
SoftwareUpdaterWindow::_GetState()
{
	return fCurrentState;
}
