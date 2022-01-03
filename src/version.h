#ifndef VERSION_H
#define VERSION_H

#define LC_VERSION_MAJOR 0
#define LC_VERSION_MINOR 5
#define LC_VERSION_BUILD 4
#define LC_VERSION_REVISION 2
#define LC_VERSION_TAG unstable
#define LC_VERSION LC_VERSION_MAJOR.LC_VERSION_MINOR.LC_VERSION_BUILD.LC_VERSION_REVISION
#define LC_VERSION_WITH_TAG LC_VERSION_MAJOR.LC_VERSION_MINOR.LC_VERSION_BUILD.LC_VERSION_REVISION-LC_VERSION_TAG
#define LC_STR_HELPER(n) #n
#define LC_STR(n) LC_STR_HELPER(n)
#define LC_VERSION_STR LC_STR(LC_VERSION_MAJOR) "." LC_STR(LC_VERSION_MINOR) "." LC_STR(LC_VERSION_BUILD) "." LC_STR(LC_VERSION_REVISION)
#define LC_VERSION_STR_WITH_TAG LC_STR(LC_VERSION_MAJOR) "." LC_STR(LC_VERSION_MINOR) "." LC_STR(LC_VERSION_BUILD) "." LC_STR(LC_VERSION_REVISION) "-" LC_STR(LC_VERSION_TAG)

#define VER_FILEVERSION             LC_VERSION_MAJOR,LC_VERSION_MINOR,LC_VERSION_BUILD,LC_VERSION_REVISION
#define VER_FILEVERSION_STR         LC_VERSION_STR

#define VER_PRODUCTVERSION          LC_VERSION_MAJOR,LC_VERSION_MINOR,LC_VERSION_BUILD,LC_VERSION_REVISION
#define VER_PRODUCTVERSION_STR      LC_VERSION_STR

#define VER_COMPANYNAME_STR         "CNE Laser"
#define VER_FILEDESCRIPTION_STR     "CNE Laser"
#define VER_INTERNALNAME_STR        "CNE Laser"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © 2021 CNE Laser"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "CNELaser.exe"
#define VER_PRODUCTNAME_STR         "CNE Laser"

#define VER_COMPANYDOMAIN_STR       "efslaser.com"

#endif // VERSION_H
