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
#include <LayoutBuilder.h>
#include <LayoutUtils.h>
#include <Message.h>
#include <NodeInfo.h>
#include <Roster.h>
#include <String.h>

#include "constants.h"
//#include "tracker_private.h"
#include "DialogPane.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SoftwareUpdaterWindow"


SoftwareUpdaterWindow::SoftwareUpdaterWindow()
	:
	BWindow(BRect(0, 0, 500, 100),
		B_TRANSLATE_SYSTEM_NAME("SoftwareUpdater"), B_TITLED_WINDOW,
		/*B_AUTO_UPDATE_SIZE_LIMITS |*/ B_NOT_ZOOMABLE
		| B_NOT_CLOSABLE /*| B_NOT_RESIZABLE*/),
	fStripeView(NULL),
	fHeaderView(NULL),
	fDetailView(NULL),
	fUpdateButton(NULL),
	fCancelButton(NULL),
//	fViewDetailsButton(NULL),
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
//	fUpdateButton->SetExplicitAlignment(BAlignment(B_ALIGN_HORIZONTAL_UNSET,
//		B_ALIGN_BOTTOM));
	fCancelButton = new BButton(B_TRANSLATE("Cancel"),
		new BMessage(kMsgCancel));
//	fCancelButton->SetExplicitAlignment(BAlignment(B_ALIGN_HORIZONTAL_UNSET,
//		B_ALIGN_BOTTOM));
	//fViewDetailsButton = new BButton(B_TRANSLATE("View details"),
	//	new BMessage(kMsgViewDetails));
//	fViewDetailsButton->SetExplicitAlignment(BAlignment(B_ALIGN_HORIZONTAL_UNSET,
//		B_ALIGN_BOTTOM));

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
	
#if USE_PANE_SWITCH
	fPackagesSwitch = new PaneSwitch("options_button");
	fPackagesSwitch->SetLabels(B_TRANSLATE("Hide update details"),
		B_TRANSLATE("Show update details"));
	fPackagesSwitch->SetMessage(new BMessage(kMsgShowInfo));
	fPackagesSwitch->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED,
		B_SIZE_UNSET));
	fPackagesSwitch->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT,
		B_ALIGN_TOP));
#endif
	
	fListView = new PackageListView();
	fScrollView = new BScrollView("scrollview", fListView, B_WILL_DRAW,
		false, true);

	BFont font;
	fHeaderView->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	font.SetSize(font.Size() * 1.5);
	fHeaderView->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	
	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(fStripeView)
		.AddGroup(B_VERTICAL, 0)
			.SetInsets(0, B_USE_WINDOW_SPACING,
				B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
			.AddGroup(new BGroupView(B_VERTICAL, B_USE_ITEM_SPACING))
				.Add(fHeaderView)
				.Add(fDetailView)
				.Add(fStatusBar)
#if USE_PANE_SWITCH
				.Add(fPackagesSwitch)
#endif
				.Add(fScrollView)
			.End()
			.AddStrut(B_USE_SMALL_SPACING)
			.AddGroup(new BGroupView(B_HORIZONTAL))
				.AddGlue()
				.Add(fCancelButton)
//				.Add(fViewDetailsButton)
				.Add(fUpdateButton)
			.End()
		.End()
	.End();
	
	fDetailsLayoutItem = layout_item_for(fDetailView);
	fProgressLayoutItem = layout_item_for(fStatusBar);
	fPackagesLayoutItem = layout_item_for(fScrollView);
#if USE_PANE_SWITCH
	fPkgSwitchLayoutItem = layout_item_for(fPackagesSwitch);
#endif
	fUpdateButtonLayoutItem = layout_item_for(fUpdateButton);
//	fDetailsButtonLayoutItem = layout_item_for(fViewDetailsButton);
	
	CenterOnScreen();
	Show();
	
	// TODO needs adjustment to consider larger fonts- do string width?
	fDefaultRect = Bounds();
	SetSizeLimits(fDefaultRect.Width(), fDefaultRect.Width(),
		fDefaultRect.Height(), fDefaultRect.Height());
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
/*		
		case kMsgViewDetails:
		{
			if (fWindowTarget.IsValid()) {
				BMessage showMessage(kMsgShow);
				showMessage.AddRect(kKeyFrame, Frame());
				fWindowTarget.SendMessage(&showMessage);
			}
			break;
		}*/

#if USE_PANE_SWITCH		
		case kMsgShowInfo:
		{
			fPackagesLayoutItem->SetVisible(fPackagesSwitch->Value());
			// TODO not sure this is working
/*			UpdateSizeLimits();
			InvalidateLayout();
			if (!fPackagesSwitch->Value()) {
				//ResizeTo(fDefaultRect.Width(), fDefaultRect.Height());
				float minWidth;
				float maxWidth;
				GetSizeLimits(&minWidth, &maxWidth, NULL, NULL);
				float height = fDefaultRect.Height();
				SetSizeLimits(minWidth, maxWidth, height, height);
				ResizeBy(0, -20);
			}*/
			break;
		}
#endif
		
	/*	case 'inva':
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
		}*/
		
		default:
			BWindow::MessageReceived(message);
	}
}


bool
SoftwareUpdaterWindow::ConfirmUpdates(const char* text /*,
	const BMessenger& target*/)
{
	Lock();
	fHeaderView->SetText(B_TRANSLATE("Updates found"));
	fDetailView->SetText(text);
	fListView->SortItems();
	Unlock();
	
//	fWindowTarget = target;
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
//	PostMessage('inva');
	_WaitForButtonClick();
}


bool
SoftwareUpdaterWindow::UserCancelRequested()
{
	if (_GetState() > STATE_GET_CONFIRMATION)
		return false;
	
	return fUserCancelRequested;
}


void
SoftwareUpdaterWindow::AddPackageInfo(uint32 install_type,
	const char* package_name, const char* cur_ver, const char* new_ver,
	const char* summary)
{
	// TODO remove cur_ver?
	BString version;
	if (new_ver == NULL)
		version.SetTo("");
	else
		version.SetTo(new_ver);
	Lock();
	fListView->AddPackage(install_type, package_name, version.String(),
		summary);
	Unlock();
}


BLayoutItem*
SoftwareUpdaterWindow::layout_item_for(BView* view)
{
	BLayout* layout = view->Parent()->GetLayout();
	int32 index = layout->IndexOfView(view);
	return layout->ItemAt(index);
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
	
	// Initial settings
	if (fCurrentState == STATE_HEAD) {
//		fDetailsButtonLayoutItem->SetVisible(false);
		fProgressLayoutItem->SetVisible(false);
		fPackagesLayoutItem->SetVisible(false);
	}
	fCurrentState = state;
	
	// Update confirmation button
	// Show only when asking for confirmation to update
	if (fCurrentState == STATE_GET_CONFIRMATION) 
		fUpdateButtonLayoutItem->SetVisible(true);
	else
		fUpdateButtonLayoutItem->SetVisible(false);
	
	// View details button and package info view
	// Show at confirmation prompt, hide at final update
	if (fCurrentState == STATE_GET_CONFIRMATION) {
//		fDetailsButtonLayoutItem->SetVisible(true);
#if !USE_PANE_SWITCH
		fPackagesLayoutItem->SetVisible(true);
#endif
		//fListView->ResizeToPreferred();
		SetSizeLimits(fDefaultRect.Width(), 9999,
		fDefaultRect.Height() + fListView->MinSize().Height() + 30, 9999);
		ResizeTo(Bounds().Width(), 400);
	}
	else if (fCurrentState == STATE_FINAL_MSG) {
//		fDetailsButtonLayoutItem->SetVisible(false);
#if USE_PANE_SWITCH
		fPackagesLayoutItem->SetVisible(false);
		fPkgSwitchLayoutItem->SetVisible(false);
#endif
		//fListView->SetExplicitMinSize(BSize(B_SIZE_UNSET, 40));
		fPackagesLayoutItem->SetVisible(false);
//		SetSizeLimits(fDefaultRect.Width(), fDefaultRect.Width(),
//		fDefaultRect.Height(), fDefaultRect.Height());
	}
	
	// Progress bar and string view
	// Hide detail text while showing status bar
	if (fCurrentState == STATE_DISPLAY_PROGRESS) {
		fDetailsLayoutItem->SetVisible(false);
		fProgressLayoutItem->SetVisible(true);
	}
	else {
		fProgressLayoutItem->SetVisible(false);
		fDetailsLayoutItem->SetVisible(true);
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


SuperItem::SuperItem(const char* label)
	:
	BListItem(),
	fLabel(label)
//	fPackageIcon(NULL)
{
}


SuperItem::~SuperItem()
{
//	delete fPackageIcon;
}


void
SuperItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{
	float width;
    owner->GetPreferredSize(&width, NULL);
    BString label(fLabel);
    owner->TruncateString(&label, B_TRUNCATE_END, width);
    owner->SetFont(&fBoldFont);
    owner->DrawString(label.String(), BPoint(item_rect.left,
		item_rect.bottom - fFontHeight.descent - 1));
	owner->SetFont(&fRegularFont);
	
//	owner->SetHighColor(tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR),
//		B_DARKEN_1_TINT));
//	owner->StrokeLine(BPoint(0, item_rect.bottom), item_rect.RightBottom());
}


void
SuperItem::Update(BView *owner, const BFont *font)
{
	fRegularFont = *font;
	fBoldFont = *font;
	fBoldFont.SetFace(B_BOLD_FACE);
	BListItem::Update(owner, &fBoldFont);
	
	// Calculate height for PackageItem
	fRegularFont.GetHeight(&fFontHeight);
	fPackageItemHeight = 2 * (fFontHeight.ascent + fFontHeight.descent
		+ fFontHeight.leading);
	
	// Calculate height for this item
	fBoldFont.GetHeight(&fFontHeight);
	SetHeight(fFontHeight.ascent + fFontHeight.descent
		+ fFontHeight.leading + 4);
	
//	_GetPackageIcon();
}

/*
// Save for later use!
void
SuperItem::_GetPackageIcon()
{
	delete fPackageIcon;
	fIconSize = 4 * int(fPackageItemHeight / 4);
		// Create icon size in multiples of 4

	status_t result = B_ERROR;
	BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
	fPackageIcon = new BBitmap(iconRect, 0, B_RGBA32);
	BMimeType nodeType;
	nodeType.SetTo("application/x-vnd.haiku-package");
	result = nodeType.GetIcon(fPackageIcon, icon_size(fIconSize));
	// Get super type icon
	if (result != B_OK) {
		BMimeType superType;
		if (nodeType.GetSupertype(&superType) == B_OK)
			result = superType.GetIcon(fPackageIcon, icon_size(fIconSize));
	}
	if (result != B_OK) {
		delete fPackageIcon;
		fPackageIcon = NULL;
	}
}*/


PackageItem::PackageItem(const char* name, const char* version,
	const char* summary, SuperItem* super)
	:
	BListItem(),
	fName(name),
	fVersion(version),
	fSummary(summary),
	fSuperItem(super)
{
	fLabelOffset = be_control_look->DefaultLabelSpacing();
}


void
PackageItem::DrawItem(BView* owner, BRect item_rect, bool complete)
{
	float width;
    owner->GetPreferredSize(&width, NULL);
    float nameWidth = width / 2.0;
    float offset_width = 0;
	
	// Save for use later!  Maybe we can get HaikuDepot icons?
/*	BBitmap* icon = fSuperItem->GetIcon();
	if (icon != NULL && icon->IsValid()) {
		float iconSize = fSuperItem->GetIconSize();
		float offsetMarginHeight = floor((Height() - iconSize) / 2);

		//owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmap(icon, BPoint(item_rect.left,
			item_rect.top + offsetMarginHeight));
		owner->SetDrawingMode(B_OP_COPY);
		offset_width += iconSize + fLabelOffset;
	}*/
	
	// Package name
	font_height fontHeight = fSuperItem->GetFontHeight();
    BString name(fName);
    owner->TruncateString(&name, B_TRUNCATE_END, nameWidth);
	BPoint cursor(item_rect.left + offset_width,
		item_rect.bottom - fSmallTotalHeight - fontHeight.descent - 1);
	owner->DrawString(name.String(), cursor);
	cursor.x += owner->StringWidth(name.String()) + fLabelOffset;
	
	// Change font and color
	owner->SetFont(&fSmallFont);
	owner->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR), 0.7));
	
	// Bullet
	float circleDiameter = 5.0;
	float circleLeft = item_rect.left - 2 * circleDiameter;
	float circleTop = cursor.y - circleDiameter - 1;
	owner->FillEllipse(BRect(circleLeft, circleTop,
		circleLeft + circleDiameter - 1, circleTop + circleDiameter - 1));
	
	// Version
	BString version(fVersion);
	owner->TruncateString(&version, B_TRUNCATE_END, width - cursor.x);
	owner->DrawString(version.String(), cursor);
	
	// Summary
	BString summary(fSummary);
	cursor.x = item_rect.left + offset_width;
	cursor.y = item_rect.bottom - fontHeight.descent;
	owner->TruncateString(&summary, B_TRUNCATE_END, width - cursor.x);
	owner->DrawString(summary.String(), cursor);
	
//	owner->SetHighColor(tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR),
//		B_DARKEN_1_TINT));
//	owner->StrokeLine(BPoint(0, item_rect.bottom), item_rect.RightBottom());

	owner->SetFont(&fRegularFont);
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
	SetHeight(fSuperItem->GetPackageItemHeight());
	
	fRegularFont = *font;
	fSmallFont = *font;
	fSmallFont.SetSize(font->Size() - 2);
	fSmallFont.GetHeight(&fSmallFontHeight);
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
SortPackageItems(const BListItem* item1, const BListItem* item2)
{
	PackageItem* first = (PackageItem*)item1;
	PackageItem* second = (PackageItem*)item2;
	return first->ICompare(second);
}


PackageListView::PackageListView()
	:
	BOutlineListView("Package list"),
	fSuperUpdateItem(NULL),
	fSuperInstallItem(NULL),
	fSuperUninstallItem(NULL)
{
	SetExplicitMinSize(BSize(B_SIZE_UNSET, 40));
	SetExplicitPreferredSize(BSize(B_SIZE_UNSET, 400));
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
PackageListView::AddPackage(uint32 install_type, const char* name,
	const char* version, const char* summary)
{
	SuperItem* super;
	switch (install_type) {
		case PACKAGE_UPDATE:
		{
			if (fSuperUpdateItem == NULL) {
				fSuperUpdateItem = new SuperItem(
					B_TRANSLATE("Packages to be updated"));
				AddItem(fSuperUpdateItem);
			}
			super = fSuperUpdateItem;
			break;
		}
		
		case PACKAGE_INSTALL:
		{
			if (fSuperInstallItem == NULL) {
				fSuperInstallItem = new SuperItem(
					B_TRANSLATE("New packages to be installed"));
				AddItem(fSuperInstallItem);
			}
			super = fSuperInstallItem;
			break;
		}
		
		case PACKAGE_UNINSTALL:
		{
			if (fSuperUninstallItem == NULL) {
				fSuperUninstallItem = new SuperItem(
					B_TRANSLATE("Packages to be uninstalled"));
				AddItem(fSuperUninstallItem);
			}
			super = fSuperUninstallItem;
			break;
		}
		
		default:
			return;
	
	}
	PackageItem* item = new PackageItem(name, version, summary, super);
	AddUnder(item, super);
}


void
PackageListView::SortItems()
{
	if (fSuperUpdateItem != NULL)
		SortItemsUnder(fSuperUpdateItem, true, SortPackageItems);
}

/*
BSize
PackageListView::PreferredSize()
{
	return BSize(B_SIZE_UNSET, 200);
}*/

/*
void
PackageListView::GetPreferredSize(float* _width, float* _height)
{
	// TODO: Something more nice as default? I need to see how this looks
	// when there are actually any packages...
	if (_width != NULL)
		*_width = 400.0;

	if (_height != NULL)
		*_height = 200.0;
}*/

/*
BSize
PackageListView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}
*/
