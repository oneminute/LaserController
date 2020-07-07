/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qsvgtinydocument.h"
#include "svgview.h"

#include "qsvgrenderer.h"
#include "qgraphicssvgitem.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QPaintEvent>
#include <qmath.h>
#include <QStack>
#include <QDebug>

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

SvgView::SvgView(QWidget *parent)
    : QGraphicsView(parent)
    , m_renderer(Native)
//    , m_svgItem(nullptr)
    , m_backgroundItem(nullptr)
    , m_outlineItem(nullptr)
    , m_svgSize(0, 0)
{
    setScene(new QGraphicsScene(this));
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);

    // Prepare background check-board pattern
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 32, 32, color);
    tilePainter.fillRect(32, 32, 32, 32, color);
    tilePainter.end();

    setBackgroundBrush(tilePixmap);
}

void SvgView::drawBackground(QPainter *p, const QRectF &)
{
    p->save();
    p->resetTransform();
    p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
    p->restore();
}

QSize SvgView::svgSize() const
{
//    return m_svgItem ? m_svgItem->boundingRect().size().toSize() : QSize();
    return m_svgSize;
}

void showNode(QSvgNode* node, const QString& prefix)
{
    qDebug().noquote() << prefix << "id:" << node->nodeId() << "," << node->type() << ", disp:" << node->displayMode();
    if (node->type() >= 4)
        return;
    QSvgStructureNode* sNode = reinterpret_cast<QSvgStructureNode*>(node);
    if (sNode)
    {
        const QList<QSvgNode*>& children = sNode->renderers();
        for (int i = 0; i < children.length(); i++)
        {
            showNode(children[i], prefix + "  ");
        }
    }
}

bool SvgView::openFile(const QString &fileName)
{
    QGraphicsScene *s = scene();

    const bool drawBackground = (m_backgroundItem ? m_backgroundItem->isVisible() : false);
    const bool drawOutline = (m_outlineItem ? m_outlineItem->isVisible() : true);

    QSvgTinyDocument* doc = QSvgTinyDocument::load(fileName);
    m_svgSize = doc->size();

    QRectF boundingRect;
    boundingRect.setSize(m_svgSize);
    m_backgroundItem = new QGraphicsRectItem(boundingRect);
    m_backgroundItem->setBrush(Qt::white);
    m_backgroundItem->setPen(Qt::NoPen);
    m_backgroundItem->setVisible(drawBackground);
    m_backgroundItem->setZValue(-1);

    m_outlineItem = new QGraphicsRectItem(boundingRect);
    QPen outline(Qt::black, 2, Qt::DashLine);
    outline.setCosmetic(true);
    m_outlineItem->setPen(outline);
    m_outlineItem->setBrush(Qt::NoBrush);
    m_outlineItem->setVisible(drawOutline);
    m_outlineItem->setZValue(1);

    s->addItem(m_backgroundItem);

    QList<QSvgNode*> nodes = doc->renderers();
    QStack<QSvgNode*> stack;
    QList<QSvgRenderer*> renderers;
    for (int i = 0; i < nodes.length(); i++)
    {
        stack.push(nodes[i]);
    }

    s->clear();
    resetTransform();

    QRectF globalBoundings(QPointF(INT_MAX, INT_MAX), QPointF(0, 0));
    while (!stack.empty())
    {
        QSvgNode* node = stack.pop();
        QSvgRenderer* renderer = nullptr;
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
        case QSvgNode::ARC:
        case QSvgNode::CIRCLE:
        case QSvgNode::ELLIPSE:
        case QSvgNode::LINE:
        case QSvgNode::PATH:
        case QSvgNode::POLYGON:
        case QSvgNode::POLYLINE:
        case QSvgNode::RECT:
        case QSvgNode::TEXT:
        case QSvgNode::TEXTAREA:
        {
            if (node->hasStyle())
            {
                qDebug() << "    type:" << node->type() << ", display:" << node->displayMode() << node->styleProperty(QSvgStyleProperty::STROKE);
                renderer = new QSvgRenderer(node);
            }
        }
            break;
        case QSvgNode::IMAGE:
        {
            renderer = new QSvgRenderer(node);
        }
            break;
        default:
            break;
        }

        if (renderer)
        {
            renderers.append(renderer);

            QRectF boundings = renderer->bounds();
            qDebug().noquote() << boundings.left() << boundings.top() << boundings.right() << boundings.bottom();
            if (boundings.left() < globalBoundings.left())
                globalBoundings.setLeft(boundings.left());
            if (boundings.top() < globalBoundings.top())
                globalBoundings.setTop(boundings.top());
            if (boundings.right() > globalBoundings.right())
                globalBoundings.setRight(boundings.right());
            if (boundings.bottom() > globalBoundings.bottom())
                globalBoundings.setBottom(boundings.bottom());
        }
    }
    qreal xFactor = m_svgSize.width() / globalBoundings.width();
    qreal yFactor = m_svgSize.height() / globalBoundings.height();
    qreal factor = xFactor < yFactor ? xFactor : yFactor;
    qDebug().noquote().nospace() << "svg doc size: " << m_svgSize << ", global boundings: " << globalBoundings << ", xFactor: " << xFactor << ", yFactor: " << yFactor << ", factor: " << factor;
    qDebug().noquote() << globalBoundings.left() << globalBoundings.top() << globalBoundings.right() << globalBoundings.bottom();

    for (int i = 0; i < renderers.length(); i++)
    {
        QGraphicsSvgItem* svgItem = new QGraphicsSvgItem;
        renderers[i]->ajustTopLeft(globalBoundings.topLeft());
        svgItem->setFactor(factor);
        svgItem->setRenderer(renderers[i]);
        svgItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        //svgItem->setPos(renderers[i]->bounds().topLeft() - globalBoundings.topLeft());
        s->addItem(svgItem);
        m_svgItems.append(svgItem);
    }

    s->addItem(m_outlineItem);

    s->setSceneRect(m_outlineItem->boundingRect().adjusted(-10, -10, 10, 10));
    return true;
}

void SvgView::setRenderer(RendererType type)
{
    m_renderer = type;

    if (m_renderer == OpenGL) {
#ifndef QT_NO_OPENGL
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
    } else {
        setViewport(new QWidget);
    }
}

void SvgView::setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
    setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
    Q_UNUSED(highQualityAntialiasing);
#endif
}

void SvgView::setViewBackground(bool enable)
{
    if (!m_backgroundItem)
          return;

    m_backgroundItem->setVisible(enable);
}

void SvgView::setViewOutline(bool enable)
{
    if (!m_outlineItem)
        return;

    m_outlineItem->setVisible(enable);
}

qreal SvgView::zoomFactor() const
{
    return transform().m11();
}

void SvgView::zoomIn()
{
    zoomBy(2);
}

void SvgView::zoomOut()
{
    zoomBy(0.5);
}

void SvgView::resetZoom()
{
    if (!qFuzzyCompare(zoomFactor(), qreal(1))) {
        resetTransform();
        emit zoomChanged();
    }
}

void SvgView::paintEvent(QPaintEvent *event)
{
    if (m_renderer == Image) {
        if (m_image.size() != viewport()->size()) {
            m_image = QImage(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
        }

        QPainter imagePainter(&m_image);
        QGraphicsView::render(&imagePainter);
        imagePainter.end();

        QPainter p(viewport());
        p.drawImage(0, 0, m_image);

    } else {
        QGraphicsView::paintEvent(event);
    }
}

void SvgView::wheelEvent(QWheelEvent *event)
{
    zoomBy(qPow(1.2, event->delta() / 240.0));
}

void SvgView::zoomBy(qreal factor)
{
    const qreal currentZoom = zoomFactor();
    if ((factor < 1 && currentZoom < 0.01) || (factor > 1 && currentZoom > 10))
        return;
    scale(factor, factor);
    emit zoomChanged();
}

QSvgRenderer *SvgView::renderer() const
{
//    if (m_svgItem)
//        return m_svgItem->renderer();
    return nullptr;
}
