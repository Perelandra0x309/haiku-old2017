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
#include <ColumnTypes.h>
#include <NodeInfo.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <String.h>
#include <package/manager/Exceptions.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SoftwareUpdaterWindow"


static const BString kTitlePackageName = B_TRANSLATE_COMMENT("Package Name",
	"Column title");
static const BString kTitleCurVer = B_TRANSLATE_COMMENT("Current version",
	"Column title");
static const BString kTitleNewVer = B_TRANSLATE_COMMENT("New version",
	"Column title");
static const BString kTitleRepo = B_TRANSLATE_COMMENT("Repository",
	"Column title");

enum {
	kNameColumn,
	kCurrentColumn,
	kNewColumn,
	kRepoColumn
};


PackageRow::PackageRow(const char* package_name, const char* cur_ver,
	const char* new_ver, const char* repo_name)
	:
	BRow()
{
	SetField(new BStringField(package_name), kNameColumn);
	SetField(new BStringField(cur_ver), kCurrentColumn);
	SetField(new BStringField(new_ver), kNewColumn);
	SetField(new BStringField(repo_name), kRepoColumn);
}


DetailsWindow::DetailsWindow(const BMessenger& target)
	:
	BWindow(BRect(0, 0, 400, 400),
		B_TRANSLATE_SYSTEM_NAME("Update package details"),
		B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
		B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE),
	fCustomQuitFlag(false),
	fStatusWindowMessenger(target)
{
	fLabelView = new BStringView("label", B_TRANSLATE("The following changes "
		"will be made:"));
	fListView = new BColumnListView("list", B_NAVIGABLE, B_PLAIN_BORDER);
	fPackageNameWidth = be_plain_font->StringWidth(kTitlePackageName) + 15;
	fCurVerWidth = be_plain_font->StringWidth(kTitleCurVer) + 15;
	fNewVerWidth = be_plain_font->StringWidth(kTitleNewVer) + 15;
	fRepoNameWidth = be_plain_font->StringWidth(kTitleRepo) + 15;
	fListView->AddColumn(new BStringColumn(kTitlePackageName,
		fPackageNameWidth, fPackageNameWidth, fPackageNameWidth,
		B_TRUNCATE_END), kNameColumn);
	fListView->AddColumn(new BStringColumn(kTitleCurVer, fCurVerWidth,
		fCurVerWidth, fCurVerWidth, B_TRUNCATE_END), kCurrentColumn);
	fListView->AddColumn(new BStringColumn(kTitleNewVer, fNewVerWidth,
		fNewVerWidth, fNewVerWidth, B_TRUNCATE_END), kNewColumn);
	fListView->AddColumn(new BStringColumn(kTitleRepo, fRepoNameWidth,
		fRepoNameWidth, fRepoNameWidth, B_TRUNCATE_END), kRepoColumn);
	
	fUpdateButton = new BButton(B_TRANSLATE("Update now"),
		new BMessage(kMsgConfirm));
	fUpdateButton->MakeDefault(true);
	fCloseButton = new BButton("Close", new BMessage(kMsgClose));
	fCloseButton->MakeDefault(true);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.AddGroup(B_HORIZONTAL)
			.Add(fLabelView)
			.AddGlue()
		.End()
		.Add(fListView)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fCloseButton)
			.Add(fUpdateButton)
		.End()
	.End();
	
	CenterOnScreen();
	Run();
	// TODO add ESC shortcut to close window
}


DetailsWindow::~DetailsWindow()
{
	BRow* row = fListView->RowAt((int32)0, NULL);
	while (row != NULL) {
		fListView->RemoveRow(row);
		delete row;
		row = fListView->RowAt((int32)0, NULL);
	}
}


bool
DetailsWindow::QuitRequested()
{
	Hide();
	if (fCustomQuitFlag)
		return BWindow::QuitRequested();
	return false;
}


void
DetailsWindow::CustomQuit()
{
	fCustomQuitFlag = true;
	QuitRequested();
}


void
DetailsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgClose:
			Hide();
			break;
		
		case kMsgConfirm:
			Hide();
			if (fStatusWindowMessenger.IsValid())
				fStatusWindowMessenger.SendMessage(kMsgConfirm);
			break;
		
		case kMsgShow:
			Show();
			break;
		
		default:
			BWindow::MessageReceived(message);
	}
}


void
DetailsWindow::AddRow(const char* package_name, const char* cur_ver,
	const char* new_ver, const char* repo_name)
{
	PackageRow* addedRow = new PackageRow(package_name, cur_ver, new_ver,
		repo_name);
	Lock();
	fListView->AddRow(addedRow);
	Unlock();
	float packageNameWidth = be_plain_font->StringWidth(package_name) + 15;
	float curVerWidth = be_plain_font->StringWidth(cur_ver) + 15;
	float newVerWidth = be_plain_font->StringWidth(new_ver) + 15;
	float repoNameWidth = be_plain_font->StringWidth(repo_name) + 15;
	bool widthChanged = false;
	if (packageNameWidth > fPackageNameWidth) {
		fPackageNameWidth = packageNameWidth;
		fListView->ColumnAt(kNameColumn)->SetWidth(fPackageNameWidth);
		widthChanged = true;
	}
	if (curVerWidth > fCurVerWidth) {
		fCurVerWidth = curVerWidth;
		fListView->ColumnAt(kCurrentColumn)->SetWidth(fCurVerWidth);
		widthChanged = true;
	}
	if (newVerWidth > fNewVerWidth) {
		fNewVerWidth = newVerWidth;
		fListView->ColumnAt(kNewColumn)->SetWidth(fNewVerWidth);
		widthChanged = true;
	}
	if (repoNameWidth > fRepoNameWidth) {
		fRepoNameWidth = repoNameWidth;
		fListView->ColumnAt(kRepoColumn)->SetWidth(fRepoNameWidth);
		widthChanged = true;
	}
	if (widthChanged) {
		float width;
		fListView->GetPreferredSize(&width, NULL);
		ResizeTo(width + 90, Frame().Height());
		CenterOnScreen();
	}
}


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
	fUpdateButton->MakeDefault(true);
	fUpdateButton->Hide();
	fCancelButton = new BButton("", new BMessage(kMsgCancel));
	fViewDetailsButton = new BButton(B_TRANSLATE("View details"),
		new BMessage(kMsgViewDetails));
	fViewDetailsButton->Hide();

	fHeaderView = new BStringView("header",
		B_TRANSLATE("Checking for updates"), B_WILL_DRAW);
//	fHeaderView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fDetailView = new BStringView("detail", B_TRANSLATE("Contacting software "
		"repositories to check for package updates."), B_WILL_DRAW);
//	fDetailView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fStatusBar = new BStatusBar("progress");
	fStatusBar->SetMaxValue(1.0);
	fStatusBar->Hide();

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
	
	_SetState(STATE_DISPLAY_STATUS);
	CenterOnScreen();
	Show();
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
			}
			else if (fCurrentState == STATE_FINAL_MSG)
				be_app->PostMessage(B_QUIT_REQUESTED);
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
			if (fWindowTarget.IsValid())
				fWindowTarget.SendMessage(kMsgShow);
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
	
	Lock();
	fHeaderView->SetText(header);
	fDetailView->SetText(detail);
	Unlock();
	_SetState(STATE_FINAL_MSG);
}


bool
SoftwareUpdaterWindow::UserCancelRequested()
{
	if (_GetState() > STATE_DISPLAY_PROGRESS)
		return false;
	
	if (fUserCancelRequested) {
		FinalUpdate(B_TRANSLATE("Updates cancelled"),
			B_TRANSLATE("No packages have been updated."));
		_WaitForButtonClick();
	}
	
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
	fCurrentState = state;
	
	Lock();
	// All these IsHidden() calls are needed because Hide/Show are cumulative
	
	// Update confirmation prompt buttons
	if (state == STATE_GET_CONFIRMATION) {
		if (fUpdateButton->IsHidden())
			fUpdateButton->Show();
		if (fViewDetailsButton->IsHidden())
			fViewDetailsButton->Show();
	}
	else {
		if (!fUpdateButton->IsHidden())
			fUpdateButton->Hide();
		if (!fViewDetailsButton->IsHidden())
		fViewDetailsButton->Hide();
	}
	
	// Progress bar and string view
	if (fCurrentState == STATE_DISPLAY_PROGRESS) {
		fDetailView->SetText("");
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
	else
		fCancelButton->SetLabel(B_TRANSLATE("Cancel"));
	fCancelButton->SetEnabled(fCurrentState != STATE_APPLY_UPDATES);
	
	InvalidateLayout(); // TODO resize window at final message to get rid of space at bottom
	Layout(true);
	UpdateSizeLimits();
	Unlock();
}


uint32
SoftwareUpdaterWindow::_GetState()
{
	return fCurrentState;
}
