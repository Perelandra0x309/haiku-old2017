/*
 * Copyright 2017 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill
 */


#include "NotificationRow.h"


#include <Catalog.h>
#include <ColumnTypes.h>
#include <Notification.h>
#include <notification/Notifications.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotificationRow"


NotificationRow::NotificationRow(BMessage notificationData)
	:
	BRow()
{
	fInitStatus = B_ERROR;
	
	status_t result = notificationData.FindMessage(kNameNotificationMessage, &fNotificationMessage);
	if (result != B_OK)
		return;
	int32 timestamp;
	result = notificationData.FindInt32(kNameTimestamp, &timestamp);
	if (result != B_OK)
		return;
	bool wasShown;
	result = notificationData.FindBool(kNameWasShown, &wasShown);
	if (result != B_OK)
		return;
	
	BNotification notification(&fNotificationMessage);
	BString type;
	switch (notification.Type()) {
	case B_INFORMATION_NOTIFICATION:
		type = B_TRANSLATE("Information");
		break;
	case B_IMPORTANT_NOTIFICATION:
		type = B_TRANSLATE("Important");
		break;
	case B_ERROR_NOTIFICATION:
		type = B_TRANSLATE("Error");
		break;
	case B_PROGRESS_NOTIFICATION:
		type = B_TRANSLATE("Progress");
		break;
	default:
		type = B_TRANSLATE("Unknown");
	}
	
	SetField(new BStringField(notification.Title()), kTitleIndex);
	SetField(new BDateField(&timestamp), kDateIndex);
	SetField(new BStringField(notification.Content()), kContentIndex);
	SetField(new BStringField(type), kTypeIndex);
	SetField(new BStringField(wasShown ?
		B_TRANSLATE("True") : B_TRANSLATE("False")), kShownIndex);
	
	fInitStatus = B_OK;
}
