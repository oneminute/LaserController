#include "CorelDrawImporter.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QWindow>

#include <VGCoreAuto.tlh>
#include <ObjIdl.h>
#include <system_error>

#include "LaserApplication.h"
#include "common/common.h"
#include "primitive/LaserPrimitiveHeaders.h"
#include "scene/LaserDocument.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"
#include "ui/LaserControllerWindow.h"
#include "util/Utils.h"
#include "util/TypeUtils.h"


//#import "libid:95E23C91-BC5A-49F3-8CD1-1FC515597048" version("12.0") \
      //rename("GetCommandLine", "VGGetCommandLine") \
      //rename("CopyFile", "VGCopyFile") \
      //rename("FindWindow", "VGFindWindow")

CorelDrawImporter::CorelDrawImporter(QObject* parent)
    : Importer(parent)
{

}

CorelDrawImporter::~CorelDrawImporter()
{
}

void CorelDrawImporter::importImpl(const QString & filename, LaserScene* scene, QList<LaserPrimitive*>& unavailables, ProgressItem* parentProgress, const QVariantMap& params)
{
    ProgressItem* progress = new ProgressItem("Import from Corel Draw", ProgressItem::PT_Simple, parentProgress);
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        QMessageBox::warning(LaserApplication::mainWindow, tr("Import failure"), tr("Cannot initialize COM."));
		CoUninitialize();
        progress->finish();
        return;
    }
    progress->increaseProgress(10);

	CLSID clsid;
	hr = CLSIDFromProgID(L"CorelDRAW.Application.18", &clsid);

	QDir tmpDir(QCoreApplication::applicationDirPath() + "\\tmp");
	QString tmpSvgFilename;
    bool success = true;
	HWND hWnd = 0;
    try
    {
        progress->increaseProgress(20);
		VGCore::IVGApplicationPtr app(L"CorelDRAW.Application.18");
		if (!app)
		{
			QMessageBox::warning(LaserApplication::mainWindow, tr("Import failure"), tr("Cannot load cdr x8's dll."));
			CoUninitialize();
            progress->finish();
            return;
		}

		IUnknown* pUnk = NULL;
		app.QueryInterface(IID_IUnknown, (void**)&pUnk);

        app->Visible = VARIANT_TRUE;
		qDebug() << "app visible:" << app->Visible;
		VGCore::IVGWindowsPtr wnds = app->Windows;
		qDebug() << "cdr windows count:" << wnds->Count;
		for (int i = 1; i <= wnds->Count; i++)
		{
			VGCore::IVGWindowPtr wnd = wnds->Item[i];
			qDebug() << "cdr window's handle" << i << hex << showbase << wnd->Handle;
		}

        VGCore::IVGWindowPtr window = app->ActiveWindow;
        if (!window)
        {
            QMessageBox::warning(LaserApplication::mainWindow, tr("Import CDR"), tr("No active document in CorelDRAW!"));
			CoUninitialize();
            progress->finish();
            return;
        }
		qDebug() << "CDR Window active object:" << window;
		qDebug() << "CDR window handle:" << hex << showbase << window->GetHandle();

		VGCore::cdrWindowState wndState = window->WindowState;
		qDebug() << "cdr window is active?" << window->Active;
		qDebug() << "cdr window state:" << wndState;
		window->Activate();

		hWnd = (HWND) window->Handle;
		while (hWnd)
		{
			HWND parent = GetParent(hWnd);
			if (!parent)
				break;
			hWnd = parent;
		}
		qDebug() << "cdr top window's handle:" << hex << showbase << hWnd;
		ShowWindow(hWnd, SW_SHOWMINIMIZED);

        VGCore::IVGDocumentPtr doc = app->ActiveDocument;
		qDebug() << "cdr document ptr:" << doc;
        if (!doc)
        {
            QMessageBox::warning(LaserApplication::mainWindow, tr("Import CDR"), tr("No active document in CorelDRAW!"));
			CoUninitialize();
            progress->finish();
            return;
        }

        qDebug() << "selection count:" << doc->Selection()->Shapes->Count;
        VGCore::cdrExportRange range = VGCore::cdrExportRange::cdrCurrentPage;
        if (doc->Selection() && doc->Selection()->Shapes->Count > 0)
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
        opt->ResolutionX = 600;
        opt->ResolutionY = 600;

        VGCore::IVGStructPaletteOptionsPtr pal = app->CreateStructPaletteOptions();
        pal->PaletteType = VGCore::cdrPaletteOptimized;
        pal->NumColors = 16;
        pal->DitherType = VGCore::cdrDitherNone;

        VGCore::ICorelExportFilterPtr filter = doc->ExportEx(typeUtils::qStringToBstr(tmpSvgFilename), VGCore::cdrSVG, range, opt, pal);
        progress->increaseProgress(95);
        if (filter->HasDialog)
        {
            if (filter->ShowDialog(LaserApplication::mainWindow->winId()))
            {
                hr = filter->Finish();
                if (FAILED(hr))
                {
                    qDebug() << hr;
                    QString errorMessage = QString::fromLocal8Bit(std::system_category().message(hr).c_str());
                    qWarning() << errorMessage;
                }
            }
			else
			{
				success = false;
			}
        }
        progress->finish();
    }
    catch (_com_error& ex)
    {
        QString errorMessage = QString::fromLocal8Bit(std::system_category().message(ex.Error()).c_str());
        qDebug() << errorMessage;
		QMessageBox::warning(LaserApplication::mainWindow, tr("Import Failure"), tr("Can not import from cdr! Please confirm that cdr has been installed."));
        success = false;
        progress->finish();
    }
	//ShowWindow(hWnd, SW_RESTORE);

    if (success)
    {
        QSharedPointer<Importer> importer = Importer::getImporter(LaserApplication::mainWindow, Importer::SVG);
        importer->importImpl(tmpSvgFilename, scene, unavailables, parentProgress, params);

        if (tmpDir.exists(tmpSvgFilename))
        {
            //tmpDir.remove(tmpSvgFilename);
        }
        emit imported();
    }
    CoUninitialize();
}

