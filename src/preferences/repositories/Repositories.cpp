/*
 * Copyright 2017 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill
 */


#include "Repositories.h"

#include <Alert.h>
#include <Catalog.h>
#include <Cursor.h>
#include <LayoutBuilder.h>
#include <Roster.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RepositoriesApplication"

const char* kAppSignature = "application/x-vnd.Haiku-Repositories";


RepositoriesApplication::RepositoriesApplication()
	:
	BApplication(kAppSignature)
{
	fWindow = new RepositoriesWindow();
}


void
RepositoriesApplication::AboutRequested()
{
	BString text(B_TRANSLATE_COMMENT("Repositories, written by Brian Hill",
		"About box line 1"));
	text.Append("\n\n")
		.Append(B_TRANSLATE_COMMENT("Copyright ©2017, Haiku.",
			"About box line 2"))
		.Append("\n\n")
		.Append(B_TRANSLATE_COMMENT("This preflet will enable and disable "
			"repositories used with Haiku package management.", "About box line 3"));
	BAlert* aboutAlert = new BAlert("About", text, kOKLabel);
	aboutAlert->SetFlags(aboutAlert->Flags() | B_CLOSE_ON_ESCAPE);
	aboutAlert->Go();
}


int
main()
{
	RepositoriesApplication myApp;
	myApp.Run();
	return 0;
}
