/*
 * Copyright 2011, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PACKAGE__PACKAGE_VERSION_H_
#define _PACKAGE__PACKAGE_VERSION_H_


#include <String.h>


namespace BPackageKit {


class BPackageVersion {
public:
								BPackageVersion();
								BPackageVersion(const BString& major,
									const BString& minor, const BString& micro,
									uint8 release);

			status_t			InitCheck() const;

			void				GetAsString(BString& string) const;

			void				SetTo(const BString& major,
									const BString& minor, const BString& micro,
									uint8 release);
			void				Clear();

			int					Compare(const BPackageVersion& other) const;
									// does a natural compare over major, minor
									// and micro, finally comparing release

private:
			BString				fMajor;
			BString				fMinor;
			BString				fMicro;
			uint8				fRelease;
};


}	// namespace BPackageKit


#endif	// _PACKAGE__PACKAGE_VERSION_H_