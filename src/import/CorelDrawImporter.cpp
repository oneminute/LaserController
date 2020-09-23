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

CorelDrawImporter::CorelDrawImporter(QWidget* parentWnd, QObject* parent)
    : Importer(parentWnd, parent)
{

}

CorelDrawImporter::~CorelDrawImporter()
{
}

LaserDocument * CorelDrawImporter::import(const QString & filename, LaserScene* scene)
{
    VGCore::IVGApplicationPtr app;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        qWarning() << "Can not initialize CorelDraw.";
        return nullptr;
    }

    app = VGCore::IVGApplicationPtr(L"CorelDRAW.Application.18");
    qDebug() << "visible: " << app->Visible;
    /*if (!app->Visible)
    {
        QMessageBox::warning(m_parentWnd, tr("CorelDRAW not open"), tr("Please open your corelDRAW application!"));
        return nullptr;
    }*/

    QDir tmpDir(QCoreApplication::applicationDirPath() + "/tmp");
    
    QString tmpSvgFilename = "";
    try
    {
        app->Visible = VARIANT_TRUE;
        //VGCore::IVGWindowPtr window = app->AppWindow;
        //qDebug() << "window:" << window;
        //window->Activate();
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
        tmpSvgFilename = tmpDir.absoluteFilePath(tmpSvgFilename);

        qDebug().noquote() << "export corel draw active document to" << tmpSvgFilename;
        
        VGCore::IVGStructExportOptionsPtr opt = app->CreateStructExportOptions();
        //opt->AlwaysOverprintBlack = true;
        //opt->AntiAliasingType = VGCore::cdrNormalAntiAliasing;
        //opt->Compression = VGCore::cdrCompressionNone;
        //opt->Dithered = false;
        opt->ImageType = VGCore::cdrGrayscaleImage;
        //opt->ResolutionX = 72;
        //opt->ResolutionY = 72;

        VGCore::IVGStructPaletteOptionsPtr pal = app->CreateStructPaletteOptions();
        //pal->PaletteType = VGCore::cdrPaletteOptimized;
        //pal->NumColors = 16;
        //pal->DitherType = VGCore::cdrDitherNone;
        VGCore::ICorelExportFilterPtr filter = doc->ExportEx(typeUtils::qStringToBstr(tmpSvgFilename), VGCore::cdrSVG, range, opt, pal);
        if (filter->HasDialog)
        {
            if (filter->ShowDialog(0))
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
        //QString errorMsg = (LPTSTR)ex.ErrorMessage();
        //wprintf(L"Error occurred: 0x%08X (%s)\n", ex.Error(), ex.ErrorMessage());
        QString errorMessage = QString::fromLocal8Bit(std::system_category().message(ex.Error()).c_str());
        qDebug() << errorMessage;
        return nullptr;
    }

    QSharedPointer<Importer> importer = Importer::getImporter(m_parentWnd, Importer::SVG);
    LaserDocument* doc = importer->import(tmpSvgFilename, scene);

    if (doc)
    {
        if (tmpDir.exists(tmpSvgFilename))
        {
            tmpDir.remove(tmpSvgFilename);
        }

        doc->open();
    }
    return doc;
}
