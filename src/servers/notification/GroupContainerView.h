/*
 * Copyright 2013-2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef GROUP_CONTAINER_VIEW_H
#define GROUP_CONTAINER_VIEW_H


#include <GroupView.h>

#include "AppGroupView.h"


//! A group view to hold items in a vertically scrollable area. Needs to
// be embedded into a BScrollView with vertical scrollbar to work. Get the
// BGroupLayout with GroupLayout() and add or remove items/views to the layout.
class GroupContainerView : public BGroupView {
public:
								GroupContainerView();

//	virtual	BSize				MinSize();
//	virtual	BSize				MaxSize();
	void						AddGroup(AppGroupView* group);

protected:
	virtual	void				DoLayout();

	BGroupView*					fNoNotificationsLabel;
};


#endif // GROUP_CONTAINER_VIEW_H
