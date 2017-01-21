/*
 * Copyright 2010-2011, Haiku, Inc. All Rights Reserved.
 * Copyright 2008-2009, Pier Luigi Fiorini. All Rights Reserved.
 * Copyright 2004-2008, Michael Davidson. All Rights Reserved.
 * Copyright 2004-2007, Mikael Eiman. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Mikael Eiman, mikael@eiman.tv
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Stephan Aßmus <superstippi@gmx.de>
 *		Adrien Destugues <pulkomandy@pulkomandy.ath.cx>
 *		Brian Hill <supernova@warpmail.net>
 */


#include "GroupHeaderView.h"

#include <ControlLook.h>

#include "AppGroupView.h"
#include "NotificationWindow.h"

static const int kHeaderSize = 23;


GroupHeaderView::GroupHeaderView(AppGroupView* parent, const char* label, uint32 type)
	:
	BView("HeaderView", B_WILL_DRAW),
	fParent(parent),
	fAttachedWindowType(type),
	fLabel(label),
	fHeight(kHeaderSize),
	fCollapsed(false),
	fCloseClicked(false)
{
	_CalculateSize();
}


void
GroupHeaderView::AttachedToWindow()
{
	fParentMessenger.SetTo(fParent);
}


void
GroupHeaderView::Draw(BRect updateRect)
{
	rgb_color menuColor = ViewColor();
	BRect bounds = Bounds();
	rgb_color hilite = tint_color(menuColor, B_DARKEN_1_TINT);
	rgb_color vlight = tint_color(menuColor, B_LIGHTEN_2_TINT);
	bounds.bottom = bounds.top + kHeaderSize;

	// Draw the header background
	SetHighColor(tint_color(menuColor, 1.22));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	if (fAttachedWindowType == SHELVED_NOTIFICATIONS_WINDOW) {
		SetLowColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
		SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
		Invalidate();
	}
	StrokeLine(bounds.LeftTop(), bounds.LeftBottom());
	uint32 borders = BControlLook::B_TOP_BORDER
		| BControlLook::B_BOTTOM_BORDER | BControlLook::B_RIGHT_BORDER;

	be_control_look->DrawButtonBackground(this, bounds, bounds, menuColor,
		0, borders);

	// Draw the buttons
	fCollapseRect.top = (kHeaderSize - kExpandSize) / 2;
	fCollapseRect.left = kEdgePadding * 3;
	fCollapseRect.right = fCollapseRect.left + 1.5 * kExpandSize;
	fCollapseRect.bottom = fCollapseRect.top + kExpandSize;

	fCloseRect = bounds;
	fCloseRect.top = (kHeaderSize - kCloseSize) / 2;
	// Take off the 1 to line this up with the close button on the
	// notification view
	fCloseRect.right -= kEdgePadding * 3 - 1;
	fCloseRect.left = fCloseRect.right - kCloseSize;
	fCloseRect.bottom = fCloseRect.top + kCloseSize;

	uint32 arrowDirection = fCollapsed
		? BControlLook::B_DOWN_ARROW : BControlLook::B_UP_ARROW;
	be_control_look->DrawArrowShape(this, fCollapseRect, fCollapseRect,
		LowColor(), arrowDirection, 0, B_DARKEN_3_TINT);

	SetPenSize(kPenSize);

	// Draw the dismiss widget
	_DrawCloseButton(updateRect);

	// Draw the label
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
	BString label = fLabel;
	if (fCollapsed)
		label << " (" << fParent->ChildrenCount() << ")";

	SetFont(be_bold_font);
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	float y = (bounds.top + bounds.bottom - ceilf(fontHeight.ascent)
		- ceilf(fontHeight.descent)) / 2.0 + ceilf(fontHeight.ascent);

	DrawString(label.String(),
		BPoint(fCollapseRect.right + 4 * kEdgePadding, y));
	Sync();
}


void
GroupHeaderView::_DrawCloseButton(const BRect& updateRect)
{
	PushState();
	BRect closeRect = fCloseRect;

	rgb_color base = ui_color(B_PANEL_BACKGROUND_COLOR);
	float tint = B_DARKEN_2_TINT;

	if (fCloseClicked) {
		BRect buttonRect(closeRect.InsetByCopy(-4, -4));
		be_control_look->DrawButtonFrame(this, buttonRect, updateRect,
			base, base,
			BControlLook::B_ACTIVATED | BControlLook::B_BLEND_FRAME);
		be_control_look->DrawButtonBackground(this, buttonRect, updateRect,
			base, BControlLook::B_ACTIVATED);
		tint *= 1.2;
		closeRect.OffsetBy(1, 1);
	}

	base = tint_color(base, tint);
	SetHighColor(base);
	SetPenSize(2);
	StrokeLine(closeRect.LeftTop(), closeRect.RightBottom());
	StrokeLine(closeRect.LeftBottom(), closeRect.RightTop());
	PopState();
}


void
GroupHeaderView::MouseDown(BPoint point)
{
	if (BRect(fCloseRect).InsetBySelf(-5, -5).Contains(point))
		fParentMessenger.SendMessage(kHeaderClosed);
	else if (BRect(fCollapseRect).InsetBySelf(-5, -5).Contains(point)) {
		fCollapsed = !fCollapsed;
		fParentMessenger.SendMessage(kHeaderCollapsed);
	}
}


bool
GroupHeaderView::Collapsed()
{
	return fCollapsed;
}


void
GroupHeaderView::_CalculateSize()
{
	//TODO modify based on font size?
	SetExplicitMinSize(BSize(0, fHeight));
	SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, fHeight));
}
