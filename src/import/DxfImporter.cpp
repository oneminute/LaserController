#include "DxfImporter.h"

#include "common/common.h"
#include "DxfNode.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "scene/LaserDocument.h"
#include "task/ProgressItem.h"
#include "task/ProgressModel.h"
#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch> 
#include <QStack>
#include <QTextStream>

class DxfImporterPrivate
{
    Q_DECLARE_PUBLIC(DxfImporter)
public:
    DxfImporterPrivate(DxfImporter* ptr)
        : q_ptr(ptr)
    {

    }

    ~DxfImporterPrivate()
    {
    }

    DxfImporter* q_ptr;
    DxfDocumentNode* documentNode;
};

DxfImporter::DxfImporter(QObject* parent)
    : Importer(parent)
    , m_ptr(new DxfImporterPrivate(this))
{

}

DxfImporter::~DxfImporter()
{
}

LaserDocument* DxfImporter::import(const QString& filename, LaserScene* scene, ProgressItem* parentProgress, const QVariantMap& params)
{
    Q_D(DxfImporter);
    qLogD << "import from dxf";

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return nullptr;

    DxfStream stream(&file, parentProgress);

    d->documentNode = new DxfDocumentNode;
    if (!d->documentNode->parse(&stream))
    {
        delete d->documentNode;
        d->documentNode = nullptr;
        return nullptr;
    }

#ifdef _DEBUG
    //d->documentNode->debugPrint();
#endif

    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem("Import Dxf", parentProgress);
    LaserDocument* laserDoc = new LaserDocument(scene);
    PageInformation page;
    page.setWidth(Global::convertUnit(laserDoc->unit(), SU_PX, LaserApplication::device->layoutWidth()));
    page.setHeight(Global::convertUnit(laserDoc->unit(), SU_PX, LaserApplication::device->layoutHeight(), Qt::Vertical));
    laserDoc->setPageInformation(page);
    laserDoc->blockSignals();

    qreal scaleX = Global::convertFromMM(SizeUnit::SU_PX, 1);
    qreal scaleY = Global::convertFromMM(SizeUnit::SU_PX, 1, Qt::Vertical);

    QTransform t(scaleX, 0, 0, -scaleY, 0, page.height()); //= QTransform::fromScale(scaleX, -scaleY).translate(0, -page.m_height());

    const DxfEntitiesNode& entities = d->documentNode->entities();
    progress->setMaximum(entities.length());
    for (DxfEntityNode* node : entities)
    {
        LaserPrimitive* primitive = node->convertTo(laserDoc, t);
        if (primitive)
        {
            laserDoc->addPrimitive(primitive);
        }
        progress->increaseProgress();
    }

    QFileInfo fileInfo(filename);
    QString docName = fileInfo.baseName();
    QRegularExpression re("^\\{.{8}-.{4}-.{4}-.{4}-.{12}\\}$");
    QRegularExpressionMatch match = re.match(docName);
    if (match.hasMatch())
    {
        docName = "root";
    }
    laserDoc->setName(docName);   
    laserDoc->blockSignals(false);
    emit imported();

    delete d->documentNode;
    d->documentNode = nullptr;
    progress->finish();
    return laserDoc;
}
