/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill <supernova@warpmail.net>
 */


#include "UpdateAction.h"

//#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <package/manager/Exceptions.h>

#include "constants.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "UpdateAction"


using namespace BPackageKit;
//using namespace BPackageKit::BPrivate;
using namespace BPackageKit::BManager::BPrivate;


UpdateAction::UpdateAction()
{
	fUpdateManager = new(std::nothrow)
		UpdateManager(B_PACKAGE_INSTALLATION_LOCATION_SYSTEM);
}


UpdateAction::~UpdateAction()
{
	delete fUpdateManager;
}


status_t
UpdateAction::Perform()
{
	fUpdateManager->Init(BPackageManager::B_ADD_INSTALLED_REPOSITORIES
		| BPackageManager::B_ADD_REMOTE_REPOSITORIES
		| BPackageManager::B_REFRESH_REPOSITORIES);
//	fUpdateManager->AddProgressListener(this);
	
	try {
		// These values indicate that all updates should be installed
		int packageCount = 0;
		const char* const packages = "";

		// perform the update
//		fUpdateManager->SetDebugLevel(1);
		fUpdateManager->Update(&packages, packageCount);
	} catch (BFatalErrorException ex) {
		BString errorString;
		errorString.SetToFormat(
			"Fatal error occurred while updating packages: "
			"%s (%s)\n", ex.Message().String(),
			ex.Details().String());
	/*	BAlert* alert(new(std::nothrow) BAlert(B_TRANSLATE("Fatal error"),
			errorString, B_TRANSLATE("Close"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_STOP_ALERT));
		if (alert != NULL)
			alert->Go();*/
		BString text(B_TRANSLATE("Error message:"));
		text.Append(" ").Append(ex.Message());
		fUpdateManager->FinalError(B_TRANSLATE("Error detected"),
			text.String());
		return ex.Error();
	} catch (BAbortedByUserException ex) {
		fprintf(stderr, "Updates aborted by user: %s\n",
			ex.Message().String());
		fUpdateManager->FinalError(B_TRANSLATE("Updates cancelled"),
			B_TRANSLATE("Updates were cancelled by the user."));
		return B_OK;
	} catch (BNothingToDoException ex) {
		fprintf(stderr, "Nothing to do while updating packages : %s\n",
			ex.Message().String());
		fUpdateManager->FinalError(B_TRANSLATE("No updates available"),
			B_TRANSLATE("There were no updates found."));
		return B_OK;
	} catch (BException ex) {
		fprintf(stderr, "Exception occurred while updating packages : %s\n",
			ex.Message().String());
		BString text(B_TRANSLATE("Error message:"));
		text.Append(" ").Append(ex.Message());
		fUpdateManager->FinalError(B_TRANSLATE("General error detected"),
			text.String());
		return B_ERROR;
	}

//	fUpdateManager->RemoveProgressListener(this);
	
	return B_OK;
}

/*
void
UpdateAction::DownloadProgressChanged(const char* packageName, float progress)
{
//	if (fCurrentStep != ACTION_STEP_DOWNLOAD)
//		return;
	
	int32 percent = int(100 * progress);
	BString header("Downloading packages");
	BString detail(packageName);
	detail.Append(" ");
	detail << percent;
	detail.Append("% complete.");
	_SetStatus(header.String(), detail.String());
}


void
UpdateAction::DownloadProgressComplete(const char* packageName)
{
	
}*/
