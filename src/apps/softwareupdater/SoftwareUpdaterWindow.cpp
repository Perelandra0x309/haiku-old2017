/*
 * Copyright 2016-2017 Haiku, Inc. All rights reserved.
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
#include <String.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SoftwareUpdaterWindow"


SoftwareUpdaterWindow::SoftwareUpdaterWindow()
	:
	BWindow(BRect(0, 0, 0, 300), B_TRANSLATE_SYSTEM_NAME("SoftwareUpdater"),
		B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE
		| B_NOT_CLOSABLE | B_NOT_RESIZABLE),
	fStripeView(NULL),
	fHeaderView(NULL),
	fDetailView(NULL),
	fUpdateButton(NULL),
	fCancelButton(NULL),
	fViewDetailsButton(NULL),
	fStatusBar(NULL),
	fIcon(NULL),
	fCurrentState(STATE_HEAD),
	fWaitingSem(-1),
	fWaitingForButton(false),
	fUserCancelRequested(false)
{
	fIcon = new BBitmap(BRect(0, 0, 31, 31), 0, B_RGBA32);
	team_info teamInfo;
	get_team_info(B_CURRENT_TEAM, &teamInfo);
	app_info appInfo;
	be_roster->GetRunningAppInfo(teamInfo.team, &appInfo);
	BNodeInfo::GetTrackerIcon(&appInfo.ref, fIcon, B_LARGE_ICON);

	fStripeView = new StripeView(fIcon);

	fUpdateButton = new BButton(B_TRANSLATE("Update now"),
		new BMessage(kMsgConfirm));
	fUpdateButton->MakeDefault(true);
	fCancelButton = new BButton(B_TRANSLATE("Cancel"),
		new BMessage(kMsgCancel));
	fViewDetailsButton = new BButton(B_TRANSLATE("View details"),
		new BMessage(kMsgViewDetails));

	fHeaderView = new BStringView("header",
		B_TRANSLATE("Checking for updates"), B_WILL_DRAW);
//	fHeaderView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fDetailView = new BStringView("detail", B_TRANSLATE("Contacting software "
		"repositories to check for package updates."), B_WILL_DRAW);
//	fDetailView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fStatusBar = new BStatusBar("progress");
	fStatusBar->SetMaxValue(1.0);

	BFont font;
	fHeaderView->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	font.SetSize(font.Size() * 1.5);
	fHeaderView->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	
	fInfoView = new BGroupView();
	BLayoutBuilder::Group<>(fInfoView, B_VERTICAL)
		.Add(fHeaderView)
		.Add(fDetailView)
	.End();

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(fStripeView)
		.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING)
			.SetInsets(0, B_USE_DEFAULT_SPACING,
				B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
			.AddGroup(B_HORIZONTAL, 0)
				.Add(fInfoView)
				.AddGlue()
			.End()
			.Add(fStatusBar)
	//		.AddStrut(B_USE_SMALL_SPACING)
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
				.AddGlue()
				.Add(fCancelButton)
				.Add(fViewDetailsButton)
				.Add(fUpdateButton)
			.End()
		.End()
	.End();
	
	CenterOnScreen();
	Show();
	_SetState(STATE_DISPLAY_STATUS);
}


SoftwareUpdaterWindow::~SoftwareUpdaterWindow()
{
	delete fIcon;
}


void
SoftwareUpdaterWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		
		case kMsgTextUpdate:
		{
			if (fCurrentState == STATE_DISPLAY_PROGRESS)
				_SetState(STATE_DISPLAY_STATUS);
			else if (fCurrentState != STATE_DISPLAY_STATUS)
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
		
		case kMsgProgressUpdate:
		{
			if (fCurrentState == STATE_DISPLAY_STATUS)
				_SetState(STATE_DISPLAY_PROGRESS);
			else if (fCurrentState != STATE_DISPLAY_PROGRESS)
				break;
			
			BString packageName;
			status_t result = message->FindString(kKeyPackageName, &packageName);
			if (result != B_OK)
				break;
			BString packageCount;
			result = message->FindString(kKeyPackageCount, &packageCount);
			if (result != B_OK)
				break;
			float percent;
			result = message->FindFloat(kKeyPercentage, &percent);
			if (result != B_OK)
				break;
			
			BString header;
			Lock();
			result = message->FindString(kKeyHeader, &header);
			if (result == B_OK && header != fHeaderView->Text())
				fHeaderView->SetText(header.String());
			fStatusBar->SetTo(percent, packageName.String(),
				packageCount.String());
			Unlock();
			break;
		}
		
		case kMsgCancel:
		{
			if (fWaitingForButton) {
				fButtonResult = message->what;
				delete_sem(fWaitingSem);
				fWaitingSem = -1;
				if (fCurrentState == STATE_FINAL_MSG)
					be_app->PostMessage(B_QUIT_REQUESTED);
			}
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
		
		case kMsgViewDetails:
		{
			if (fWindowTarget.IsValid()) {
				BMessage showMessage(kMsgShow);
				showMessage.AddRect(kKeyFrame, Frame());
				fWindowTarget.SendMessage(&showMessage);
			}
			break;
		}
		
		case 'inva':
		{
			// TODO resize window at final message to get rid of space at bottom
			
			float minWidth;
			float maxWidth;
			InvalidateLayout();
			GetSizeLimits(&minWidth, &maxWidth, NULL, NULL);
			SetSizeLimits(minWidth, maxWidth, 0, 50);
			ResizeTo(Frame().Width(), 50);
			UpdateSizeLimits();
			InvalidateLayout();
			Layout(true);
			ResizeBy(0, -20);
			break;
		}
		
		default:
			BWindow::MessageReceived(message);
	}
}


bool
SoftwareUpdaterWindow::ConfirmUpdates(const char* text,
	const BMessenger& target)
{
	Lock();
	fHeaderView->SetText(B_TRANSLATE("Updates found"));
	fDetailView->SetText(text);
	Unlock();
	
	fWindowTarget = target;
	uint32 priorState = _GetState();
	_SetState(STATE_GET_CONFIRMATION);
	
	_WaitForButtonClick();
	_SetState(priorState);
	return fButtonResult == kMsgConfirm;
}


void
SoftwareUpdaterWindow::UpdatesApplying(const char* header, const char* detail)
{
	Lock();
	fHeaderView->SetText(header);
	fDetailView->SetText(detail);
	Unlock();
	_SetState(STATE_APPLY_UPDATES);
}


void
SoftwareUpdaterWindow::FinalUpdate(const char* header, const char* detail)
{
	if (_GetState() == STATE_FINAL_MSG)
		return;
	
	
	_SetState(STATE_FINAL_MSG);
	Lock();
	fHeaderView->SetText(header);
	fDetailView->SetText(detail);
	fStatusBar->RemoveSelf();
	delete fStatusBar;
	fStatusBar = NULL;
	Unlock();
	PostMessage('inva');
	_WaitForButtonClick();
}


bool
SoftwareUpdaterWindow::UserCancelRequested()
{
	if (_GetState() > STATE_DISPLAY_PROGRESS)
		return false;
	
/*	if (fUserCancelRequested) {
		FinalUpdate(B_TRANSLATE("Updates cancelled"),
			B_TRANSLATE("No packages have been updated."));
		_WaitForButtonClick();
	}*/
	
	return fUserCancelRequested;
}


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
	if (state <= STATE_HEAD || state >= STATE_MAX)
		return;
	
	Lock();
	// All these IsHidden() calls are needed because Hide/Show are cumulative
	
	if (fCurrentState == STATE_HEAD) {
		if (!fViewDetailsButton->IsHidden())
			fViewDetailsButton->Hide();
	}
	fCurrentState = state;
	
	// Update confirmation button
	if (fCurrentState == STATE_GET_CONFIRMATION) {
		if (fUpdateButton->IsHidden())
			fUpdateButton->Show();
	}
	else {
		if (!fUpdateButton->IsHidden())
			fUpdateButton->Hide();
	}
	
	// View details button
	if (fCurrentState == STATE_GET_CONFIRMATION) {
		if (fViewDetailsButton->IsHidden())
			fViewDetailsButton->Show();
	}
	else if (fCurrentState == STATE_FINAL_MSG) {
		if (!fViewDetailsButton->IsHidden())
			fViewDetailsButton->Hide();
	}
	
	// Progress bar and string view
	if (fCurrentState == STATE_DISPLAY_PROGRESS) {
	//	fDetailView->SetText("");
		if (!fDetailView->IsHidden())
			fDetailView->Hide();
		if (fStatusBar->IsHidden())
			fStatusBar->Show();
	}
	else {
		if (fDetailView->IsHidden())
			fDetailView->Show();
		if (!fStatusBar->IsHidden())
			fStatusBar->Hide();
	}
	
	// Cancel button
	if (fCurrentState == STATE_FINAL_MSG)
		fCancelButton->SetLabel(B_TRANSLATE("OK"));
	fCancelButton->SetEnabled(fCurrentState != STATE_APPLY_UPDATES);
	
//	InvalidateLayout(); // TODO resize window at final message to get rid of space at bottom
	
//	Layout(true);
//	UpdateSizeLimits();
	Unlock();
}


uint32
SoftwareUpdaterWindow::_GetState()
{
	return fCurrentState;
}
