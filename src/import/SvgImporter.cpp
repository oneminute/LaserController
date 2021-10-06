#include "SvgImporter.h"

#include <QDebug>
#include <QFileInfo>
#include <QMainWindow>
#include <QRegularExpression>
#include <QStack>

#include "LaserApplication.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"
#include "scene/LaserDocument.h"
#include "scene/LaserPrimitive.h"
#include "svg/qsvggraphics.h"
#include "svg/qsvgrenderer.h"
#include "svg/qsvgtinydocument.h"
#include "ui/ImportSVGDialog.h"
#include "ui/LaserControllerWindow.h"
#include "util/UnitUtils.h"

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

LaserDocument* SvgImporter::import(const QString & filename, LaserScene* scene, const QVariantMap& params)
{
	LaserApplication::mainWindow->activateWindow();

    //调用QSvgTinyDocument组件，解析并读取SVG文件。
    QSvgTinyDocument* svgDoc = QSvgTinyDocument::load(filename);
    LaserDocument* laserDoc = new LaserDocument(scene);
	if (svgDoc == nullptr)
	{
		qWarning() << "Load SVG document failure!";
		laserDoc->deleteLater();
		return nullptr;
	}

    QRectF viewBox = svgDoc->viewBox();
    
	laserDoc->setUnit(svgDoc->sizeUnit());

    QSize svgSize = svgDoc->size();
    qreal docScaleWidth = svgSize.width() * 1.0 / viewBox.width();
    qreal docScaleHeight = svgSize.height() * 1.0 / viewBox.height();

	//float layoutWidth, layoutHeight;
	//LaserDriver::instance().getLayout(layoutWidth, layoutHeight);
    PageInformation page;
    page.setWidth(Global::convertUnit(laserDoc->unit(), SU_PX, LaserApplication::device->layoutWidth()));
    page.setHeight(Global::convertUnit(laserDoc->unit(), SU_PX, LaserApplication::device->layoutHeight(), Qt::Vertical));
    laserDoc->setPageInformation(page);
    laserDoc->blockSignals();

    qDebug() << "shapeUnit:" << laserDoc->unit();
    qDebug() << "svg size:" << svgSize;
    qDebug() << "svg viewBox size:" << viewBox;
    qDebug() << "svg scale width:" << docScaleWidth;
    qDebug() << "svg scale height:" << docScaleHeight;
	qDebug() << "page size:" << page.width() << page.height();

    QList<QSvgNode*> nodes = svgDoc->renderers();
    QStack<QSvgNode*> stack;
    QList<QSvgRenderer*> renderers;
    for (int i = 0; i < nodes.length(); i++)
    {
        stack.push(nodes[i]);
    }

	QMatrix matrix;
	matrix.scale(docScaleWidth * Global::convertUnit(laserDoc->unit(), SU_PX, 1.0f), docScaleHeight * Global::convertUnit(laserDoc->unit(), SU_PX, 1.0f, Qt::Vertical));

    while (!stack.empty())
    {
        QSvgNode* node = stack.pop();
		//qDebug() << "node shape:" << node->type() << ", display mode:" << node->displayMode();
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
            item = new LaserEllipse(bounds, laserDoc);
        }
            break;
        case QSvgNode::LINE:
        {
            QSvgLine* svgLineNode = reinterpret_cast<QSvgLine*>(node);
			QLineF line = matrix.map(svgLineNode->line());
            item = new LaserLine(line, laserDoc);
        }
            break;
        case QSvgNode::ARC:
        case QSvgNode::PATH:
        {
            QSvgPath* svgPathNode = reinterpret_cast<QSvgPath*>(node);
			QPainterPath path = matrix.map(svgPathNode->path());
            item = new LaserPath(path, laserDoc);
            int i = item->layerIndex();
        }
            break;
        case QSvgNode::POLYGON:
        {
            QSvgPolygon* svgPolygonNode = reinterpret_cast<QSvgPolygon*>(node);
			QPolygonF polygon = matrix.map(svgPolygonNode->polygon());
            item = new LaserPolygon(polygon, laserDoc);
        }
            break;
        case QSvgNode::POLYLINE:
        {
            QSvgPolyline* svgPolylineNode = reinterpret_cast<QSvgPolyline*>(node);
			QPolygonF polyline = matrix.map(svgPolylineNode->polyline());
            item = new LaserPolyline(polyline, laserDoc);
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
                    qreal cornerRaius = Global::mm2PixelsXF(svgRectNode->rx());
					item = new LaserRect(rect, cornerRaius, laserDoc);
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
            item = new LaserBitmap(svgImageNode->image(), bounds, laserDoc);
        }
            break;
        default:
            break;
        }

        if (item)
        {
            QTransform t;
            t = node->getCascadeTransform();
	        //qDebug() << t;

			if (!qFuzzyCompare(t.dx(), 0) || !qFuzzyCompare(t.dy(), 0))
			{
                QTransform t1 = QTransform(
					t.m11(), t.m12(), t.m13(),
					t.m21(), t.m22(), t.m23(),
					//0, 0, t.m33()
					//t.m31() * docScaleWidth, t.m32() * docScaleHeight, t.m33()
					Global::convertFromMM(SU_PX, t.m31() * docScaleWidth), Global::convertFromMM(SU_PX, t.m32() * docScaleHeight, Qt::Vertical), t.m33()
				);
				
                t = t1;
				qDebug() << t;
			}
            item->setTransform(t);

            if (!node->nodeId().isEmpty() && !node->nodeId().isNull())
                item->setName(node->nodeId());

            laserDoc->addPrimitive(item);
        }
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
    laserDoc->open();
    return laserDoc;
}
