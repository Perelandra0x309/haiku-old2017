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
#include <Notification.h>

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
								~NotificationRow();
			status_t			InitStatus() { return fInitStatus; }
			BNotification*		GetNotification() { return fNotification; }
private:
			BNotification*		fNotification;
			status_t			fInitStatus;
};


#endif
