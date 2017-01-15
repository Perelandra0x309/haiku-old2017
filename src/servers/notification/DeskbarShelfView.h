/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@warpmail.net
 */
#ifndef DESKBAR_SHELF_VIEW_H
#define DESKBAR_SHELF_VIEW_H


#include <Deskbar.h>
#include <InterfaceKit.h>
#include <stdio.h>

static const char* kShelfviewName = "notifications_shelfview";


class _EXPORT DeskbarShelfView : public BView {
public:
							DeskbarShelfView();
							DeskbarShelfView(BMessage* data);
	virtual					~DeskbarShelfView();

	virtual void			AttachedToWindow();
	virtual void			DetachedFromWindow();
	static DeskbarShelfView*	Instantiate(BMessage* data);
	virtual	status_t		Archive(BMessage* data, bool deep = true) const;
	virtual void			Draw(BRect rect);
	virtual void			MessageReceived(BMessage* message);
	virtual void	 		MouseDown(BPoint);

private:
	void					_Quit();
	
	BBitmap*				fIcon;
};

#endif	/* DESKBAR_SHELF_VIEW_H */
