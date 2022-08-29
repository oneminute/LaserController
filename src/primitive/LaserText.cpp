#include "LaserText.h"
#include "LaserShapePrivate.h"

#include <QPainter>

#include "LaserApplication.h"
#include "LaserTextRowPath.h"
#include "common/Config.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserScene.h"
#include "task/ProgressItem.h"
#include "ui/LaserControllerWindow.h"
#include "undo/PrimitiveAddingCommand.h"
#include "undo/PrimitiveRemovingCommand.h"
#include "util/Utils.h"
#include "util/MachiningUtils.h"

class LaserTextPrivate : public LaserShapePrivate
{
    Q_DECLARE_PUBLIC(LaserText)
public:
    LaserTextPrivate(LaserText* ptr)
        : LaserShapePrivate(ptr)
        , first(true)
        , insertIndex(0)
    {
        spaceY = LaserApplication::mainWindow->fontSpaceYDoubleSpinBox()->value();
    }
	QRect rect;
    QString content;
    QString lastContent;
    QPointF startPos;
    //QList<QPainterPath> pathList;
    //QMap<QPointF, QList<QPainterPath>> pathList;
    QList<LaserTextRowPath> pathList;
    //QPainterPath allPath;
    QFont font;
    int alignHType;
    int lastAlignHType;
    int alignVType;
    int lastAlignVType;
    QGraphicsView* view;
    qreal spaceY;

    QPoint textMousePressPos;
    bool first;
    int insertIndex;
};

LaserText::LaserText(LaserDocument* doc, QTransform transform, int layerIndex)
    : LaserText(doc, QPointF(), QFont(), 1000, Qt::AlignHCenter, Qt::AlignVCenter, transform, layerIndex)
{
}

LaserText::LaserText(LaserDocument* doc, QPointF startPos, QFont font, qreal spaceY, int alighHType, int alighVType, QTransform saveTransform, int layerIndex)
	: LaserShape(new LaserTextPrivate(this),  doc,  LPT_TEXT, layerIndex, saveTransform)
{
    Q_D(LaserText);
    d->outline.addRect(d->rect);
    d->font = font;
    d->alignHType = alighHType;
    d->lastAlignHType = alighHType;
    d->alignVType = alighVType;
    d->lastAlignVType = alighVType;
    d->startPos = mapFromScene(d->startPos);
    d->view = doc->scene()->views()[0];
    d->allTransform = saveTransform;
    d->spaceY = spaceY;
    sceneTransformToItemTransform(saveTransform);
}

LaserText::~LaserText()
{
}

bool LaserText::isFirst() const
{
    Q_D(const LaserText);
    return d->first;
}

void LaserText::setIsFirst(bool value)
{
    Q_D(LaserText);
    d->first = value;
}

QRect LaserText::rect() const
{
    Q_D(const LaserText);
    return d->rect; 
}

QString LaserText::content() const 
{
    Q_D(const LaserText);
    return d->content; 
}

void LaserText::setContent(QString c)
{
    Q_D(LaserText);
    d->content = c;
}

QPainterPath LaserText::path() const
{
    Q_D(const LaserText);
    /*QPainterPath paths;

    for (QMap<QPointF, QList<QPainterPath>>::const_iterator i = d->pathList.begin(); i != d->pathList.end(); i++) {
        QPainterPath rowPath;
        QPointF startPos = i.key();
        QList<QPainterPath> subRowPathList = i.value();
        for (QPainterPath path : subRowPathList) {
            rowPath.addPath(path);
            
        }
        
        paths.addPath(rowPath);
    }*/
    
    return d->path;
}

QVector<QLine> LaserText::edges()
{
    Q_D(const LaserText);
    return  LaserPrimitive::edges(sceneTransform().map(path()));
}

void LaserText::setFont(QFont font)
{
    Q_D(LaserText);
    d->font = font;
    modifyPathList();
}

QFont LaserText::font()
{
    Q_D(const LaserText);
    return d->font;
}

void LaserText::setAlignH(int a)
{
    Q_D(LaserText);
    d->alignHType = a;

}

int LaserText::alignH()
{
    Q_D(LaserText);
    return d->alignHType;
}

void LaserText::setAlignV(int a)
{
    Q_D(LaserText);
    d->alignVType = a;
}

int LaserText::alignV()
{
    Q_D(LaserText);
    return d->alignVType;
}

QPointF LaserText::startPos()
{
    Q_D(LaserText);
    return d->startPos;
}

void LaserText::setStartPos(const QPoint& pos)
{
    Q_D(LaserText);
    d->startPos = pos;
}

void LaserText::setSaveTransform(QTransform t)
{

}

QTransform LaserText::saveTransform()
{
    return QTransform();
}

/*void LaserText::setAlignType(int type)
{
    Q_D(LaserText);
    d->alignType = type;
}

int LaserText::alignType()
{
    Q_D(LaserText);
    return d->alignType;
}*/

void LaserText::insertContent(QString str, int index)
{
    Q_D(LaserText);
    d->lastContent = d->content;
    d->content.insert(index, str);
}

void LaserText::addPath(QString content, int insertIndex)
{
    Q_D(LaserText);
    insertContent(content, insertIndex);
    modifyPathList();
    d->view->viewport()->repaint();
}

void LaserText::delPath(int index)
{
    Q_D(LaserText);
    d->lastContent = d->content;
    d->content.remove(index, 1);
    modifyPathList();
    d->view->viewport()->repaint();
}

qreal LaserText::spaceY()
{
    Q_D(LaserText);
    return d->spaceY;
}

void LaserText::setSpacceY(qreal space)
{
    Q_D(LaserText);
    d->spaceY = space;
}

void LaserText::modifyPathList()
{
    Q_D(LaserText);
    QList<QList<QPainterPath>> subRowPathList;
    QList<QList<QRectF>> subBoundList;
    QList<QPainterPath> rowPathList;
    QList<QPointF> startPosList;

    QList<QPainterPath>* listPtr = &QList<QPainterPath>();
    QList<QRectF>* boundListPtr = &QList<QRectF>();

    QPainterPath rowPath;
    QPainterPath* rowPathPtr = &rowPath;

    int fontSize = d->font.pixelSize();
    qreal letterSpacing = d->font.letterSpacing();
    //qreal wordSpacing = d->font.wordSpacing();
    
    bool isNewLine = true;
    QRectF lastBound;
    //QPainterPath allPath;
    //QPainterPath lastPath;
    //qDebug() << d->lastContent;
    //qDebug() << d->content;
    //for (QChar c : d->content) {
    
    for (int i = 0; i < d->content.size(); i++) {
        QChar c = d->content[i];
        qreal rowY = (subRowPathList.size() + 1) * fontSize + subRowPathList.size()* spaceY() + d->startPos.y();
        QPointF startP(d->startPos.x(), rowY);
        if (c == "\n") {
            subRowPathList.append(*listPtr);
            rowPathList.append(*rowPathPtr);
            subBoundList.append(*boundListPtr);
            startPosList.append(startP);
            rowY = (subRowPathList.size() + 1) * fontSize + subRowPathList.size()* spaceY() + d->startPos.y();
            startP .setY(rowY);
            //换行
            isNewLine = true;
            listPtr = &QList<QPainterPath>();
            rowPathPtr = &QPainterPath();
            boundListPtr = &QList<QRectF>();
        }
        else {
            QPainterPath path;
            QTransform pathT;
            QRectF bound;
            QPointF pos;
            //top left
            if (isNewLine) {
                pos = startP;
            }
            else {
                pos = QPointF(lastBound.right() + d->font.letterSpacing(), rowY);
            }
            if (c == " ") {
                QFontMetrics m(d->font);
                qreal width = m.averageCharWidth();
                bound = QRectF(pos.x(), pos.y(), width, d->font.pixelSize());
            }
            else {
                path.addText(pos, d->font, c);
                bound = path.boundingRect();
            }
            
            listPtr->append(path);
            rowPathPtr->addPath(path);
            boundListPtr->append(bound);
            lastBound = bound;
            isNewLine = false;
        }
        if (i == d->content.size() - 1) {
            subRowPathList.append(*listPtr);
            rowPathList.append(*rowPathPtr);
            subBoundList.append(*boundListPtr);
            startPosList.append(startP);
        }
        //allPath.addPath(path);
        
        
    }
    qreal allHeight = fontSize * subRowPathList.size() + (subRowPathList.size()-1)* spaceY();
    d->pathList.clear();
    d->path = QPainterPath();
    for (int j = 0; j < subRowPathList.size(); j++) {
        QList<QPainterPath> subRowPath = subRowPathList[j];
        QList<QRectF> subRowBound = subBoundList[j];
        QPainterPath rowPath = rowPathList[j];
        QPointF startLeftTop = startPosList[j];
        qreal rowWidth = rowPath.boundingRect().width();
        QPointF diff;
        
        switch (d->alignHType) {
        case Qt::AlignLeft: {
            switch (d->alignVType) {
                case Qt::AlignTop: {
                    diff = QPointF(0, 0);
                    break;
                }
                case Qt::AlignBottom: {
                    diff = QPointF(0, -allHeight);
                    break;
                }
                case Qt::AlignVCenter: {
                    diff = QPointF(0, -allHeight*0.5);
                    break;
                }
            }
            break;
        }
        case Qt::AlignRight: {
            switch (d->alignVType) {
                case Qt::AlignTop: {
                    diff = QPointF(-rowWidth, 0);
                    break;
                }
                case Qt::AlignBottom: {
                    diff = QPointF(-rowWidth, -allHeight);
                    break;
                }
                case Qt::AlignVCenter: {
                    diff = QPointF(-rowWidth, -allHeight*0.5);
                    break;
                }   
            }
            break;
        }
        case Qt::AlignHCenter: {
            switch (d->alignVType) {
                case Qt::AlignTop: {
                    diff = QPointF(-rowWidth*0.5, 0);
                    break;
                }
                case Qt::AlignBottom: {
                    diff = QPointF(-rowWidth * 0.5, -allHeight);
                    break;
                }
                case Qt::AlignVCenter: {
                    diff = QPointF(-rowWidth * 0.5, -allHeight*0.5);
                    break;
                }
            }
            break;
        }
        }
        
        LaserTextRowPath rowPathStruct;
        QPainterPath newRowPath;
        QList<QPainterPath> newRowSubPaths;
        QList<QRectF> newRowBounds;
        QPointF rowLeftTop = startLeftTop + diff;
        for (int subIndex = 0; subIndex < subRowPath.size(); subIndex++ ) {
            QPainterPath subPath = subRowPath[subIndex];
            QRectF subBound = subRowBound[subIndex];

            //qDebug() << diff;
            subPath.translate(diff.x(), diff.y());
            subBound.translate(diff.x(), diff.y());

            newRowPath.addPath(subPath);
            newRowSubPaths.append(subPath);
            newRowBounds.append(subBound);
        }
        rowPathStruct.setPath(newRowPath);
        rowPathStruct.setLeftTop(rowLeftTop);
        rowPathStruct.setSubRowPathlist(newRowSubPaths);
        rowPathStruct.setSubRowBoundlist(newRowBounds);
        d->pathList.append(rowPathStruct);
        d->path.addPath(newRowPath);
    }
    
    d->boundingRect = d->path.boundingRect().toRect();
    document()->updateDocumentBounding();
}

QList<LaserTextRowPath> LaserText::subPathList()
{
    Q_D(const LaserText);

    return d->pathList;
}

int LaserText::detectInsertIndex(const QPoint& insertPoint)
{
    Q_D(LaserText);
    QPainterPath lastPath;
    int rowSize = d->pathList.size();
    int line = 0;
    qreal extend = 10.0 * 1000;

    //再遍历每一行的外包框
    for (LaserTextRowPath& rowPathStruct: d->pathList)
    {
        QList<QPainterPath> subRowPathlist = rowPathStruct.subRowPathlist();
        QList<QRectF> subRowBoundList = rowPathStruct.subRowBoundList();
        int subPathSize = subRowPathlist.size();
        //qreal worldSpacing = m_textFont.wordSpacing();
        QRectF rowPathBoundingRect = rowPathStruct.path().boundingRect();
        qreal halfWTopSpacing = 0;
        qreal halfWBottomSpacing = 0;
        //遍历每一行的外包框,只有一行直接遍历字符path
        bool isInRowPathBound = false;
        QRectF extendRowRect;
        if (rowSize > 1) {
            if (line + 1 < rowSize) {
                QRectF nextRowPathBoundingRect = rowPathStruct.path().boundingRect();
                halfWBottomSpacing = (nextRowPathBoundingRect.top() - rowPathBoundingRect.bottom()) * 0.5;
                if (halfWBottomSpacing < 0) {
                    halfWBottomSpacing = 0;
                }
            }
            if (line - 1 >= 0) {
                QRectF lastRowPathBoundingRect = rowPathStruct.path().boundingRect();
                halfWTopSpacing = (rowPathBoundingRect.top() - lastRowPathBoundingRect.bottom()) * 0.5;
                if (halfWTopSpacing < 0) {
                    halfWTopSpacing = 0;
                }
            }
            if (line == 0) {
                extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -extend),
                    rowPathBoundingRect.bottomRight() + QPointF(extend, halfWBottomSpacing));
            }
            else if (line == rowSize - 1) {

                extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -halfWTopSpacing),
                    rowPathBoundingRect.bottomRight() + QPointF(extend, extend));
            }
            else {
                extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -halfWTopSpacing),
                    rowPathBoundingRect.bottomRight() + QPointF(extend, halfWBottomSpacing));
            }
            if (sceneTransform().map(extendRowRect).containsPoint(insertPoint, Qt::OddEvenFill)) {
                isInRowPathBound = true;
            }

        }
        else {
            extendRowRect = QRectF(rowPathBoundingRect.topLeft() + QPointF(-extend, -extend),
                rowPathBoundingRect.bottomRight() + QPointF(extend, extend));
            isInRowPathBound = true;
        }
        if (!isInRowPathBound) {
            continue;
        }
        //遍历这一行的字符
        for (int j = 0; j < subPathSize; j++) {
            QPainterPath subPath = subRowPathlist[j];
            QRectF rect = subRowBoundList[j];
            //QRectF extendSubRect;
            qreal halfLLeftSpacing = 0;
            qreal halfLRightSpacing = 0;
            qreal halfWidth = rect.width() * 0.5;
            QRectF frontRect, backRect;
            if (j + 1 < subPathSize) {
                QRectF nextRect = subRowBoundList[j + 1];
                halfLRightSpacing = nextRect.left() - rect.right();
                if (halfLRightSpacing < 0) {
                    halfLRightSpacing = 0;
                }
            }
            if (j - 1 >= 0) {
                QRectF lastRect = subRowBoundList[j - 1];
                halfLLeftSpacing = rect.left() - lastRect.right();
                if (halfLLeftSpacing < 0) {
                    halfLLeftSpacing = 0;
                }
            }
            if (subPathSize == 1) {

                qreal cx = extendRowRect.left() + extendRowRect.width() * 0.5;
                frontRect = QRectF(extendRowRect.topLeft(), QPointF(cx, extendRowRect.bottom()));
                backRect = QRectF(QPointF(cx, extendRowRect.top()), extendRowRect.bottomRight());

            }
            else {
                qreal cx = rect.left() + rect.width() * 0.5;
                if (j == 0) {
                    frontRect = QRectF(extendRowRect.topLeft(), QPointF(cx, extendRowRect.bottom()));
                    backRect = QRectF(QPointF(cx, extendRowRect.top()),
                        QPointF(rect.right() + halfLRightSpacing, extendRowRect.bottom()));
                }
                else if (j == subPathSize - 1) {
                    frontRect = QRectF(QPointF(extendRowRect.left() - halfLLeftSpacing, extendRowRect.top()),
                        QPointF(QPointF(cx, extendRowRect.bottom())));
                    backRect = QRectF(QPointF(cx, extendRowRect.top()), extendRowRect.bottomRight());
                }
                else {
                    frontRect = QRectF(QPointF(extendRowRect.left() - halfLLeftSpacing, extendRowRect.top()),
                        QPointF(QPointF(cx, extendRowRect.bottom())));
                    backRect = QRectF(QPointF(cx, extendRowRect.top()),
                        QPointF(rect.right() + halfLRightSpacing, extendRowRect.bottom()));
                }
            }
            //前面
            if (sceneTransform().map(frontRect).containsPoint(insertPoint, Qt::OddEvenFill)) {
                int index = line - 1;
                d->insertIndex = 0;
                while (index >= 0) {
                    d->insertIndex += rowPathStruct.subRowPathlist().size() + 1;
                    index--;
                }
                d->insertIndex += j;

                return true;
            }
            //后面
            else if (sceneTransform().map(backRect).containsPoint(insertPoint, Qt::OddEvenFill)) {
                int index = line - 1;
                d->insertIndex = 0;
                while (index >= 0) {
                    d->insertIndex += subPathList()[index].subRowPathlist().size() + 1;
                    index--;
                }
                d->insertIndex += (j + 1);

                return true;

            }
        }
    }
    return 0;
}

QRectF LaserText::originalBoundingRect(qreal extendPixel) const
{
    
    Q_D(const LaserPrimitive);
    QRectF boundingRect = path().boundingRect();;
    qreal x = boundingRect.topLeft().x() - extendPixel;
    qreal y = boundingRect.topLeft().y() - extendPixel;
    qreal width = boundingRect.width() + 2 * extendPixel;
    qreal height = boundingRect.height() + 2 * extendPixel;
    QRectF rect = QRectF(x, y, width, height);
    return rect;
}

void LaserText::draw(QPainter * painter)
{
    Q_D(LaserText);
    painter->drawPath(mapFromScene(sceneTransform().map(path())));
    
}

void LaserText::sceneMousePressEvent(LaserViewer* viewer, LaserScene* scene,
    const QPoint& point, QMouseEvent* event)
{
}

void LaserText::sceneMouseMoveEvent(LaserViewer* viewer, LaserScene* scene,
    const QPoint& point, QMouseEvent* event, bool isPressed)
{
}

void LaserText::sceneMouseReleaseEvent(LaserViewer* viewer, LaserScene* scene,
    const QPoint& point, QMouseEvent* event, bool isPressed)
{
    Q_D(LaserText);
    if (isEditing())
    {
        if (d->first)
        {
            d->first = false;
            LaserLayer* layer = this->layer();

            PrimitiveAddingCommand* cmdAdding = new PrimitiveAddingCommand(
                tr("Add Line"), viewer, scene, this->document(), this->id(), 
                layer->id(), this);

            // we must ensure that when we undo the adding operation we should 
            // end the editing state in LaserViewer
            cmdAdding->setUndoCallback([=]()
                {
                    emit viewer->endEditing();
                }
            );
            // as we adding and editing the line, we must ensure that the
            // LaserViewer know it's in editing state
            cmdAdding->setRedoCallback([=]()
                {
                    viewer->setEditingPrimitiveId(id());
                    emit viewer->beginEditing();
                }
            );

            viewer->addUndoCommand(cmdAdding);
        }
        if (d->content.trimmed().isEmpty())
        {
            this->setStartPos(point);
        }
        else
        {
            detectInsertIndex(point);
        }
    }
}

void LaserText::sceneKeyPressEvent(LaserViewer* viewer, QKeyEvent* event)
{
    Q_D(LaserText);
    if (isEditing())
    {
        QString chr = utils::keyCodeToString(event->key(), viewer->isCapsLock(), 
            event->modifiers() == Qt::ShiftModifier);
        
        addPath(chr, d->insertIndex);
        d->insertIndex += chr.size();
    }
}

void LaserText::sceneKeyReleaseEvent(LaserViewer* viewer, QKeyEvent* event)
{
}

LaserPrimitive * LaserText::cloneImplement()
{
	Q_D(LaserText);
	LaserText* text = new LaserText(document(), d->startPos, d->font, d->spaceY, 
        d->alignHType, d->alignVType, sceneTransform(), d->layerIndex);
    text->setContent(d->content);
    text->modifyPathList();
    text->setIsFirst(isFirst());
	return text;
}

QJsonObject LaserText::toJson()
{
    Q_D(const LaserText);
    QJsonObject object;
    //QJsonArray position = { pos() .x(), pos() .y()};
    //QTransform transform = d->allTransform;
    QTransform transform = QTransform();
    QJsonArray matrix = {
        transform.m11(), transform.m12(), transform.m13(),
        transform.m21(), transform.m22(), transform.m23(),
        transform.m31(), transform.m32(), transform.m33()
    };
    QTransform parentTransform = this->sceneTransform();
    QJsonArray parentMatrix = { parentTransform.m11(), parentTransform.m12(), parentTransform.m13(), parentTransform.m21(), parentTransform.m22(), parentTransform.m23(), parentTransform.m31(), parentTransform.m32(), parentTransform.m33() };
    object.insert("parentMatrix", parentMatrix);
    object.insert("name", name());
    object.insert("className", this->metaObject()->className());
    object.insert("matrix", matrix);
    object.insert("layerIndex", layerIndex());
    //content
    object.insert("content", d->content);
    QJsonArray startPosArray{ d->startPos.x(), d->startPos.y() };
    object.insert("startPos", startPosArray);
    //font
    QJsonObject font;
    font.insert("family", d->font.family());
    font.insert("size", d->font.pixelSize());
    font.insert("bold", d->font.bold());
    font.insert("italic", d->font.italic());
    font.insert("upper", d->font.capitalization());
    font.insert("spaceX", d->font.letterSpacing());
    font.insert("letterSpaceTpye", d->font.letterSpacingType());
    font.insert("spaceY", d->font.wordSpacing());
    font.insert("alignH", d->alignHType);
    font.insert("alignV", d->alignVType);
    object.insert("font", font);
    return object;
}

bool LaserText::isClosed() const
{
    return false;
}

QPointF LaserText::position() const
{
    return QPointF();
}

LaserPointListList LaserText::updateMachiningPoints(ProgressItem* parentProgress)
{
    Q_D(LaserText);
    ProgressItem* progress = new ProgressItem(tr("%1 update machining points").arg(name()), ProgressItem::PT_Simple, parentProgress);
    int total = 0;
    for (int i = 0; i < d->pathList.size(); i++) {
        QList<QPainterPath> rowPathList = d->pathList[i].subRowPathlist();
        total += rowPathList.length();
    }
    progress->setMaximum(total);

    QTransform transform = sceneTransform();

    d->machiningPointsList.clear();
    d->startingIndices.clear();
    int totalPoints = 0;
    for (int i = 0; i < d->pathList.size(); i++) {
        QList<QPainterPath> rowPathList = d->pathList[i].subRowPathlist();
        for (QPainterPath rowPath : rowPathList) {
            QList<int> indices;
            LaserPointListList pointsList;
            QPoint center;
            machiningUtils::path2Points(nullptr, rowPath, pointsList, indices, center, transform);

            if (indices.length() <= Config::PathOptimization::maxStartingPoints())
            {
                for (int index : indices)
                {
                    d->startingIndices.append(index + totalPoints);
                }
            }
            else
            {
                for (int i = 0; i < Config::PathOptimization::maxStartingPoints(); i++)
                {
                    int index = i * indices.length() / Config::PathOptimization::maxStartingPoints();
                    d->startingIndices.append(indices.at(index) + totalPoints);
                }
            }
            
            d->machiningPointsList.append(pointsList);
            for (LaserPointList& list : pointsList)
            {
                totalPoints += list.count();
            }
            progress->increaseProgress();
        }
    }
    
    d->machiningCenter = transform.mapRect(path().boundingRect()).toRect().center();
    progress->finish();
    
    return d->machiningPointsList;
}

LaserLineListList LaserText::generateFillData(QPointF& lastPoint)
{
    Q_D(LaserText);
    QPainterPath path = sceneTransform().map(d->path);
    LaserLineListList lineList = utils::interLines(path, layer()->fillingRowInterval());
    return lineList;
}
