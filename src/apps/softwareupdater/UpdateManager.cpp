/*
 * Copyright 2013-2015, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler <axeld@pinc-software.de>
 *		Rene Gollent <rene@gollent.com>
 *		Ingo Weinhold <ingo_weinhold@gmx.de>
 *		Brian Hill <supernova@warpmail.net>
 */


#include "UpdateManager.h"

#include <sys/ioctl.h>
#include <unistd.h>

#include <Alert.h>

#include <package/CommitTransactionResult.h>
#include <package/DownloadFileRequest.h>
#include <package/RefreshRepositoryRequest.h>
#include <package/manager/Exceptions.h>
#include <package/solver/SolverPackage.h>
#include <package/solver/SolverProblem.h>
#include <package/solver/SolverProblemSolution.h>

#include "constants.h"

using namespace BPackageKit;
using namespace BPackageKit::BManager::BPrivate;


UpdateProgressListener::~UpdateProgressListener()
{
}


void
UpdateProgressListener::DownloadProgressChanged(const char* packageName,
	float progress)
{
}


void
UpdateProgressListener::DownloadProgressComplete(const char* packageName)
{
}


void
UpdateProgressListener::StartApplyingChanges(
	BPackageManager::InstalledRepository& repository)
{
}


void
UpdateProgressListener::ApplyingChangesDone(
	BPackageManager::InstalledRepository& repository)
{
}


void
UpdateProgressListener::SetUpdateStep(int32 step)
{
}


UpdateManager::UpdateManager(BPackageInstallationLocation location)
	:
	BPackageManager(location, &fClientInstallationInterface, this),
	BPackageManager::UserInteractionHandler(),
	fClientInstallationInterface()
{
}


UpdateManager::~UpdateManager()
{
}


void
UpdateManager::JobFailed(BSupportKit::BJob* job)
{
	BString error = job->ErrorString();
	if (error.Length() > 0) {
		error.ReplaceAll("\n", "\n*** ");
		fprintf(stderr, "%s", error.String());
	}
}


void
UpdateManager::JobAborted(BSupportKit::BJob* job)
{
	//DIE(job->Result(), "aborted");
}



void
UpdateManager::AddProgressListener(UpdateProgressListener* listener)
{
	fUpdateProgressListeners.AddItem(listener);
}


void
UpdateManager::RemoveProgressListener(UpdateProgressListener* listener)
{
	fUpdateProgressListeners.RemoveItem(listener);
}


void
UpdateManager::HandleProblems()
{
	int32 problemCount = fSolver->CountProblems();
	for (int32 i = 0; i < problemCount; i++) {
		// print problem and possible solutions
		BSolverProblem* problem = fSolver->ProblemAt(i);
		printf("problem %" B_PRId32 ": %s\n", i + 1,
			problem->ToString().String());

		int32 solutionCount = problem->CountSolutions();
		for (int32 k = 0; k < solutionCount; k++) {
			const BSolverProblemSolution* solution = problem->SolutionAt(k);
			printf("  solution %" B_PRId32 ":\n", k + 1);
			int32 elementCount = solution->CountElements();
			for (int32 l = 0; l < elementCount; l++) {
				const BSolverProblemSolutionElement* element
					= solution->ElementAt(l);
				printf("    - %s\n", element->ToString().String());
			}
		}

		// let the user choose a solution
		printf("Please select a solution, skip the problem for now or quit.\n");
		for (;;) {
			if (solutionCount > 1)
				printf("select [1...%" B_PRId32 "/s/q]: ", solutionCount);
			else
				printf("select [1/s/q]: ");

			char buffer[32];
			if (fgets(buffer, sizeof(buffer), stdin) == NULL
				|| strcmp(buffer, "q\n") == 0) {
				//exit(1);
			}

			if (strcmp(buffer, "s\n") == 0)
				break;

			/*
			char* end;
			long selected = strtol(buffer, &end, 0);
			if (end == buffer || *end != '\n' || selected < 1
				|| selected > solutionCount) {
				printf("*** invalid input\n");
				continue;
			}

			status_t error = fSolver->SelectProblemSolution(problem,
				problem->SolutionAt(selected - 1));
			if (error != B_OK)
				DIE(error, "failed to set solution");
			*/
			break;
		}
	}
}


void
UpdateManager::ConfirmChanges(bool fromMostSpecific)
{
	printf("The following changes will be made:\n");

	int32 count = fInstalledRepositories.CountItems();
	int32 upgradeCount = 0;
	int32 installCount = 0;
	int32 uninstallCount = 0;
	if (fromMostSpecific) {
		for (int32 i = count - 1; i >= 0; i--)
			_PrintResult(*fInstalledRepositories.ItemAt(i), upgradeCount,
				installCount, uninstallCount);
	} else {
		for (int32 i = 0; i < count; i++)
			_PrintResult(*fInstalledRepositories.ItemAt(i), upgradeCount,
				installCount, uninstallCount);
	}
	
	// TODO use the status window?
	printf("Upgrade count=%lu, Install count=%lu, Uninstall count=%lu\n",
		upgradeCount, installCount, uninstallCount);
	BString text("The following changes will be made:\nPackages upgraded: ");
	text << upgradeCount;
	text.Append("\nPackages installed: ");
	text << installCount;
	text.Append("\nPackages uninstalled: ");
	text << uninstallCount;
	text.Append("\n");
	BAlert* alert = new BAlert("Continue", text, "Cancel", "Update Now");
	int32 choice = alert->Go();
	printf("Choice: %lu\n", choice);
	
	if (choice == 0)
		throw BAbortedByUserException();
	
	for (int32 i = 0; i < fUpdateProgressListeners.CountItems(); i++) {
		fUpdateProgressListeners.ItemAt(i)->SetUpdateStep(ACTION_STEP_DOWNLOAD);
	}
}


void
UpdateManager::Warn(status_t error, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	if (error == B_OK)
		printf("\n");
	else
		printf(": %s\n", strerror(error));
}


void
UpdateManager::ProgressPackageDownloadStarted(const char* packageName)
{
	printf("Downloading %s...\n", packageName);
}


void
UpdateManager::ProgressPackageDownloadActive(const char* packageName,
	float completionPercentage, off_t bytes, off_t totalBytes)
{
	for (int32 i = 0; i < fUpdateProgressListeners.CountItems(); i++) {
		fUpdateProgressListeners.ItemAt(i)->DownloadProgressChanged(
			packageName, completionPercentage);
	}
/*	return;
	
	if (!fInteractive)
		return;
*/
	static const char* progressChars[] = {
		"\xE2\x96\x8F",
		"\xE2\x96\x8E",
		"\xE2\x96\x8D",
		"\xE2\x96\x8C",
		"\xE2\x96\x8B",
		"\xE2\x96\x8A",
		"\xE2\x96\x89",
		"\xE2\x96\x88",
	};

	int width = 70;

	struct winsize winSize;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winSize) == 0
		&& winSize.ws_col < 77) {
		// We need 7 characters for the percent display
		width = winSize.ws_col - 7;
	}

	int position;
	int ipart = (int)(completionPercentage * width);
	int fpart = (int)(((completionPercentage * width) - ipart) * 8);

	printf("\r"); // return to the beginning of the line

	for (position = 0; position < width; position++) {
		if (position < ipart) {
			// This part is fully downloaded, show a full block
			printf(progressChars[7]);
		} else if (position > ipart) {
			// This part is not downloaded, show a space
			printf(" ");
		} else {
			// This part is partially downloaded
			printf(progressChars[fpart]);
		}
	}

	// Also print the progress percentage
	printf(" %3d%%", (int)(completionPercentage * 100));

	fflush(stdout);
	
}


void
UpdateManager::ProgressPackageDownloadComplete(const char* packageName)
{
	for (int32 i = 0; i < fUpdateProgressListeners.CountItems(); i++) {
		fUpdateProgressListeners.ItemAt(i)->DownloadProgressComplete(
			packageName);
	}
	
	// Overwrite the progress bar with whitespace
	printf("\r");
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (int i = 0; i < (w.ws_col); i++)
		printf(" ");
	printf("\r\x1b[1A"); // Go to previous line.

	printf("Downloading %s...done.\n", packageName);
}


void
UpdateManager::ProgressPackageChecksumStarted(const char* title)
{
	printf("%s...", title);
}


void
UpdateManager::ProgressPackageChecksumComplete(const char* title)
{
	printf("done.\n");
}


void
UpdateManager::ProgressStartApplyingChanges(InstalledRepository& repository)
{
	printf("[%s] Applying changes ...\n", repository.Name().String());
	
	// TODO prevent completion for now during testing
	throw BAbortedByUserException();
}


void
UpdateManager::ProgressTransactionCommitted(InstalledRepository& repository,
	const BCommitTransactionResult& result)
{
	const char* repositoryName = repository.Name().String();

	int32 issueCount = result.CountIssues();
	for (int32 i = 0; i < issueCount; i++) {
		const BTransactionIssue* issue = result.IssueAt(i);
		if (issue->PackageName().IsEmpty()) {
			printf("[%s] warning: %s\n", repositoryName,
				issue->ToString().String());
		} else {
			printf("[%s] warning: package %s: %s\n", repositoryName,
				issue->PackageName().String(), issue->ToString().String());
		}
	}

	printf("[%s] Changes applied. Old activation state backed up in \"%s\"\n",
		repositoryName, result.OldStateDirectory().String());
	printf("[%s] Cleaning up ...\n", repositoryName);
}


void
UpdateManager::ProgressApplyingChangesDone(InstalledRepository& repository)
{
	printf("[%s] Done.\n", repository.Name().String());
}


void
UpdateManager::_PrintResult(InstalledRepository& installationRepository,
	int32& upgradeCount, int32& installCount, int32& uninstallCount)
{
	if (!installationRepository.HasChanges())
		return;

	printf("  in %s:\n", installationRepository.Name().String());

	PackageList& packagesToActivate
		= installationRepository.PackagesToActivate();
	PackageList& packagesToDeactivate
		= installationRepository.PackagesToDeactivate();

	BStringList upgradedPackages;
	BStringList upgradedPackageVersions;
	for (int32 i = 0;
		BSolverPackage* installPackage = packagesToActivate.ItemAt(i);
		i++) {
		for (int32 j = 0;
			BSolverPackage* uninstallPackage = packagesToDeactivate.ItemAt(j);
			j++) {
			if (installPackage->Info().Name() == uninstallPackage->Info().Name()) {
				upgradedPackages.Add(installPackage->Info().Name());
				upgradedPackageVersions.Add(uninstallPackage->Info().Version().ToString());
				break;
			}
		}
	}

	for (int32 i = 0; BSolverPackage* package = packagesToActivate.ItemAt(i);
		i++) {
		BString repository;
		if (dynamic_cast<MiscLocalRepository*>(package->Repository()) != NULL)
			repository = "local file";
		else
			repository.SetToFormat("repository %s", package->Repository()->Name().String());

		int position = upgradedPackages.IndexOf(package->Info().Name());
		if (position >= 0) {
			printf("    upgrade package %s-%s to %s from %s\n",
				package->Info().Name().String(),
				upgradedPackageVersions.StringAt(position).String(),
				package->Info().Version().ToString().String(),
				repository.String());
				upgradeCount++;
		} else {
			printf("    install package %s-%s from %s\n",
				package->Info().Name().String(),
				package->Info().Version().ToString().String(),
				repository.String());
				installCount++;
		}
	}

	for (int32 i = 0; BSolverPackage* package = packagesToDeactivate.ItemAt(i);
		i++) {
		if (upgradedPackages.HasString(package->Info().Name()))
			continue;
		printf("    uninstall package %s\n", package->VersionedName().String());
		uninstallCount++;
	}
}
