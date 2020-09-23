#ifndef CORELDRAWIMPORTER_H
#define CORELDRAWIMPORTER_H

#include "common/common.h"
#include "Importer.h"

//#ifdef _DEBUG
//// If you are not building 32-bit Debug target, update this include path, so
//// the IntelliSense manages to load the generated header file for the type library.
//#include <LaserController.dir\Debug\VGCoreAuto.tlh>
//#else
//#import "libid:95E23C91-BC5A-49F3-8CD1-1FC515597048" version("12.0") \
//      rename("GetCommandLine", "VGGetCommandLine") \
//      rename("CopyFile", "VGCopyFile") \
//      rename("FindWindow", "VGFindWindow")
//#endif
#import "libid:95E23C91-BC5A-49F3-8CD1-1FC515597048" version("12.0") \
      rename("GetCommandLine", "VGGetCommandLine") \
      rename("CopyFile", "VGCopyFile") \
      rename("FindWindow", "VGFindWindow")

class CorelDrawImporter : public Importer
{
    Q_OBJECT
public:
    explicit CorelDrawImporter(QWidget* parentWnd, QObject* parent = nullptr);
    virtual ~CorelDrawImporter();

    virtual LaserDocument* import(const QString& filename, LaserScene* scene);
};

#endif // CORELDRAWIMPORTER_H