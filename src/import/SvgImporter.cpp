#include "SvgImporter.h"

#include <QDebug>
#include <QStack>
#include <QMainWindow>

#include "svg/qsvgtinydocument.h"
#include "svg/qsvgrenderer.h"
#include "svg/qsvggraphics.h"
#include "ui/ImportSVGDialog.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserDocument.h"
#include "util/UnitUtils.h"
#include "laser\LaserDriver.h"

SvgImporter::SvgImporter(QWidget* parentWnd, QObject* parent)
    : Importer(parentWnd, parent)
{

}

SvgImporter::~SvgImporter()
{
}

LaserDocument* SvgImporter::import(const QString & filename, LaserScene* scene, const QVariantMap& params)
{
    QMainWindow* parent = nullptr;
    if (params.contains("parent_win"))
    {
        parent = params["parent_win"].value<QMainWindow*>();
		parent->activateWindow();
		qDebug() << "is active window:" << parent->isActiveWindow();
    }

    LaserDocument* ldoc = new LaserDocument(scene);
    QSvgTinyDocument* doc = QSvgTinyDocument::load(filename);
    QRectF viewBox = doc->viewBox();
    if (doc == nullptr)
    {
		qWarning() << "Load SVG document failure!";
		ldoc->deleteLater();
        return nullptr;
    }
    QSize svgSize = doc->size();

    qreal docScaleWidth = svgSize.width() * 1.0 / viewBox.width();
    qreal docScaleHeight = svgSize.height() * 1.0 / viewBox.height();

    qDebug() << "document size:" << svgSize;
    qDebug() << "viewBox size:" << viewBox;
    qDebug() << "doc scale width:" << docScaleWidth;
    qDebug() << "doc scale height:" << docScaleHeight;

    SizeUnit docUnit;
    SizeUnit shapeUnit = SU_MM100;

	float width, height;
	if (!LaserDriver::instance().getLayout(width, height))
	{
		qWarning() << "Read register layout size failure!";
		ldoc->deleteLater();
		return nullptr;
	}
    PageInformation page;
    page.setWidth(width);
    page.setHeight(height);
    ldoc->setPageInformation(page);
    ldoc->blockSignals();

    qDebug() << "shapeUnit:" << shapeUnit;

    QList<QSvgNode*> nodes = doc->renderers();
    QStack<QSvgNode*> stack;
    QList<QSvgRenderer*> renderers;
    for (int i = 0; i < nodes.length(); i++)
    {
        stack.push(nodes[i]);
    }

    while (!stack.empty())
    {
        QSvgNode* node = stack.pop();
        QSvgRenderer* renderer = nullptr;
        LaserPrimitive* item = nullptr;
        switch (node->type())
        {
        case QSvgNode::DOC:
        case QSvgNode::G:
        case QSvgNode::DEFS:
        case QSvgNode::SWITCH:
        {
            QSvgStructureNode* sNode = reinterpret_cast<QSvgStructureNode*>(node);
            if (sNode)
            {
                const QList<QSvgNode*>& children = sNode->renderers();
                for (int i = 0; i < children.length(); i++)
                {
                    stack.push(children[i]);
                }
            }
        }
            break;
        case QSvgNode::CIRCLE:
        case QSvgNode::ELLIPSE:
        {
            QSvgEllipse* svgEllipseNode = reinterpret_cast<QSvgEllipse*>(node);
            item = new LaserEllipse(svgEllipseNode->bounds(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::LINE:
        {
            QSvgLine* svgLineNode = reinterpret_cast<QSvgLine*>(node);
            item = new LaserLine(svgLineNode->line(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::ARC:
        case QSvgNode::PATH:
        {
            QSvgPath* svgPathNode = reinterpret_cast<QSvgPath*>(node);
            item = new LaserPath(svgPathNode->path(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::POLYGON:
        {
            QSvgPolygon* svgPolygon = reinterpret_cast<QSvgPolygon*>(node);
            item = new LaserPolygon(svgPolygon->polygon(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::POLYLINE:
        {
            QSvgPolyline* svgPolylineNode = reinterpret_cast<QSvgPolyline*>(node);
            item = new LaserPolyline(svgPolylineNode->polyline(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::RECT:
        {
            if (node->hasStyle())
            {
                QSvgRect* svgRectNode = reinterpret_cast<QSvgRect*>(node);
                qreal area = svgRectNode->rect().width() * svgRectNode->rect().height();
                if (area > 0)
                    item = new LaserRect(svgRectNode->rect(), ldoc, shapeUnit);
            }
            break;
        }
        case QSvgNode::TEXT:
        case QSvgNode::TEXTAREA:
            break;
        case QSvgNode::IMAGE:
        {
            QSvgImage* svgImageNode = reinterpret_cast<QSvgImage*>(node);
            item = new LaserBitmap(svgImageNode->image(), svgImageNode->imageBounds(), ldoc, shapeUnit);
        }
            break;
        default:
            break;
        }

        if (item)
        {
            QTransform t;
            
            t = node->getCascadeTransform();

            QTransform tt = QTransform(t.m11(), t.m12(), t.m21(), t.m22(), t.dx() * docScaleWidth, t.dy() * docScaleHeight).scale(docScaleWidth, docScaleHeight);
            item->setTransform(tt);

            if (!node->nodeId().isEmpty() && !node->nodeId().isNull())
                item->setObjectName(node->nodeId());

            ldoc->addPrimitive(item);
        }
    }
    ldoc->blockSignals(false);
    
    emit imported();
    ldoc->open();
    return ldoc;
}
