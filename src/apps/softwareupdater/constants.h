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
	ACTION_STEP_MAX
};

// Message what values
static const uint32 kMsgUpdate = 'iUPD';
static const uint32 kMsgExit = 'iEXT';

// Message data keys
#define kKeyHeader "key_header"
#define kKeyDetail "key_detail"


#endif // CONSTANTS_H
