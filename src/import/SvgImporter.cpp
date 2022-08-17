#include "SvgImporter.h"

#include <QDebug>
#include <QFileInfo>
#include <QMainWindow>
#include <QRegularExpression>
#include <QStack>

#include "LaserApplication.h"
#include "common/Config.h"
#include "laser/LaserDevice.h"
#include "laser/LaserDriver.h"
#include "primitive/LaserPrimitiveHeaders.h"
#include "scene/LaserDocument.h"
#include "scene/LaserScene.h"
#include "svg/qsvggraphics.h"
#include "svg/qsvgrenderer.h"
#include "svg/qsvgtinydocument.h"
#include "svg/qsvghandler.h"
#include "task/ProgressModel.h"
#include "ui/LaserControllerWindow.h"
#include "util/UnitUtils.h"
#include "util/MachiningUtils.h"
#include "util/WidgetUtils.h"

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

void SvgImporter::importImpl(const QString & filename, LaserScene* scene, QList<LaserPrimitive*>& unavailables, ProgressItem* parentProgress, const QVariantMap& params)
{
	LaserApplication::mainWindow->activateWindow();
    LaserDocument* doc = scene->document();

    //调用QSvgTinyDocument组件，解析并读取SVG文件。
    QSvgTinyDocument* svgDoc = QSvgTinyDocument::load(filename);
    int nodeCount = svgDoc->handler()->nodeCount();
    ProgressItem* progress = new ProgressItem("import svg", ProgressItem::PT_Simple, parentProgress);
    progress->setMaximum(nodeCount);
	if (svgDoc == nullptr)
	{
		qWarning() << "Load SVG document failure!";
		return;
	}

    QRectF viewBox = svgDoc->viewBox();
    
    QSize svgSize = svgDoc->size();
    // 转换为微米
    qreal docScaleWidth = svgSize.width() * 1000.0 / viewBox.width();
    qreal docScaleHeight = svgSize.height() * 1000.0 / viewBox.height();

    QPoint originOffset = -LaserApplication::device->originOffset();

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
	matrix.scale(docScaleWidth, docScaleHeight);

    QList<QString> errors;

    while (!stack.empty())
    {
        QSvgNode* node = stack.pop();
        QSvgRenderer* renderer = nullptr;
        LaserPrimitive* item = nullptr;
        QList<LaserPrimitive*> items;

        QTransform t;
        t = node->getCascadeTransform();

        if (!qFuzzyCompare(t.dx(), 0) || !qFuzzyCompare(t.dy(), 0))
        {
            QTransform t1 = QTransform(
                t.m11(), t.m12(), t.m13(),
                t.m21(), t.m22(), t.m23(),
                t.m31() * docScaleWidth, t.m32() * docScaleHeight, t.m33()
            );
            t = t1;
            qDebug() << t;
        }
        t = QTransform(
            t.m11(), t.m12(), t.m13(),
            t.m21(), t.m22(), t.m23(),
            t.m31() + originOffset.x(), t.m32() + originOffset.y(), t.m33()
        );

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
            QPainterPath path;
            path.addEllipse(bounds);
            path = t.map(path);
            LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_CIRCLE);
            if (layer)
                item = new LaserPath(path, doc, QTransform(), layer->index());
            else
                errors << QString(tr("A circle or ellipse cannnot be imported as there's no capable layer to be added: %1"))
                .arg(QVariant(bounds).toString());
        }
            break;
        case QSvgNode::LINE:
        {
            QSvgLine* svgLineNode = reinterpret_cast<QSvgLine*>(node);
			QLineF line = matrix.map(svgLineNode->line());
            line = t.map(line);
            LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_LINE);
            if (layer)
                item = new LaserLine(line.toLine(), doc, QTransform(), layer->index());
            else
                errors << QString(tr("A line cannnot be imported as there's no capable layer to be added: %1"))
                .arg(QVariant(line).toString());
        }
            break;
        case QSvgNode::ARC:
        case QSvgNode::PATH:
        {
            QSvgPath* svgPathNode = reinterpret_cast<QSvgPath*>(node);
			QPainterPath path = matrix.map(svgPathNode->path());
            path = t.map(path);
            QList<QPolygon> subPaths = machiningUtils::path2SubpathPolygons(parentProgress, path, QTransform(), Config::Export::curveFlatteningThreshold());
            for (const QPolygon subPoly : subPaths)
            {
                QPainterPath subPath;
                subPath.addPolygon(subPoly);
                LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_POLYGON);
                if (layer)
                {
                    LaserPrimitive* subItem = new LaserPath(subPath, doc, QTransform(), layer->index());
                    items.append(subItem);
                }
                else
                    errors << QString(tr("A sub path of an arc or path cannnot be imported as there's no capable layer to be added."));
            }
        }
            break;
        case QSvgNode::POLYGON:
        {
            QSvgPolygon* svgPolygonNode = reinterpret_cast<QSvgPolygon*>(node);
			QPolygonF polygon = matrix.map(svgPolygonNode->polygon());
            polygon = t.map(polygon);
            LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_POLYGON);
            if (layer)
                item = new LaserPolygon(polygon.toPolygon(), doc, QTransform(), layer->index());
            else
                errors << QString(tr("A polygon cannnot be imported as there's no capable layer to be added."));
        }
            break;
        case QSvgNode::POLYLINE:
        {
            QSvgPolyline* svgPolylineNode = reinterpret_cast<QSvgPolyline*>(node);
			QPolygonF polyline = matrix.map(svgPolylineNode->polyline());
            polyline = t.map(polyline);
            LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_POLYGON);
            if (layer)
                item = new LaserPolyline(polyline.toPolygon(), doc, QTransform(), layer->index());
            else
                errors << QString(tr("A polyline cannnot be imported as there's no capable layer to be added."));
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
                    qreal cornerRadius = svgRectNode->rx() * docScaleWidth;
                    QPainterPath path;
                    path.addRoundedRect(rect, cornerRadius, cornerRadius);
                    path = t.map(path);
                    LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_POLYGON);
                    if (layer)
                        item = new LaserPath(path, doc, QTransform(), layer->index());
                    else
                        errors << QString(tr("A rectangle cannnot be imported as there's no capable layer to be added: %1")
                        .arg(QVariant(rect).toString()));
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
                //font.setPixelSize(Global::mm2PixelsX(fontSize) * 3);
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
            LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_TEXT);
            if (layer)
            {
                LaserText* laserText = new LaserText(doc, pos, font, 0, Qt::AlignLeft, Qt::AlignVCenter, QTransform(), layer->index());
                laserText->setContent(svgTextNode->text());
                laserText->modifyPathList();
                item = laserText;
            }
            else
            {
                errors << QString(tr("A text element cannnot be imported as there's no capable layer to be added: %1")
                .arg(svgTextNode->text()));
            }
            break;
        }
        case QSvgNode::TEXTAREA:
            break;
        case QSvgNode::IMAGE:
        {
            QSvgImage* svgImageNode = reinterpret_cast<QSvgImage*>(node);
			QRectF bounds = matrix.mapRect(svgImageNode->imageBounds());
            //bounds = t.mapRect(bounds);
            LaserLayer* layer = doc->getCurrentOrCapableLayer(LPT_BITMAP);
            if (layer)
                item = new LaserBitmap(svgImageNode->image(), bounds.toRect(), doc, t, layer->index());
            else
                errors << QString(tr("A bitmap cannnot be imported as there's no capable layer to be added: %1")
                .arg(QVariant(bounds).toString()));
        }
            break;
        default:
            break;
        }

        if (item)
        {
            //item->setTransform(t);
            items.append(item);
        }

        if (!items.isEmpty())
        {
            for (LaserPrimitive* item : items)
            {
                if (!node->nodeId().isEmpty() && !node->nodeId().isNull())
                    item->setName(node->nodeId());

                if (item)
                {
                    if (item->isAvailable())
                    {
                        doc->addPrimitive(item, false, false);
                    }
                    else
                        unavailables.append(item);
                }
            }
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
    if (!errors.isEmpty())
    {
        QString errorInfo = errors.join("\n");
        widgetUtils::showWarningMessage(LaserApplication::mainWindow,
            tr("Warning"), errorInfo);
    }
    emit imported();
    progress->finish();
}
