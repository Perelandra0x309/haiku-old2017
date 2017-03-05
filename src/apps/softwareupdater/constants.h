/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill <supernova@warpmail.net>
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

enum {
	ACTION_STEP_INIT = 0,
	ACTION_STEP_START,
	ACTION_STEP_DOWNLOAD,
	ACTION_STEP_APPLY,
	ACTION_STEP_COMPLETE,
	ACTION_STEP_MAX
};

enum {
	STATE_HEAD = 0,
	STATE_DISPLAY_STATUS,
	STATE_DISPLAY_PROGRESS,
	STATE_GET_CONFIRMATION,
	STATE_APPLY_UPDATES,
	STATE_FINAL_MSG,
	STATE_MAX
};

// Message what values
static const uint32 kMsgTextUpdate = 'iUPD';
static const uint32 kMsgProgressUpdate = 'iPRO';
static const uint32 kMsgCancel = 'iCAN';
static const uint32 kMsgConfirm = 'iCON';
static const uint32 kMsgClose = 'iCLO';
static const uint32 kMsgShow = 'iSHO';
//static const uint32 kMsgViewDetails = 'iVIE';
static const uint32 kMsgShowInfo = 'iSHI';

// Message data keys
#define kKeyHeader "key_header"
#define kKeyDetail "key_detail"
#define kKeyPackageName "key_packagename"
#define kKeyPackageCount "key_packagecount"
#define kKeyPercentage "key_percentage"
#define kKeyFrame "key_frame"


#endif // CONSTANTS_H
