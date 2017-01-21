/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@warpmail.net
 */


#include <Application.h>
#include <NodeInfo.h>
#include <Roster.h>

#include "DeskbarShelfView.h"

const char* kShelfviewName = "notifications_shelfview";

DeskbarShelfView::DeskbarShelfView()
	:
	BView(BRect(0, 0, 15, 15), kShelfviewName, B_FOLLOW_NONE, B_WILL_DRAW),
	fIcon(NULL)
{
	app_info info;
	be_app->GetAppInfo(&info);
	fIcon = new BBitmap(Bounds(), B_RGBA32);
	if (fIcon->InitCheck() == B_OK) {
		status_t result = BNodeInfo::GetTrackerIcon(&info.ref, fIcon, B_MINI_ICON);
		if(result != B_OK) {
			printf("Error getting tracker icon\n");
			delete fIcon;
			fIcon = NULL;
		}
	}
	else {
		printf("Error creating bitmap\n");
		delete fIcon;
		fIcon = NULL;
	}
}


DeskbarShelfView::DeskbarShelfView(BMessage* message)
	:
	BView(message),
	fIcon(NULL)
{
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if (result == B_OK)
		fIcon = new BBitmap(&iconArchive);
}


DeskbarShelfView::~DeskbarShelfView()
{
	delete fIcon;
}


void
DeskbarShelfView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());
	Invalidate();
}


void
DeskbarShelfView::DetachedFromWindow()
{

}


DeskbarShelfView*
DeskbarShelfView::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "DeskbarShelfView"))
		return NULL;
	return new DeskbarShelfView(data);
}


status_t
DeskbarShelfView::Archive(BMessage* data, bool deep) const
{
	BView::Archive(data, deep);
	data->AddString("add_on", "application/x-vnd.Haiku-notification_server");
	if (fIcon != NULL) {
		BMessage archive;
		fIcon->Archive(&archive);
		data->AddMessage("fIconArchive", &archive);
	}
	return B_NO_ERROR;
}


void
DeskbarShelfView::Draw(BRect rect)
{
	if (fIcon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fIcon);
	SetDrawingMode(B_OP_COPY);
}


void
DeskbarShelfView::MouseDown(BPoint pos)
{
	if(be_roster->IsRunning("application/x-vnd.Haiku-notification_server"))
	{
		status_t rc = B_ERROR;
		BMessenger appMessenger("application/x-vnd.Haiku-notification_server", -1, &rc);
		if(!appMessenger.IsValid())
			return;
		appMessenger.SendMessage(kDeskbarReplicantClicked);
	}
	else
		_Quit(); //TODO show alert?
}


void
DeskbarShelfView::_Quit()
{
	BDeskbar deskbar;
	deskbar.RemoveItem(kShelfviewName);
}
