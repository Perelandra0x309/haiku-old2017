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
#include <ControlLook.h>
#include <LayoutUtils.h>
#include <NodeInfo.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Roster.h>
#include <String.h>

#include "constants.h"
//#include "tracker_private.h"
#include "DialogPane.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SoftwareUpdaterWindow"


BLayoutItem*
SoftwareUpdaterWindow::layout_item_for(BView* view)
{
	BLayout* layout = view->Parent()->GetLayout();
	int32 index = layout->IndexOfView(view);
	return layout->ItemAt(index);
}


SoftwareUpdaterWindow::SoftwareUpdaterWindow()
	:
	BWindow(BRect(-2000, -2000, -1800, -1800),
		B_TRANSLATE_SYSTEM_NAME("SoftwareUpdater"), B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE
		| B_NOT_CLOSABLE /*| B_NOT_RESIZABLE*/),
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
	fHeaderView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fHeaderView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));
	fDetailView = new BStringView("detail", B_TRANSLATE("Contacting software "
		"repositories to check for package updates."), B_WILL_DRAW);
	fDetailView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fDetailView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));
	fStatusBar = new BStatusBar("progress");
	fStatusBar->SetMaxValue(1.0);
	
	fPackagesSwitch = new PaneSwitch("options_button");
	fPackagesSwitch->SetLabels(B_TRANSLATE("Hide update details"),
		B_TRANSLATE("Show update details"));
	fPackagesSwitch->SetMessage(new BMessage(kMsgShowInfo));
	fPackagesSwitch->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED,
		B_SIZE_UNSET));
	fPackagesSwitch->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT,
		B_ALIGN_TOP));
	
	fListView = new PackageListView();
	fScrollView = new BScrollView("scrollview", fListView, B_WILL_DRAW,
		false, true);

	BFont font;
	fHeaderView->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	font.SetSize(font.Size() * 1.5);
	fHeaderView->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	
/*	fInfoView = new BGroupView();
	BLayoutBuilder::Group<>(fInfoView, B_VERTICAL)
		.Add(fHeaderView)
		.Add(fDetailView)
	.End();*/
	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(fStripeView)
		.AddGroup(B_VERTICAL, B_USE_ITEM_SPACING)
			.SetInsets(0, B_USE_WINDOW_SPACING,
				B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
	//		.AddGroup(B_HORIZONTAL, 0)
	//			.Add(fInfoView)
				.Add(fHeaderView)
				.Add(fDetailView)
	//			.AddGlue()
	//		.End()
			.AddGroup(new BGroupView(B_VERTICAL, B_USE_ITEM_SPACING))
				.Add(fStatusBar)
				.Add(fPackagesSwitch)
				.Add(fScrollView)
			.End()
	//		.AddStrut(B_USE_SMALL_SPACING)
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(fCancelButton)
				.Add(fViewDetailsButton)
				.Add(fUpdateButton)
			.End()
		.End()
	.End();
	
	fPackagesLayoutItem = layout_item_for(fScrollView);
	fPkgSwitchLayoutItem = layout_item_for(fPackagesSwitch);
	fPackagesLayoutItem->SetVisible(false);
//	fPkgSwitchLayoutItem->SetVisible(false);
	
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
		
		case kMsgShowInfo:
		{
			fPackagesLayoutItem->SetVisible(fPackagesSwitch->Value());
			break;
		}
		
		case 'inva':
		{
			// TODO resize window at final message to get rid of space at bottom
			
	/*		float minWidth;
			float maxWidth;
			InvalidateLayout();
			GetSizeLimits(&minWidth, &maxWidth, NULL, NULL);
			SetSizeLimits(minWidth, maxWidth, 0, 50);
			ResizeTo(Frame().Width(), 50);
			UpdateSizeLimits();
			InvalidateLayout();
			Layout(true);
			ResizeBy(0, -20);*/
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
	if (_GetState() > STATE_GET_CONFIRMATION)
		return false;
	
/*	if (fUserCancelRequested) {
		FinalUpdate(B_TRANSLATE("Updates cancelled"),
			B_TRANSLATE("No packages have been updated."));
		_WaitForButtonClick();
	}*/
	
	return fUserCancelRequested;
}


void
SoftwareUpdaterWindow::AddPackageInfo(const char* package_name,
	const char* cur_ver, const char* new_ver, const char* repo_name)
{
	Lock();
	fListView->AddItem(new PackageItem(package_name, new_ver, repo_name));
	// TODO move this elsewhere so it isn't run every time something is added
	fListView->SortItems(SortPackageItems);
	Unlock();
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
	/*	if (!fPackagesSwitch->IsHidden())
			fPackagesSwitch->Hide();
		if (!fScrollView->IsHidden())
			fScrollView->Hide();*/
//		fPackagesLayoutItem->SetVisible(false);
//		fPkgSwitchLayoutItem->SetVisible(false);
	}
	fCurrentState = state;
	
	// Update confirmation button
	// Show only when asking for confirmation to update
	if (fCurrentState == STATE_GET_CONFIRMATION) {
		if (fUpdateButton->IsHidden())
			fUpdateButton->Show();
	}
	else {
		if (!fUpdateButton->IsHidden())
			fUpdateButton->Hide();
	}
	
	// View details button and package info view
	// Show at confirmation prompt, hide at final update
	if (fCurrentState == STATE_GET_CONFIRMATION) {
		if (fViewDetailsButton->IsHidden())
			fViewDetailsButton->Show();
	//	if (fPackagesSwitch->IsHidden())
	//		fPackagesSwitch->Show();
		fPkgSwitchLayoutItem->SetVisible(true);
	}
	else if (fCurrentState == STATE_FINAL_MSG) {
		if (!fViewDetailsButton->IsHidden())
			fViewDetailsButton->Hide();
	/*	if (!fScrollView->IsHidden())
			fScrollView->Hide();
		if (!fPackagesSwitch->IsHidden())
			fPackagesSwitch->Hide();*/
		fPackagesLayoutItem->SetVisible(false);
		fPkgSwitchLayoutItem->SetVisible(false);
	}
	
	// Progress bar and string view
	// Hide detail text while showing status bar
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
	// Disable ability to cancel while updates are applying
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


PackageItem::PackageItem(const char* name, const char* version,
	const char* repository)
	:
	BListItem()
{
	fName.SetTo(name);
	fVersion.SetTo(B_TRANSLATE("Version:"));
	fVersion.Append(" ").Append(version);
	fRepository.SetTo(B_TRANSLATE("Repository:"));
	fRepository.Append(" ").Append(repository);
	fNameOffset = be_control_look->DefaultLabelSpacing();
}


void
PackageItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{
	float width, height;
    owner->GetPreferredSize(&width, &height);
    float nameWidth = width / 2.0;
//	rgb_color color;
//	bool selected = IsSelected();
	// Background redraw
/*	if(selected) {
		color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	}
	else {
		color = ui_color(B_LIST_BACKGROUND_COLOR);
	}
	owner->SetLowColor(color);
	owner->SetDrawingMode(B_OP_COPY);
	if(selected || complete)
	{	owner->SetHighColor(color);
		owner->FillRect(item_rect);
	}*/
	
	// Package name
	BFont detailsFont;
	owner->SetFont(&detailsFont);
    BString name(fName);
    owner->TruncateString(&name, B_TRUNCATE_END, nameWidth);
	owner->MovePenTo(BPoint(item_rect.left + fNameOffset, item_rect.bottom
		- fFontHeight.descent));
	owner->DrawString(name.String());
	
	// Repository and version
	detailsFont.SetSize(detailsFont.Size() - 2);
	
	owner->SetFont(&detailsFont);
	BPoint cursor(item_rect.left + fNameOffset + nameWidth + 1,
		item_rect.top + fFontHeight.ascent);
	owner->MovePenTo(cursor);
	owner->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR), 0.7));
	owner->DrawString(fRepository.String());
	cursor.y += fSmallTotalHeight;
	owner->MovePenTo(cursor);
	owner->DrawString(fVersion.String());
	
	owner->SetHighColor(tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR),
		B_DARKEN_1_TINT));
	owner->StrokeLine(item_rect.LeftBottom(), item_rect.RightBottom());
}


void
PackageItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);
	SetItemHeight(font);
}


void
PackageItem::SetItemHeight(const BFont* font)
{
	font->GetHeight(&fFontHeight);
	float height = fFontHeight.ascent + fFontHeight.descent
		+ fFontHeight.leading;
	SetHeight(2 * height);
	
	BFont smallFont(font);
	smallFont.SetSize(font->Size() - 2);
	smallFont.GetHeight(&fSmallFontHeight);
	fSmallTotalHeight = fSmallFontHeight.ascent + fSmallFontHeight.descent
		+ fSmallFontHeight.leading;
}


int
PackageItem::ICompare(PackageItem* item)
{
	// sort by package name
	return fName.ICompare(item->fName);
}


int
SortPackageItems(const void* item1, const void* item2)
{
	PackageItem* first = *(PackageItem**)item1;
	PackageItem* second = *(PackageItem**)item2;
	return (first->ICompare(second));
}


PackageListView::PackageListView()
	:
	BListView("Package list")
{
}


void
PackageListView::FrameResized(float newWidth, float newHeight)
{
	BListView::FrameResized(newWidth, newHeight);
	
	float count = CountItems();
	for (int32 i = 0; i < count; i++) {
		BListItem *item = ItemAt(i);
		item->Update(this, be_plain_font);
	}
	Invalidate();
}


void
PackageListView::GetPreferredSize(float* _width, float* _height)
{
	// TODO: Something more nice as default? I need to see how this looks
	// when there are actually any packages...
	if (_width != NULL)
		*_width = 400.0;

	if (_height != NULL)
		*_height = 80.0;
}


BSize
PackageListView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}
