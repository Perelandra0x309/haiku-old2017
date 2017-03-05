/*
 * Copyright 2017 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license
 *
 * Authors:
 *		Brian Hill <supernova@warpmail.net>
 */


#include "DetailsWindow.h"

#include <Catalog.h>
#include <ColumnTypes.h>
#include <LayoutBuilder.h>
#include <Message.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DetailsWindow"


static const BString kTitlePackageName = B_TRANSLATE_COMMENT("Package name",
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


DetailsWindow::DetailsWindow(/*const BMessenger& target*/)
	:
	BWindow(BRect(0, 0, 400, 400),
		B_TRANSLATE_SYSTEM_NAME("Update package details"),
		B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE | B_NOT_CLOSABLE),
	fCustomQuitFlag(false)
//	fStatusWindowMessenger(target)
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
	
//	fUpdateButton = new BButton(B_TRANSLATE("Update now"),
//		new BMessage(kMsgConfirm));
//	fUpdateButton->MakeDefault(true);
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
//			.Add(fUpdateButton)
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

/*
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
}*/


void
DetailsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgClose:
			Hide();
			break;
		
/*		case kMsgConfirm:
			Hide();
			if (fStatusWindowMessenger.IsValid())
				fStatusWindowMessenger.SendMessage(kMsgConfirm);
			break;*/
		
		case kMsgShow:
		{
			if (IsHidden()) {
				BRect rect;
				status_t result = message->FindRect(kKeyFrame, &rect);
				if (result == B_OK) {
					float tabOffset = DecoratorFrame().Height() - Bounds().Height();
					CenterIn(rect);
					MoveTo(Frame().left, rect.bottom + tabOffset);
				}
				Show();
			}
			break;
		}
		
		default:
			BWindow::MessageReceived(message);
	}
}


void
DetailsWindow::AddRow(const char* package_name, const char* cur_ver,
	const char* new_ver, const char* repo_name)
{
/*	PackageRow* addedRow = new PackageRow(package_name, cur_ver, new_ver,
		repo_name);
	Lock();
	fListView->AddRow(addedRow);
	Unlock();
	
	// Recalculate width needed for columns
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
		float width = fListView->ColumnAt(kNameColumn)->Width()
			+ fListView->ColumnAt(kCurrentColumn)->Width()
			+ fListView->ColumnAt(kNewColumn)->Width()
			+ fListView->ColumnAt(kRepoColumn)->Width() + 70;
		ResizeTo(width, Frame().Height());
		CenterOnScreen();
	}*/
}
