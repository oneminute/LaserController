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

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

LaserDocument* SvgImporter::import(const QString & filename, LaserScene* scene, const QVariantMap& params)
{
	Global::mainWindow->activateWindow();

    LaserDocument* ldoc = new LaserDocument(scene);
    QSvgTinyDocument* doc = QSvgTinyDocument::load(filename);
    QRectF viewBox = doc->viewBox();
    if (doc == nullptr)
    {
		qWarning() << "Load SVG document failure!";
		ldoc->deleteLater();
        return nullptr;
    }
	ldoc->setUnit(doc->sizeUnit());

    QSize svgSize = doc->size();
    qreal docScaleWidth = svgSize.width() * 1.0 / viewBox.width();
    qreal docScaleHeight = svgSize.height() * 1.0 / viewBox.height();

	float width, height;
	LaserDriver::instance().getLayout(width, height);
    PageInformation page;
    page.setWidth(Global::convertUnit(ldoc->unit(), SU_PX, width));
    page.setHeight(Global::convertUnit(ldoc->unit(), SU_PX, height, Qt::Vertical));
    ldoc->setPageInformation(page);
    ldoc->blockSignals();

    qDebug() << "shapeUnit:" << ldoc->unit();
    qDebug() << "document size:" << svgSize;
    qDebug() << "viewBox size:" << viewBox;
    qDebug() << "doc scale width:" << docScaleWidth;
    qDebug() << "doc scale height:" << docScaleHeight;
	qDebug() << "page size:" << page.width() << page.height();

    QList<QSvgNode*> nodes = doc->renderers();
    QStack<QSvgNode*> stack;
    QList<QSvgRenderer*> renderers;
    for (int i = 0; i < nodes.length(); i++)
    {
        stack.push(nodes[i]);
    }

	QMatrix matrix;
	matrix.scale(docScaleWidth * Global::convertUnit(ldoc->unit(), SU_PX, 1.0f), docScaleHeight * Global::convertUnit(ldoc->unit(), SU_PX, 1.0f, Qt::Vertical));

    while (!stack.empty())
    {
        QSvgNode* node = stack.pop();
		qDebug() << "node shape:" << node->type() << ", display mode:" << node->displayMode();
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
			QRectF bounds = matrix.mapRect(svgEllipseNode->bounds());
            item = new LaserEllipse(bounds, ldoc);
        }
            break;
        case QSvgNode::LINE:
        {
            QSvgLine* svgLineNode = reinterpret_cast<QSvgLine*>(node);
			QLineF line = matrix.map(svgLineNode->line());
            item = new LaserLine(line, ldoc);
        }
            break;
        case QSvgNode::ARC:
        case QSvgNode::PATH:
        {
            QSvgPath* svgPathNode = reinterpret_cast<QSvgPath*>(node);
			QPainterPath path = matrix.map(svgPathNode->path());
            item = new LaserPath(path, ldoc);
        }
            break;
        case QSvgNode::POLYGON:
        {
            QSvgPolygon* svgPolygonNode = reinterpret_cast<QSvgPolygon*>(node);
			QPolygonF polygon = matrix.map(svgPolygonNode->polygon());
            item = new LaserPolygon(polygon, ldoc);
        }
            break;
        case QSvgNode::POLYLINE:
        {
            QSvgPolyline* svgPolylineNode = reinterpret_cast<QSvgPolyline*>(node);
			QPolygonF polyline = matrix.map(svgPolylineNode->polyline());
            item = new LaserPolyline(polyline, ldoc);
        }
            break;
        case QSvgNode::RECT:
        {
            if (node->hasStyle())
            {
                QSvgRect* svgRectNode = reinterpret_cast<QSvgRect*>(node);
                qreal area = svgRectNode->rect().width() * svgRectNode->rect().height();
				if (area > 0)
				{
					QRectF rect = matrix.mapRect(svgRectNode->rect());
					item = new LaserRect(rect, ldoc);
					qDebug() << "rect:" << rect;
				}
            }
            break;
        }
        case QSvgNode::TEXT:
        case QSvgNode::TEXTAREA:
            break;
        case QSvgNode::IMAGE:
        {
            QSvgImage* svgImageNode = reinterpret_cast<QSvgImage*>(node);
			QRectF bounds = matrix.mapRect(svgImageNode->imageBounds());
            item = new LaserBitmap(svgImageNode->image(), bounds, ldoc);
        }
            break;
        default:
            break;
        }

        if (item)
        {
            QTransform t;
            t = node->getCascadeTransform();

			if (!qFuzzyCompare(t.dx(), 0) || !qFuzzyCompare(t.dy(), 0))
			{
				qDebug() << t;
				t = QTransform(
					t.m11(), t.m12(), t.m13(),
					t.m21(), t.m22(), t.m23(),
					Global::convertFromMM(SU_PX, t.m31() * docScaleWidth), Global::convertFromMM(SU_PX, t.m32() * docScaleHeight, Qt::Vertical), t.m33()
				);
				qDebug() << t;
			}
            item->setTransform(t);

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
