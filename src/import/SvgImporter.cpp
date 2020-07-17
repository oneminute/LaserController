#include "SvgImporter.h"

#include <QDebug>
#include <QStack>

#include "svg/qsvgtinydocument.h"
#include "svg/qsvgrenderer.h"
#include "svg/qsvggraphics.h"
#include "ui/ImportSVGDialog.h"
#include "scene/LaserItem.h"
#include "scene/LaserDocument.h"
#include "util/UnitUtils.h"

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

LaserDocument* SvgImporter::import(const QString & filename)
{
    LaserDocument* ldoc = new LaserDocument;
    ImportSVGDialog dialog;
    dialog.exec();

    QSvgTinyDocument* doc = QSvgTinyDocument::load(filename);
    QSize svgSize = doc->size();
    qDebug() << "document size:" << svgSize;

    SizeUnit docUnit;
    SizeUnit shapeUnit = SU_MM100;

    if (dialog.pageUnitFromSVG())
    {
        docUnit = doc->sizeUnit();
    }
    else
    {
        docUnit = dialog.pageSizeUnit();
    }

    PageInformation page;
    page.setWidth(svgSize.width());
    page.setHeight(svgSize.height());
    ldoc->setPageInformation(page);

    if (!dialog.shapeUnitFromSVG())
    {
        shapeUnit = dialog.shapeSizeUnit();
    }

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
        LaserItem* item = nullptr;
        qDebug() << node->nodeId() << node->type();
        switch (node->type())
        {
        case QSvgNode::DOC:
        {
            qDebug().noquote() << "Doc:" << node->transformedBounds();
        }
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
            item = new LaserEllipseItem(svgEllipseNode->bounds(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::LINE:
        {
            QSvgLine* svgLineNode = reinterpret_cast<QSvgLine*>(node);
            item = new LaserLineItem(svgLineNode->line(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::ARC:
        case QSvgNode::PATH:
        {
            QSvgPath* svgPathNode = reinterpret_cast<QSvgPath*>(node);
            item = new LaserPathItem(svgPathNode->path(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::POLYGON:
        {
            QSvgPolygon* svgPolygon = reinterpret_cast<QSvgPolygon*>(node);
            item = new LaserPolygonItem(svgPolygon->polygon(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::POLYLINE:
        {
            QSvgPolyline* svgPolylineNode = reinterpret_cast<QSvgPolyline*>(node);
            item = new LaserPolylineItem(svgPolylineNode->polyline(), ldoc, shapeUnit);
        }
            break;
        case QSvgNode::RECT:
        {
            if (node->hasStyle())
            {
                QSvgRect* svgRectNode = reinterpret_cast<QSvgRect*>(node);
                qreal area = svgRectNode->rect().width() * svgRectNode->rect().height();
                qDebug() << "svg rect's area:" << area;
                if (area > 0)
                    item = new LaserRectItem(svgRectNode->rect(), ldoc, shapeUnit);
            }
            break;
        }
        case QSvgNode::TEXT:
        case QSvgNode::TEXTAREA:
            break;
        case QSvgNode::IMAGE:
        {
            QSvgImage* svgImageNode = reinterpret_cast<QSvgImage*>(node);
            item = new LaserBitmapItem(svgImageNode->image(), svgImageNode->imageBounds(), ldoc, shapeUnit);
        }
            break;
        default:
            break;
        }

        if (item)
        {
            QTransform t;
            qreal ratio = unitUtils::unitToMM(item->unit());
            
            t = node->getCascadeTransform();
            qreal scaleX = ratio;
            qreal scaleY = ratio;
            
            QTransform tt = QTransform(t.m11(), t.m12(), t.m21(), t.m22(), t.dx() * ratio, t.dy() * ratio).scale(scaleX, scaleY);
            item->setTransform(tt);
            ldoc->addItem(item);
        }
    }
    
    
    return ldoc;
}
