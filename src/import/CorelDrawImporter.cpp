#include "common/common.h"
#include "CorelDrawImporter.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"
#include "scene/LaserDocument.h"

#include <system_error>
#include <QDir>
#include <QCoreApplication>
#include <QDialog>

CorelDrawImporter::CorelDrawImporter(QObject* parent)
    : Importer(parent)
{

}

CorelDrawImporter::~CorelDrawImporter()
{
}

LaserDocument * CorelDrawImporter::import(const QString & filename)
{
    VGCore::IVGApplicationPtr app;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        qWarning() << "Can not initialize CorelDraw.";
        return nullptr;
    }

    app = VGCore::IVGApplicationPtr(L"CorelDRAW.Application.18");

    QDir tmpDir(QCoreApplication::applicationDirPath() + "/tmp");
    
    QString tmpSvgFilename = "";
    try
    {
        app->Visible = VARIANT_TRUE;
        VGCore::IVGDocumentPtr doc = app->ActiveDocument;
        if (!doc)
        {
            return nullptr;
        }

        qDebug() << "selection count:" << doc->Selection()->Shapes->Count;
        VGCore::cdrExportRange range = VGCore::cdrExportRange::cdrCurrentPage;
        if (doc->Selection()->Shapes->Count > 0)
        {
            range = VGCore::cdrExportRange::cdrSelection;
        }

        tmpSvgFilename = utils::createUUID() + ".svg";
        tmpSvgFilename = tmpDir.absoluteFilePath(tmpSvgFilename);
        //QString tmpSvgFilename("d:/tmp.svg");

        qDebug().noquote() << "export corel draw active document to" << tmpSvgFilename;
        
        VGCore::IVGStructExportOptionsPtr opt = app->CreateStructExportOptions();
        VGCore::IVGStructPaletteOptionsPtr pal = app->CreateStructPaletteOptions();
        opt->AntiAliasingType = VGCore::cdrNormalAntiAliasing;
        opt->ImageType = VGCore::cdrRGBColorImage;
        opt->ResolutionX = 72;
        opt->ResolutionY = 72;
        pal->PaletteType = VGCore::cdrPaletteOptimized;
        pal->NumColors = 16;
        pal->DitherType = VGCore::cdrDitherNone;
        VGCore::ICorelExportFilterPtr filter = doc->ExportEx(typeUtils::qStringToBstr(tmpSvgFilename), VGCore::cdrSVG, range, opt, pal);
        if (filter->HasDialog)
        {
            if (filter->ShowDialog(0))
            {
                hr = filter->Finish();
                if (FAILED(hr))
                {
                    QString errorMessage = QString::fromLocal8Bit(std::system_category().message(hr).c_str());
                    qWarning() << errorMessage;
                }
            }
        }
    }
    catch (_com_error& ex)
    {
        //QString errorMsg = (LPTSTR)ex.ErrorMessage();
        //wprintf(L"Error occurred: 0x%08X (%s)\n", ex.Error(), ex.ErrorMessage());
        QString errorMessage = QString::fromLocal8Bit(std::system_category().message(ex.Error()).c_str());
        qDebug() << errorMessage;
    }

    QSharedPointer<Importer> importer = Importer::getImporter(Importer::SVG);
    LaserDocument* doc = importer->import(tmpSvgFilename);

    if (tmpDir.exists(tmpSvgFilename))
    {
        tmpDir.remove(tmpSvgFilename);
    }

    doc->open();
    return doc;
}
