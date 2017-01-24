/*
 * Copyright 2010-2015, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */


#include "NotificationServer.h"

#include <stdlib.h>

#include <Alert.h>
#include <Beep.h>
#include <Catalog.h>
#include <File.h>
#include <FindDirectory.h>
#include <NodeMonitor.h>
#include <Notifications.h>
#include <Path.h>
#include <PropertyInfo.h>
#include <Roster.h>

#include "DeskbarShelfView.h"
#include "NotificationWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotificationServer"


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
	fWindow = new NotificationWindow();
	fCenter = new NotificationWindow(SHELVED_NOTIFICATIONS_WINDOW);

	_ShowShelfView(true);

	_LoadSettings();
}


void
NotificationServer::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_NODE_MONITOR:
		{
			_LoadSettings();
			break;
		}

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
		case kDeskbarRegistration:
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
NotificationServer::_LoadSettings()
{
	BPath path;
	BMessage settings;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append(kSettingsFile);
	BFile file(path.Path(), B_READ_ONLY);
	settings.Unflatten(&file);
	fWindow->LoadSettings(settings);
	fCenter->LoadSettings(settings);

	node_ref nref;
	BEntry entry(path.Path());
	entry.GetNodeRef(&nref);
	if (watch_node(&nref, B_WATCH_ALL, BMessenger(this)) != B_OK) {
		BAlert* alert = new BAlert(B_TRANSLATE("Warning"),
					B_TRANSLATE("Couldn't start general settings monitor.\n"
					"Live filter changes disabled."), B_TRANSLATE("OK"));
		alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
		alert->Go();
	}
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
