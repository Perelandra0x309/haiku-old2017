/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */


#include "HistoryView.h"

#include <Alert.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Notification.h>
#include <Path.h>
#include <TextControl.h>
#include <Window.h>

#include <notification/Notifications.h>
#include <notification/NotificationReceived.h>

#include "NotificationRow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "HistoryView"

const float kEdgePadding = 5.0;
const float kCLVTitlePadding = 8.0;

const int32 kGroupSelected = '_GSL';
const int32 kNotificationSelected = '_NSL';
const int32 kNotificationInvoked = '_NIN';

const int32 kCLVDeleteRow = 'av02';

// Applications column indexes
const int32 kGroupIndex = 0;
const int32 kCountIndex = 1;

// Notifications column indexes
//const int32 kTitleIndex = 0;
//const int32 kDateIndex = 1;
//const int32 kContentIndex = 2;
//const int32 kTypeIndex = 3;
//const int32 kShownIndex = 4;

//TODO undo duplication
const float kCloseSize				= 6;
const float kExpandSize				= 8;
const float kPenSize				= 1;
//const float kEdgePadding			= 2;
const float kSmallPadding			= 2;


HistoryView::HistoryView(SettingsHost* host)
	:
	SettingsPane("apps", host),
	fShowingPreview(false),
	fCurrentPreview(NULL)
{
	BRect rect(0, 0, 500, 100);

	// Search application field
	fSearch = new BTextControl(B_TRANSLATE("Search:"), NULL,
		new BMessage(kSettingChanged));

	// Applications list
	fGroups = new BColumnListView(B_TRANSLATE("Groups"),
		0, B_FANCY_BORDER, true);
	fGroups->SetSelectionMode(B_SINGLE_SELECTION_LIST);

	fGroupCol = new BStringColumn(B_TRANSLATE("Group"), 200,
		be_plain_font->StringWidth(B_TRANSLATE("Group")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fGroups->AddColumn(fGroupCol, kGroupIndex);

	fCountCol = new BStringColumn(B_TRANSLATE("Count"), 10,
		be_plain_font->StringWidth(B_TRANSLATE("Count")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fGroups->AddColumn(fCountCol, kCountIndex);

	// Notifications list
	fNotifications = new BColumnListView(B_TRANSLATE("Notifications"),
		0, B_FANCY_BORDER, true);
	fNotifications->SetSelectionMode(B_SINGLE_SELECTION_LIST);

	fTitleCol = new BStringColumn(B_TRANSLATE("Title"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Title")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fTitleCol, kTitleIndex);

	fDateCol = new BDateColumn(B_TRANSLATE("Last received"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Last received")) +
		(kCLVTitlePadding * 2), rect.Width(), B_ALIGN_LEFT);
	fNotifications->AddColumn(fDateCol, kDateIndex);
	
	fContentCol = new BStringColumn(B_TRANSLATE("Content"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Content")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fContentCol, kContentIndex);
	
	fTypeCol = new BStringColumn(B_TRANSLATE("Type"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Type")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fTypeCol, kTypeIndex);

	fShownCol = new BStringColumn(B_TRANSLATE("Shown"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Shown")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fShownCol, kShownIndex);
	
	// Preview view
	fGroupView = new AppGroupView(BMessenger(this),
		B_TRANSLATE("Notification Preview"));
							
	// Add views
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fSearch)
		.End()
		.Add(fGroups)
		.Add(fNotifications)
		.Add(fGroupView)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING,
			B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING);
}


HistoryView::~HistoryView()
{
	_UpdatePreview(NULL);
}


void
HistoryView::AttachedToWindow()
{
	fGroups->SetTarget(this);
	fGroups->SetSelectionMessage(new BMessage(kGroupSelected));

	fNotifications->SetTarget(this);
	fNotifications->SetSelectionMessage(new BMessage(kNotificationSelected));
//	fNotifications->SetInvocationMessage(new BMessage(kNotificationInvoked));

#if 0
	fNotifications->AddFilter(new BMessageFilter(B_ANY_DELIVERY,
		B_ANY_SOURCE, B_KEY_DOWN, CatchDelete));
#endif
}


void
HistoryView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kGroupSelected:
		{
			_UpdatePreview(NULL);
			BRow* row = fGroups->CurrentSelection();
			if (row == NULL)
				return;
			BStringField* groupName
				= dynamic_cast<BStringField*>(row->GetField(kGroupIndex));
			if (groupName == NULL)
				break;
			
			_PopulateNotifications(groupName->String());

/*			appusage_t::iterator it = fAppFilters.find(groupName->String());
			if (it != fAppFilters.end())
				_Populate(it->second);*/

			break;
		}
		case kNotificationSelected: {
			NotificationRow* row = dynamic_cast<NotificationRow*>(fNotifications->CurrentSelection());
			if (row == NULL)
				_UpdatePreview(NULL);
			else {
				
				NotificationView* newPreview = new NotificationView(
					row->GetNotification(), bigtime_t(10), B_MINI_ICON); //TODO change
				_UpdatePreview(newPreview);
			}
			break;
		}
//		case kNotificationInvoked: {
//			fShowPreview = !fShowPreview;
//			_UpdatePreview();
//			break;
//		}
		default:
			BView::MessageReceived(message);
			break;
	}
}


status_t
HistoryView::Load(BMessage& settings)
{
	_PopulateGroups();

	return B_OK;
}


status_t
HistoryView::Save(BMessage& storage)
{
/*	appusage_t::iterator fIt;
	for (fIt = fAppFilters.begin(); fIt != fAppFilters.end(); fIt++)
		storage.AddFlat("app_usage", fIt->second);
*/
	return B_OK;
}


void
HistoryView::_PopulateGroups()
{
	fGroups->Clear();

	BPath archivePath;
	find_directory(B_USER_CACHE_DIRECTORY, &archivePath);
	archivePath.Append("Notifications");
	BDirectory archiveDir(archivePath.Path());
	entry_ref entry;
	while (archiveDir.GetNextRef(&entry) == B_OK) {
		BFile archiveFile(&entry, B_READ_ONLY);
		BMessage archive;
		archive.Unflatten(&archiveFile);
		archiveFile.Unset();
		
		// Check if message archive is valid
		int32 count;
		if (!_ArchiveIsValid(archive, count))
			continue;
		
		BRow* row = new BRow();
		row->SetField(new BStringField(entry.name), kGroupIndex);
		BString countString;
		countString<<count;
		row->SetField(new BStringField(countString), kCountIndex);
		fGroups->AddRow(row);
	}
}


void
HistoryView::_PopulateNotifications(const char* group)
{
	fNotifications->Clear();
	
	// Get archive from file based on group
	BPath archivePath;
	find_directory(B_USER_CACHE_DIRECTORY, &archivePath);
	archivePath.Append("Notifications");
	archivePath.Append(group);
	BFile archiveFile(archivePath.Path(), B_READ_ONLY);
	BMessage archive;
	archive.Unflatten(&archiveFile);
	archiveFile.Unset();
	
	// Check if message archive is valid
	int32 count;
	if (!_ArchiveIsValid(archive, count))
		return;
	
	int32 index;
	for (index = 0; index < count; index++) {
		BMessage notificationData;
		status_t result = archive.FindMessage(kNameNotificationData, index, &notificationData);
		if (result != B_OK)
			continue;
		NotificationRow* row = new NotificationRow(notificationData);
		if (row->InitStatus() == B_OK)
			fNotifications->AddRow(row);
	}
}


bool
HistoryView::_ArchiveIsValid(BMessage& archive, int32& count)
{
	// Check message identifier
	if (archive.what != kNotificationsArchive)
		return false;
	// Check content
	type_code type;
	status_t result = archive.GetInfo(kNameNotificationData, &type, &count);
	if (result != B_OK || type != B_MESSAGE_TYPE || count < 1)
		return false;
	return true;	
}


void
HistoryView::_UpdatePreview(NotificationView* view)
{
	if (fShowingPreview && fCurrentPreview != NULL) {
		fCurrentPreview->RemoveSelf();
		delete fCurrentPreview;
	}
	if (view == NULL) {
		fCurrentPreview = NULL;
		fShowingPreview = false;
	} else {
		fGroupView->AddInfo(view);
		fCurrentPreview = view;
		fShowingPreview = true;
	}
}
