/*
 * Copyright 2010-2015, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@warpmail.net
 */


#include <Application.h>
#include <NodeInfo.h>
#include <Roster.h>

#include "DeskbarShelfView.h"


DeskbarShelfView::DeskbarShelfView()
	:
	BView(BRect(0, 0, 15, 15), kShelfviewName, B_FOLLOW_NONE, B_WILL_DRAW),
	fIcon(NULL)
{
	printf("Here:Constructor\n");
	app_info info;
	be_app->GetAppInfo(&info);
	fIcon = new BBitmap(Bounds(), B_RGBA32);
	if (fIcon->InitCheck() == B_OK) {
		printf("Here:GetIcon\n");
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

//	SetToolTip(EL_TOOLTIP_TEXT);
}


DeskbarShelfView::DeskbarShelfView(BMessage* message)
	:
	BView(message),
	fIcon(NULL)
{
	printf("Here:Constructor(message)\n");
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if (result == B_OK)
		fIcon = new BBitmap(&iconArchive);
	// Apparently Haiku does not yet archive tool tips (Release 1 Alpha 3)
//	SetToolTip(EL_TOOLTIP_TEXT);
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
	printf("Here:AttachedToWindow\n");
}


void
DeskbarShelfView::DetachedFromWindow()
{
	
}


DeskbarShelfView*
DeskbarShelfView::Instantiate(BMessage* data)
{
	printf("Here:Instantiate\n");
	if (!validate_instantiation(data, "DeskbarShelfView"))
		return NULL;
	return new DeskbarShelfView(data);
}


status_t
DeskbarShelfView::Archive(BMessage* data, bool deep) const
{
	printf("Here:Archive\n");
	BView::Archive(data, deep);
	data->AddString("add_on", "application/x-vnd.Haiku-notification_server");
//	data->AddString("class", "DeskbarShelfView");
	if (fIcon != NULL) {
		BMessage archive;
		fIcon->Archive(&archive);
		data->AddMessage("fIconArchive", &archive);
		printf("Here:SaveIcon\n");
	}
	return B_NO_ERROR;
}


void
DeskbarShelfView::Draw(BRect rect)
{
	printf("Here:Draw\n");
	if (fIcon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fIcon);
	SetDrawingMode(B_OP_COPY);
}


void
DeskbarShelfView::MessageReceived(BMessage* message)
{
	printf("Here:MessageReceived\n");
	switch(message->what)
	{
/*		case EL_SHELFVIEW_OPENPREFS:
		{
			if(be_roster->IsRunning(e_launcher_sig))
			{
				status_t rc = B_ERROR;
				BMessenger appMessenger(e_launcher_sig, -1, &rc);
				if(!appMessenger.IsValid())
					break;
				appMessenger.SendMessage(EL_SHOW_SETTINGS);
			}
			else
			{
				BMessage goToMessage(EL_SHOW_SETTINGS);
				be_roster->Launch(e_launcher_sig, &goToMessage);
			}
			break;
		}*/
		
		default:
			BView::MessageReceived(message);
	}
}


void
DeskbarShelfView::MouseDown(BPoint pos)
{
/*	ConvertToScreen(&pos);
	if (fMenu)
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);*/
}


void
DeskbarShelfView::_Quit()
{
	BDeskbar deskbar;
	deskbar.RemoveItem(kShelfviewName);
}
