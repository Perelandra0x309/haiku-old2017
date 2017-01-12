/*
 * Copyright 2017 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill
 */
#ifndef NOTIFICATION_ROW_H
#define NOTIFICATION_ROW_H


#include <ColumnListView.h>
#include <Message.h>

enum {
	kTitleIndex,
	kDateIndex,
	kContentIndex,
	kTypeIndex,
	kShownIndex
};


class NotificationRow : public BRow {
public:
								NotificationRow(BMessage notificationData);
			status_t			InitStatus() { return fInitStatus; }
			BMessage			GetMessage() { return fNotificationMessage; }
private:
			status_t			fInitStatus;
			BMessage			fNotificationMessage;
};


#endif
