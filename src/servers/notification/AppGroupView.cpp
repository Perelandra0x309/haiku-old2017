/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2008-2009, Pier Luigi Fiorini. All Rights Reserved.
 * Copyright 2004-2008, Michael Davidson. All Rights Reserved.
 * Copyright 2004-2007, Mikael Eiman. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Mikael Eiman, mikael@eiman.tv
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <algorithm>

#include <Application.h>
#include <GroupLayout.h>
#include <GroupView.h>

#include "AppGroupView.h"

#include "NotificationWindow.h"
#include "NotificationView.h"


AppGroupView::AppGroupView(NotificationWindow* win, const char* label)
	:
	BGroupView("appGroup", B_VERTICAL, 0),
	fLabel(label),
	fParent(win)
{
	fHeaderView = new GroupHeaderView(this, label, fParent->Type());
	GetLayout()->AddView(fHeaderView);
}


void
AppGroupView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kViewClosed:
		case kTimeoutExpired:
		{
			NotificationView* view = NULL;
			if (msg->FindPointer("view", (void**)&view) != B_OK)
				return;

			infoview_t::iterator vIt = find(fInfo.begin(), fInfo.end(), view);
			if (vIt == fInfo.end())
				break;

			fInfo.erase(vIt);
			view->RemoveSelf();

			if (msg->what != kTimeoutExpired)
				delete view;

			fParent->PostMessage(msg);

			if (!this->HasChildren()) {
				Hide();
				BMessage removeSelfMessage(kRemoveGroupView);
				removeSelfMessage.AddPointer("view", this);
				fParent->PostMessage(&removeSelfMessage);
			}

			// Send message to add view to notification center
			if (msg->what == kTimeoutExpired) {
				msg->what = kAddViewToCenter;
				be_app->PostMessage(msg);
			}

			break;
		}

		case kHeaderClosed:
		{
			int32 children = fInfo.size();
			for (int32 i = 0; i < children; i++) {
				fInfo[i]->RemoveSelf();
				delete fInfo[i];
			}
			fInfo.clear();

			// Remove ourselves from the parent view
			BMessage message(kRemoveGroupView);
			message.AddPointer("view", this);
			fParent->PostMessage(&message);
			break;
		}

		case kHeaderCollapsed:
		{
			int32 children = fInfo.size();
			if (fHeaderView->Collapsed()) {
				for (int32 i = 0; i < children; i++) {
					if (!fInfo[i]->IsHidden())
						fInfo[i]->Hide();
				}
		//		GetLayout()->SetExplicitMaxSize(BSize(B_SIZE_UNSET,
		//			GetLayout()->MinSize().Height()));
			} else {
				for (int32 i = 0; i < children; i++) {
					if (fInfo[i]->IsHidden())
						fInfo[i]->Show();
				}
		//		GetLayout()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));
			}

			InvalidateLayout();
			Invalidate(); // Need to redraw the collapse indicator and title
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void
AppGroupView::AddInfo(NotificationView* view)
{
	BString id = view->MessageID();
	bool found = false;

	if (id.Length() > 0) {
		int32 children = fInfo.size();

		for (int32 i = 0; i < children; i++) {
			if (id == fInfo[i]->MessageID()) {
				NotificationView* oldView = fInfo[i];
				fParent->NotificationViewSwapped(oldView, view);
				oldView->RemoveSelf();
				delete oldView;

				fInfo[i] = view;
				found = true;

				break;
			}
		}
	}

	// Invalidate all children to show or hide the close buttons in the
	// notification view
	int32 children = fInfo.size();
	for (int32 i = 0; i < children; i++) {
		fInfo[i]->Invalidate();
	}

	if (!found) {
		fInfo.push_back(view);
	}
	GetLayout()->AddView(view);

	if (IsHidden())
		Show();
	if (view->IsHidden(view) && !fHeaderView->Collapsed())
		view->Show();
}


const BString&
AppGroupView::Group() const
{
	return fLabel;
}


bool
AppGroupView::HasChildren()
{
	return !fInfo.empty();
}


int32
AppGroupView::ChildrenCount()
{
	return fInfo.size();
}
