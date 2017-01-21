/*
 * Copyright 2010-2015, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */


#include "NotificationServer.h"

#include <stdlib.h>

#include <Beep.h>
#include <Notifications.h>
#include <PropertyInfo.h>
#include <Roster.h>

#include "DeskbarShelfView.h"
#include "NotificationWindow.h"


const char* kSoundNames[] = {
	"Information notification",
	"Important notification",
	"Error notification",
	"Progress notification",
	NULL
};


NotificationServer::NotificationServer(status_t& error)
	:
	BServer(kNotificationServerSignature, true, &error)
{
}


NotificationServer::~NotificationServer()
{
	_ShowShelfView(false);
}


void
NotificationServer::ReadyToRun()
{
	fWindow = new NotificationWindow(NEW_NOTIFICATIONS_WINDOW);
	fCenter = new NotificationWindow(SHELVED_NOTIFICATIONS_WINDOW);

	_ShowShelfView(true);
}


void
NotificationServer::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kNotificationMessage:
		{
			// Skip this message if we don't have the window
			if (!fWindow)
				return;

			int32 type = 0;

			// Emit a sound for this event
			if (message->FindInt32("type", &type) == B_OK) {
				if (type < (int32)(sizeof(kSoundNames) / sizeof(const char*)))
					system_beep(kSoundNames[type]);
			}

			// Let the notification window handle this message
			BMessenger(fWindow).SendMessage(message);
			break;
		}

		case kAddViewToCenter:
		case kDeskbarReplicantClicked:
		{
			fCenter->PostMessage(message);
			break;
		}

		default:
			BApplication::MessageReceived(message);
	}
}


status_t
NotificationServer::GetSupportedSuites(BMessage* msg)
{
	msg->AddString("suites", "suite/x-vnd.Haiku-notification_server");

	BPropertyInfo info(main_prop_list);
	msg->AddFlat("messages", &info);

	return BApplication::GetSupportedSuites(msg);
}


BHandler*
NotificationServer::ResolveSpecifier(BMessage* msg, int32 index,
	BMessage* spec, int32 from, const char* prop)
{
	BPropertyInfo info(main_prop_list);

	if (strcmp(prop, "message") == 0) {
		BMessenger messenger(fWindow);
		messenger.SendMessage(msg, fWindow);
		return NULL;
	}

	return BApplication::ResolveSpecifier(msg, index, spec, from, prop);
}


void
NotificationServer::_ShowShelfView(bool show)
{
	BDeskbar deskbar;
	// Don't add another DeskbarShelfView to the Deskbar if one is already
	// attached
	if (show && !deskbar.HasItem(kShelfviewName)) {
		BView* shelfView = new DeskbarShelfView();
		status_t status = deskbar.AddItem(shelfView);
		if (status < B_OK) {
			fprintf(stderr, "Can't add deskbar replicant: %s\n",
				strerror(status));
			return;
		}
		delete shelfView;
	}
	// Remove DeskbarShelfView if there is one in the deskbar
	else if (!show && deskbar.HasItem(kShelfviewName))
		deskbar.RemoveItem(kShelfviewName);
}


// #pragma mark -


int
main(int argc, char* argv[])
{
	int32 i = 0;

	// Add system sounds
	while (kSoundNames[i] != NULL)
		add_system_beep_event(kSoundNames[i++], 0);

	// Start!
	status_t error;
	NotificationServer server(error);
	if (error == B_OK)
		server.Run();

	return error == B_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
