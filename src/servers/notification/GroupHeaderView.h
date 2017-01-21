/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2008-2009, Pier Luigi Fiorini. All Rights Reserved.
 * Copyright 2004-2008, Michael Davidson. All Rights Reserved.
 * Copyright 2004-2007, Mikael Eiman. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _GROUP_HEADER_VIEW_H
#define _GROUP_HEADER_VIEW_H


#include <Messenger.h>
#include <String.h>
#include <View.h>

class AppGroupView;

const uint32 kHeaderClosed = 'HeCl';
const uint32 kHeaderCollapsed = 'HeCo';


class GroupHeaderView : public BView {
public:
								GroupHeaderView(AppGroupView* parent,
									const char* label, uint32 type);

	virtual	void 				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint point);

	bool						Collapsed();

private:
			void				_CalculateSize();
			void				_DrawCloseButton(const BRect& updateRect);

			AppGroupView*		fParent;
			BMessenger			fParentMessenger;
			uint32				fAttachedWindowType;
			BString				fLabel;
			float				fHeight;
			bool				fCollapsed;
			BRect				fCloseRect;
			BRect				fCollapseRect;
			bool				fCloseClicked;
};

#endif	// _GROUP_HEADER_VIEW_H
