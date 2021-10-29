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
#include "scene/LaserScene.h"
#include "svg/qsvggraphics.h"
#include "svg/qsvgrenderer.h"
#include "svg/qsvgtinydocument.h"
#include "svg/qsvghandler.h"
#include "task/ProgressModel.h"
#include "ui/LaserControllerWindow.h"
#include "util/UnitUtils.h"

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

void SvgImporter::import(const QString & filename, LaserScene* scene, ProgressItem* parentProgress, const QVariantMap& params)
{
	LaserApplication::mainWindow->activateWindow();
    LaserDocument* doc = scene->document();

    //调用QSvgTinyDocument组件，解析并读取SVG文件。
    QSvgTinyDocument* svgDoc = QSvgTinyDocument::load(filename);
    int nodeCount = svgDoc->handler()->nodeCount();
    ProgressItem* progress = LaserApplication::progressModel->createSimpleItem("import svg", parentProgress);
    progress->setMaximum(nodeCount);
    //LaserDocument* laserDoc = new LaserDocument(scene);
	if (svgDoc == nullptr)
	{
		qWarning() << "Load SVG document failure!";
		return;
	}

    QRectF viewBox = svgDoc->viewBox();
    
    QSize svgSize = svgDoc->size();
    qreal docScaleWidth = svgSize.width() * 1.0 / viewBox.width();
    qreal docScaleHeight = svgSize.height() * 1.0 / viewBox.height();

    doc->blockSignals(true);

    qDebug() << "svg size:" << svgSize;
    qDebug() << "svg viewBox size:" << viewBox;
    qDebug() << "svg scale width:" << docScaleWidth;
    qDebug() << "svg scale height:" << docScaleHeight;

    QList<QSvgNode*> nodes = svgDoc->renderers();
    QStack<QSvgNode*> stack;
    for (int i = 0; i < nodes.length(); i++)
    {
        stack.push(nodes[i]);
    }

	QMatrix matrix;
	matrix.scale(Global::mm2PixelsXF(docScaleWidth), Global::mm2PixelsYF(docScaleHeight));

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
			QRectF bounds = matrix.mapRect(svgEllipseNode->bounds());
            item = new LaserEllipse(bounds, doc);
        }
            break;
        case QSvgNode::LINE:
        {
            QSvgLine* svgLineNode = reinterpret_cast<QSvgLine*>(node);
			QLineF line = matrix.map(svgLineNode->line());
            item = new LaserLine(line, doc);
        }
            break;
        case QSvgNode::ARC:
        case QSvgNode::PATH:
        {
            QSvgPath* svgPathNode = reinterpret_cast<QSvgPath*>(node);
			QPainterPath path = matrix.map(svgPathNode->path());
            item = new LaserPath(path, doc);
            int i = item->layerIndex();
        }
            break;
        case QSvgNode::POLYGON:
        {
            QSvgPolygon* svgPolygonNode = reinterpret_cast<QSvgPolygon*>(node);
			QPolygonF polygon = matrix.map(svgPolygonNode->polygon());
            item = new LaserPolygon(polygon, doc);
        }
            break;
        case QSvgNode::POLYLINE:
        {
            QSvgPolyline* svgPolylineNode = reinterpret_cast<QSvgPolyline*>(node);
			QPolygonF polyline = matrix.map(svgPolylineNode->polyline());
            item = new LaserPolyline(polyline, doc);
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
                    qreal cornerRaius = Global::mm2PixelsXF(svgRectNode->rx()) * docScaleWidth;
					item = new LaserRect(rect, cornerRaius, doc);
					qDebug() << "rect:" << rect;
				}
            }
            break;
        }
        case QSvgNode::TEXT:
        {
            QSvgText* svgTextNode = reinterpret_cast<QSvgText*>(node);
            QSvgStyleProperty* styleProperty = svgTextNode->styleProperty(QSvgStyleProperty::Type::FONT);
            QSvgFontStyle* fontStyle = reinterpret_cast<QSvgFontStyle*>(styleProperty);
            //QFont font("Microsoft YaHei", 24);
            QFont font;
            if (fontStyle)
            {
                QFont svgFont = fontStyle->qfont();
                font = QFont(svgFont);
                qreal fontSize = 100 / svgFont.pointSizeF() * docScaleWidth;
                qLogD << fontSize;
                //font.setPointSize(24);
                //font.setPixelSize(fontPixel);
                font.setPixelSize(Global::mm2PixelsX(fontSize) * 3);
                //qreal scale = 100.0 / font.pointSizeF();
                //font.setPixelSize(Global::mm2PixelsXF(font.pointSizeF() * scale * docScaleWidth));
                //font.setPointSizeF(svgFont.pointSizeF() * docScaleWidth);
                //font.setPointSize(24);
                QFontMetricsF fm(svgFont);
                //font.setPixelSize(fm.height());
                //qLogD << font.family() << ", " << font.pixelSize() << ", " << font.pointSize() << ", " << font.pointSizeF();
            }
            qLogD << font;
            QPointF pos = matrix.map(svgTextNode->coord());
            LaserText* laserText = new LaserText(doc, pos, font, Qt::AlignLeft, Qt::AlignVCenter);
            laserText->setContent(svgTextNode->text());
            laserText->modifyPathList();
            item = laserText;
            break;
        }
        case QSvgNode::TEXTAREA:
            break;
        case QSvgNode::IMAGE:
        {
            QSvgImage* svgImageNode = reinterpret_cast<QSvgImage*>(node);
			QRectF bounds = matrix.mapRect(svgImageNode->imageBounds());
            item = new LaserBitmap(svgImageNode->image(), bounds, doc);
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

            //item->moveToThread(LaserApplication::mainThread);
            scene->addLaserPrimitive(item);
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
    doc->blockSignals(false);
    emit imported();
    progress->finish();
}
