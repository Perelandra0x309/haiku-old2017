/*
 * Copyright 2017 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill
 */


#include "AddRepoWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <Clipboard.h>
#include <LayoutBuilder.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AddRepoWindow"

static float sAddWindowWidth = 500.0;


AddRepoWindow::AddRepoWindow(BRect size, BLooper* looper)
	:
	BWindow(BRect(0, 0, sAddWindowWidth, 10), "AddWindow", B_MODAL_WINDOW,
		B_ASYNCHRONOUS_CONTROLS	| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fReplyLooper(looper)
{
	fView = new BView("view", B_SUPPORTS_LAYOUT);
	fText = new BTextControl("text", B_TRANSLATE_COMMENT("Repository URL:",
		"Text box label"), "", new BMessage(ADD_BUTTON_PRESSED));
	fAddButton = new BButton(B_TRANSLATE_COMMENT("Add", "Button label"),
		new BMessage(ADD_BUTTON_PRESSED));
	fAddButton->MakeDefault(true);
	fCancelButton = new BButton(kCancelLabel,
		new BMessage(CANCEL_BUTTON_PRESSED));

	BLayoutBuilder::Group<>(fView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING,
			B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(fText)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(fCancelButton)
			.Add(fAddButton);
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fView);
	_GetClipboardData();
	fText->MakeFocus();

	// Move to the center of the preflet window
	CenterIn(size);
	float widthDifference = size.Width() - Frame().Width();
	if (widthDifference < 0)
		MoveBy(widthDifference / 2.0, 0);
	Show();
}


void
AddRepoWindow::Quit()
{
	fReplyLooper->PostMessage(ADD_WINDOW_CLOSED);
	BWindow::Quit();
}


void
AddRepoWindow::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
		case CANCEL_BUTTON_PRESSED: {
			if (QuitRequested())
				Quit();
			break;
		}
		case ADD_BUTTON_PRESSED: {
			BString url(fText->Text());
			if (url != "") {
				// URL must have a protocol
				if (url.FindFirst("://") == B_ERROR) {
					BAlert* alert = new BAlert("error",
						B_TRANSLATE_COMMENT("The URL must start with a "
							"protocol, for example http:// or https://",
							"Add URL error message"),
						kOKLabel, NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->SetFeel(B_MODAL_APP_WINDOW_FEEL);
					alert->Go(NULL);
					// Center the alert to this window and move down some
					alert->CenterIn(Frame());
					alert->MoveBy(0, kAddWindowOffset);
				} else {
					BMessage* addMessage = new BMessage(ADD_REPO_URL);
					addMessage->AddString(key_url, url);
					fReplyLooper->PostMessage(addMessage);
					Quit();
				}
			}
			break;
		}
		default:
			BWindow::MessageReceived(message);
	}
}


void
AddRepoWindow::FrameResized(float newWidth, float newHeight)
{
	sAddWindowWidth = newWidth;
}


status_t
AddRepoWindow::_GetClipboardData()
{
	if (be_clipboard->Lock()) {
		const char* string;
		ssize_t stringLen;
		BMessage* clip = be_clipboard->Data();
		clip->FindData("text/plain", B_MIME_TYPE, (const void **)&string,
			&stringLen);
		be_clipboard->Unlock();

		// The string must contain a web protocol
		BString clipString(string, stringLen);
		int32 ww = clipString.FindFirst("://");
		if (ww == B_ERROR)
			return B_ERROR;
		else
			fText->SetText(clipString);
	}
	return B_OK;
}
