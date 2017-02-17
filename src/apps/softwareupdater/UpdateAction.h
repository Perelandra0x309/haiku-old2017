/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill <supernova@warpmail.net>
 */
#ifndef UPDATE_ACTION_H
#define UPDATE_ACTION_H


#include <SupportDefs.h>

#include "UpdateManager.h"


class UpdateAction /*: private UpdateProgressListener*/ {
public:
								UpdateAction();
								~UpdateAction();
		status_t				Perform();

/*	virtual	void				DownloadProgressChanged(
									const char* packageName,
									float progress);
	virtual	void				DownloadProgressComplete(
									const char* packageName);
*/
private:
		UpdateManager*			fUpdateManager;
};


#endif // UPDATE_ACTION_H
