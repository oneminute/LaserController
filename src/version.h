#ifndef VERSION_H
#define VERSION_H

#ifdef ARCH_x64
#define LC_ARCH x64
#else
#define LC_ARCH x86
#endif
#define LC_VERSION_MAJOR 0
#define LC_VERSION_MINOR 5
#define LC_VERSION_BUILD 10
#define LC_VERSION_REVISION 7
#define LC_VERSION_TAG released
#define LC_VERSION LC_VERSION_MAJOR.LC_VERSION_MINOR.LC_VERSION_BUILD.LC_VERSION_REVISION
#define LC_VERSION_WITH_TAG LC_VERSION_MAJOR.LC_VERSION_MINOR.LC_VERSION_BUILD.LC_VERSION_REVISION-LC_VERSION_TAG
#define LC_STR_HELPER(n) #n
#define LC_STR(n) LC_STR_HELPER(n)
#define LC_VERSION_STR LC_STR(LC_VERSION_MAJOR) "." LC_STR(LC_VERSION_MINOR) "." LC_STR(LC_VERSION_BUILD) "." LC_STR(LC_VERSION_REVISION)
#define LC_VERSION_STR_WITH_TAG LC_STR(LC_VERSION_MAJOR) "." LC_STR(LC_VERSION_MINOR) "." LC_STR(LC_VERSION_BUILD) "." LC_STR(LC_VERSION_REVISION) "-" LC_STR(LC_ARCH) "-" LC_STR(LC_VERSION_TAG)

#define VER_FILEVERSION             LC_VERSION_MAJOR,LC_VERSION_MINOR,LC_VERSION_BUILD,LC_VERSION_REVISION
#define VER_FILEVERSION_STR         LC_VERSION_STR

#define VER_PRODUCTVERSION          LC_VERSION_MAJOR,LC_VERSION_MINOR,LC_VERSION_BUILD,LC_VERSION_REVISION
#define VER_PRODUCTVERSION_STR      LC_VERSION_STR

#define VER_COMPANYNAME_STR         "Laser CEO"
#define VER_FILEDESCRIPTION_STR     "Laser CEO"
#define VER_INTERNALNAME_STR        "Laser CEO"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © 2021 Laser CEO"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "LaserCEO.exe"
#define VER_PRODUCTNAME_STR         "Laser CEO"

#define VER_COMPANYDOMAIN_STR       "efslaser.com"

#endif // VERSION_H
