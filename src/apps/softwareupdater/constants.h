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
	STATE_GET_CONFIRMATION,
	STATE_FINAL_MSG,
	STATE_MAX
};

// Message what values
static const uint32 kMsgUpdate = 'iUPD';
static const uint32 kMsgCancel = 'iCAN';
static const uint32 kMsgConfirm = 'iCON';

// Message data keys
#define kKeyHeader "key_header"
#define kKeyDetail "key_detail"


#endif // CONSTANTS_H
