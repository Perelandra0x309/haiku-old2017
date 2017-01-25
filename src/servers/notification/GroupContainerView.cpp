/*
 * Copyright 2013-2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */


#include "GroupContainerView.h"

#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ScrollBar.h>
#include <SpaceLayoutItem.h>
#include <StringView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GroupContainerView"


GroupContainerView::GroupContainerView()
	:
	BGroupView(B_VERTICAL, 0.0)
{
//	AddChild(BSpaceLayoutItem::CreateGlue());
// TODO add string when there are no notifications
/*	fNoNotificationsLabel = new BGroupView();
	BStringView* stringView = new BStringView("none",
		B_TRANSLATE("There are no new notifications"));
	stringView->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER,
		B_ALIGN_VERTICAL_UNSET));
	BLayoutBuilder::Group<>(fNoNotificationsLabel, B_VERTICAL, 0)
		.SetInsets(0, 10, 0, 10)
		.Add(stringView)
	.End();
	GroupLayout()->AddView(fNoNotificationsLabel);*/
}

/*
BSize
GroupContainerView::MinSize()
{
	BSize minSize = BGroupView::MinSize();
	return BSize(minSize.width, 60);
}*/

/*
BSize
GroupContainerView::MaxSize()
{
	BSize maxSize = BGroupView::MaxSize();
	return BSize(maxSize.width, 200);
}*/


void
GroupContainerView::AddGroup(AppGroupView* group)
{
//	if (fNoNotificationsLabel->Parent() != NULL)
//		fNoNotificationsLabel->RemoveSelf();
	GroupLayout()->AddView(group);
}


void
GroupContainerView::DoLayout()
{
	BGroupView::DoLayout();

	BScrollBar* scrollBar = ScrollBar(B_VERTICAL);

	if (scrollBar == NULL)
		return;

	BRect layoutArea = GroupLayout()->LayoutArea();
	float layoutHeight = layoutArea.Height();
	// Min size is not reliable with HasHeightForWidth() children,
	// since it does not reflect how those children are currently
	// laid out, but what their theoretical minimum size would be.

	BLayoutItem* lastItem = GroupLayout()->ItemAt(
		GroupLayout()->CountItems() - 1);
	if (lastItem != NULL)
		layoutHeight = lastItem->Frame().bottom;

	float viewHeight = Bounds().Height();

	float max = layoutHeight- viewHeight;
	scrollBar->SetRange(0, max);
	if (layoutHeight > 0)
		scrollBar->SetProportion(viewHeight / layoutHeight);
	else
		scrollBar->SetProportion(1);
}
