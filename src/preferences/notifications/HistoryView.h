/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _HISTORY_VIEW_H
#define _HISTORY_VIEW_H

#include <View.h>

#include <notification/AppUsage.h>

#include "NotificationView.h"
#include "SettingsPane.h"

typedef std::map<BString, AppUsage *> appusage_t;

class BCheckBox;
class BTextControl;
class BColumnListView;
class BStringColumn;
class BDateColumn;

class HistoryView : public SettingsPane {
public:
								HistoryView(SettingsHost* host);
								~HistoryView();
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			status_t			Load(BMessage&);
			status_t			Save(BMessage&);
			status_t			Revert() {return B_OK;} // FIXME implement this
			void				_PopulateGroups();
			void				_PopulateNotifications(const char* group);
			bool				_ArchiveIsValid(BMessage& archive, int32& count);
			void				_UpdatePreview(NotificationView* view);

			bool				fShowingPreview;
			appusage_t			fAppFilters;
			BCheckBox*			fBlockAll;
			BTextControl*		fSearch;
			BColumnListView*	fGroups;
			BStringColumn*		fGroupCol;
			BStringColumn*		fCountCol;
			BColumnListView*	fNotifications;
			BStringColumn*		fTitleCol;
			BDateColumn*		fDateCol;
			BStringColumn*		fContentCol;
			BStringColumn*		fTypeCol;
			BStringColumn*		fShownCol;
			NotificationView* 	fCurrentPreview;
};

#endif // _HISTORY_VIEW_H
