#include "common/common.h"
#include "CorelDrawImporter.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"
#include "scene/LaserDocument.h"

#include <system_error>
#include <QDir>
#include <QCoreApplication>
#include <QDialog>
#include <QMessageBox>

#ifdef _DEBUG
// If you are not building 32-bit Debug target, update this include path, so
// the IntelliSense manages to load the generated header file for the type library.
#include <LaserController.dir\Debug\VGCoreAuto.tlh>
#else
#import "libid:95E23C91-BC5A-49F3-8CD1-1FC515597048" version("12.0") \
      rename("GetCommandLine", "VGGetCommandLine") \
      rename("CopyFile", "VGCopyFile") \
      rename("FindWindow", "VGFindWindow")
#endif

CorelDrawImporter::CorelDrawImporter(QWidget* parentWnd, QObject* parent)
    : Importer(parentWnd, parent)
{

}

CorelDrawImporter::~CorelDrawImporter()
{
}

LaserDocument * CorelDrawImporter::import(const QString & filename, LaserScene* scene, const QVariantMap& params)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    QDir tmpDir(QCoreApplication::applicationDirPath() + "/tmp");
    VGCore::IVGApplicationPtr app(L"CorelDRAW.Application.18");
    
    QString tmpSvgFilename;
    bool success = true;
    try
    {
        app->Visible = VARIANT_TRUE;
        VGCore::IVGWindowPtr window = app->ActiveWindow;
        qDebug() << "window:" << window;
        window->Activate();
        VGCore::IVGDocumentPtr doc = app->ActiveDocument;
        if (!doc)
        {
            QMessageBox::warning(m_parentWnd, tr("No active document"), tr("No active document in CorelDRAW!"));
            return nullptr;
        }

        qDebug() << "selection count:" << doc->Selection()->Shapes->Count;
        VGCore::cdrExportRange range = VGCore::cdrExportRange::cdrCurrentPage;
        if (doc->Selection()->Shapes->Count > 0)
        {
            range = VGCore::cdrExportRange::cdrSelection;
        }

        tmpSvgFilename = utils::createUUID() + ".svg";
        tmpSvgFilename = QDir::toNativeSeparators(tmpDir.absoluteFilePath(tmpSvgFilename));

        qDebug().noquote() << "export corel draw active document to" << tmpSvgFilename;
        
        VGCore::IVGStructExportOptionsPtr opt = app->CreateStructExportOptions();
        opt->AlwaysOverprintBlack = true;
        opt->AntiAliasingType = VGCore::cdrNormalAntiAliasing;
        opt->Compression = VGCore::cdrCompressionNone;
        opt->Dithered = false;
        opt->ImageType = VGCore::cdrRGBColorImage;
        opt->ResolutionX = 72;
        opt->ResolutionY = 72;

        VGCore::IVGStructPaletteOptionsPtr pal = app->CreateStructPaletteOptions();
        pal->PaletteType = VGCore::cdrPaletteOptimized;
        pal->NumColors = 16;
        pal->DitherType = VGCore::cdrDitherNone;

        VGCore::ICorelExportFilterPtr filter = doc->ExportEx(typeUtils::qStringToBstr(tmpSvgFilename), VGCore::cdrSVG, range, opt, pal);
        if (filter->HasDialog)
        {
            int parentWinId = 0;
            if (params.contains("parent_winid"))
                parentWinId = params["parent_winid"].toInt();
            if (filter->ShowDialog(parentWinId))
            {
                hr = filter->Finish();
                if (FAILED(hr))
                {
                    qDebug() << hr;
                    QString errorMessage = QString::fromLocal8Bit(std::system_category().message(hr).c_str());
                    qWarning() << errorMessage;
                }
            }
        }
    }
    catch (_com_error& ex)
    {
        QString errorMessage = QString::fromLocal8Bit(std::system_category().message(ex.Error()).c_str());
        qDebug() << errorMessage;
        success = false;
    }

    LaserDocument* doc = nullptr;
    if (success)
    {
        QSharedPointer<Importer> importer = Importer::getImporter(m_parentWnd, Importer::SVG);
        doc = importer->import(tmpSvgFilename, scene, params);

        if (doc)
        {
            if (tmpDir.exists(tmpSvgFilename))
            {
                tmpDir.remove(tmpSvgFilename);
            }

            doc->open();
        }
    }
    app->Release();
    CoUninitialize();
    return doc;
}

