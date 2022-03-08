#include "DxfImporter.h"

#include "common/common.h"
#include "DxfNode.h"
#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "scene/LaserDocument.h"
#include "scene/LaserScene.h"
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

void DxfImporter::importImpl(const QString& filename, LaserScene* scene, QList<LaserPrimitive*>& unavailables, ProgressItem* parentProgress, const QVariantMap& params)
{
    Q_D(DxfImporter);
    qLogD << "import from dxf";

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return;

    DxfStream stream(&file, parentProgress);

    d->documentNode = new DxfDocumentNode;
    if (!d->documentNode->parse(&stream))
    {
        delete d->documentNode;
        d->documentNode = nullptr;
        return;
    }

#ifdef _DEBUG
    //d->documentNode->debugPrint();
#endif

    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem("Import Dxf", parentProgress);
    LaserDocument* laserDoc = scene->document();
    laserDoc->blockSignals(true);

    QRect deviceRect = LaserApplication::device->layoutRect();

    // 毫米转微米
    QTransform t(1000, 0, 0, -1000, 0, deviceRect.height()); //= QTransform::fromScale(scaleX, -scaleY).translate(0, -page.m_height());

    const DxfEntitiesNode& entities = d->documentNode->entities();
    progress->setMaximum(entities.length());
    for (DxfEntityNode* node : entities)
    {
        LaserPrimitive* primitive = node->convertTo(laserDoc, t);
        if (primitive)
        {
            if (primitive->isAvailable())
                scene->document()->addPrimitive(primitive, false, false);
            else
                unavailables.append(primitive);
        }
        progress->increaseProgress();
    }

    laserDoc->blockSignals(false);
    emit imported();

    delete d->documentNode;
    d->documentNode = nullptr;
    progress->finish();
}
