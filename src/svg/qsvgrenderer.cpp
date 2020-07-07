/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt SVG module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsvgrenderer.h"

#ifndef QT_NO_SVGRENDERER

#include "qsvgtinydocument.h"

#include "qbytearray.h"
#include "qtimer.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

class QSvgRendererPrivate : public QSharedData
{
public:
    explicit QSvgRendererPrivate()
        : render(0)
        , timer(0)
        , fps(30)
    {}
    ~QSvgRendererPrivate()
    {
        delete render;
    }

    static void callRepaintNeeded(QSvgRenderer *const q);

    QSvgRenderer* q_ptr;
//    QSvgTinyDocument *render;
    QSvgNode* render;
    QTimer *timer;
    int fps;
};

/*!
    Constructs a new renderer with the given \a parent.
*/
QSvgRenderer::QSvgRenderer(QObject *parent)
    : QObject(parent)
    , d_ptr(new QSvgRendererPrivate)
{
    d_ptr->q_ptr = this;
}

QSvgRenderer::QSvgRenderer(QSvgNode *node, QObject *parent)
    : QObject(parent)
    , d_ptr(new QSvgRendererPrivate)
{
    d_ptr->render = node;
}

/*!
    Constructs a new renderer with the given \a parent and loads the contents of the
    SVG file with the specified \a filename.
*/
//QSvgRenderer::QSvgRenderer(const QString &filename, QObject *parent)
//    : QObject(parent)
//    , d_ptr(new QSvgRendererPrivate)
//{
//    d_ptr->q_ptr = this;
//    load(filename);
//}

/*!
    Constructs a new renderer with the given \a parent and loads the SVG data
    from the byte array specified by \a contents.
*/
//QSvgRenderer::QSvgRenderer(const QByteArray &contents, QObject *parent)
//    : QObject(parent)
//    , d_ptr(new QSvgRendererPrivate)
//{
//    d_ptr->q_ptr = this;
//    load(contents);
//}

/*!
    \since 4.5

    Constructs a new renderer with the given \a parent and loads the SVG data
    using the stream reader specified by \a contents.
*/
//QSvgRenderer::QSvgRenderer(QXmlStreamReader *contents, QObject *parent)
//    : QObject(parent)
//    , d_ptr(new QSvgRendererPrivate)
//{
//    d_ptr->q_ptr = this;
//    load(contents);
//}

/*!
    Destroys the renderer.
*/
QSvgRenderer::~QSvgRenderer()
{

}

/*!
    Returns true if there is a valid current document; otherwise returns false.
*/
bool QSvgRenderer::isValid() const
{
    return d_ptr->render;
}

/*!
    Returns the default size of the document contents.
*/
//QSize QSvgRenderer::defaultSize() const
//{
//    if (d_ptr->render)
//        return d_ptr->render->size();
//    else
//        return QSize();
//}

/*!
    Returns viewBoxF().toRect().

    \sa viewBoxF()
*/
//QRect QSvgRenderer::viewBox() const
//{
//    if (d_ptr->render)
//        return d_ptr->render->viewBox().toRect();
//    else
//        return QRect();
//}

/*!
    \property QSvgRenderer::viewBox
    \brief the rectangle specifying the visible area of the document in logical coordinates
    \since 4.2
*/
//void QSvgRenderer::setViewBox(const QRect &viewbox)
//{
//    if (d_ptr->render)
//        d_ptr->render->setViewBox(viewbox);
//}

/*!
    Returns true if the current document contains animated elements; otherwise
    returns false.

    \sa framesPerSecond()
*/
//bool QSvgRenderer::animated() const
//{
//    if (d_ptr->render)
//        return d_ptr->render->animated();
//    else
//        return false;
//}

/*!
    \property QSvgRenderer::framesPerSecond
    \brief the number of frames per second to be shown

    The number of frames per second is 0 if the current document is not animated.

    \sa animated()
*/
int QSvgRenderer::framesPerSecond() const
{
    return d_ptr->fps;
}

void QSvgRenderer::setFramesPerSecond(int num)
{
    if (num < 0) {
        qWarning("QSvgRenderer::setFramesPerSecond: Cannot set negative value %d", num);
        return;
    }
    d_ptr->fps = num;
}

/*!
  \property QSvgRenderer::currentFrame
  \brief the current frame of the document's animation, or 0 if the document is not animated
  \internal

  \sa animationDuration(), framesPerSecond, animated()
*/

/*!
  \internal
*/
//int QSvgRenderer::currentFrame() const
//{
//    return d_ptr->render->currentFrame();
//}

/*!
  \internal
*/
//void QSvgRenderer::setCurrentFrame(int frame)
//{
//    d_ptr->render->setCurrentFrame(frame);
//}

/*!
    \internal

    Returns the number of frames in the animation, or 0 if the current document is not
    animated.

    \sa animated(), framesPerSecond
*/
//int QSvgRenderer::animationDuration() const
//{
//    return d_ptr->render->animationDuration();
//}

/*!
 \internal
 \since 4.5

 We can't have template functions, that's loadDocument(), as friends, for this
 code, so we let this function be a friend of QSvgRenderer instead.
 */
void QSvgRendererPrivate::callRepaintNeeded(QSvgRenderer *const q)
{
    emit q->repaintNeeded();
}

//template<typename TInputType>
//static bool loadDocument(QSvgRenderer *const q,
//                         QSvgRendererPrivate *const d,
//                         const TInputType &in)
//{
//    delete d->render;
//    d->render = QSvgTinyDocument::load(in);
//    if (d->render && d->render->animated() && d->fps > 0) {
//        if (!d->timer)
//            d->timer = new QTimer(q);
//        else
//            d->timer->stop();
//        q->connect(d->timer, SIGNAL(timeout()),
//                   q, SIGNAL(repaintNeeded()));
//        d->timer->start(1000/d->fps);
//    } else if (d->timer) {
//        d->timer->stop();
//    }

//    //force first update
//    QSvgRendererPrivate::callRepaintNeeded(q);

//    return d->render;
//}

/*!
    Loads the SVG file specified by \a filename, returning true if the content
    was successfully parsed; otherwise returns false.
*/
//bool QSvgRenderer::load(const QString &filename)
//{
//    return loadDocument(this, d_ptr, filename);
//}

/*!
    Loads the specified SVG format \a contents, returning true if the content
    was successfully parsed; otherwise returns false.
*/
//bool QSvgRenderer::load(const QByteArray &contents)
//{
//    return loadDocument(this, d_ptr, contents);
//}

/*!
  Loads the specified SVG in \a contents, returning true if the content
  was successfully parsed; otherwise returns false.

  The reader will be used from where it currently is positioned. If \a contents
  is \c null, behavior is undefined.

  \since 4.5
*/
//bool QSvgRenderer::load(QXmlStreamReader *contents)
//{
//    return loadDocument(this, d_ptr, contents);
//}

/*!
    Renders the current document, or the current frame of an animated
    document, using the given \a painter.
*/
void QSvgRenderer::render(QPainter *painter, QSvgExtraStates &states)
{
    if (d_ptr->render) {
        d_ptr->render->draw(painter, states);
    }
}

/*!
    \fn void QSvgRenderer::repaintNeeded()

    This signal is emitted whenever the rendering of the document
    needs to be updated, usually for the purposes of animation.
*/

/*!
    Renders the given element with \a elementId using the given \a painter
    on the specified \a bounds. If the bounding rectangle is not specified
    the SVG element is mapped to the whole paint device.
*/
//void QSvgRenderer::render(QPainter *painter, const QString &elementId,
//                          const QRectF &bounds)
//{
//    if (d_ptr->render) {
//        d_ptr->render->draw(painter, elementId, bounds);
//    }
//}

/*!
    Renders the current document, or the current frame of an animated
    document, using the given \a painter on the specified \a bounds within
    the painter.  If the bounding rectangle is not specified
    the SVG file is mapped to the whole paint device.
*/
//void QSvgRenderer::render(QPainter *painter, const QRectF &bounds, QSvgExtraStates &states)
//{
//    if (d_ptr->render) {
//        d_ptr->render->draw(painter, bounds);
//    }
//}

//QRectF QSvgRenderer::viewBoxF() const
//{
//    if (d_ptr->render)
//        return d_ptr->render->viewBox();
//    else
//        return QRect();
//}

//void QSvgRenderer::setViewBox(const QRectF &viewbox)
//{
//    if (d_ptr->render)
//        d_ptr->render->setViewBox(viewbox);
//}

QRectF QSvgRenderer::bounds() const
{
    QRectF bounds;
    if (d_ptr->render)
        bounds = d_ptr->render->transformedBounds();
    return bounds;
}

void QSvgRenderer::ajustTopLeft(QPointF topLeft)
{
    if (d_ptr->render)
    {
        QRectF bounds = d_ptr->render->transformedBounds();
        bounds.setTopLeft(bounds.topLeft() - topLeft);
        bounds.setBottomRight(bounds.bottomRight() - topLeft);
        d_ptr->render->setCachedBounds(bounds);
    }
}

void QSvgRenderer::setNode(QSvgNode *node)
{
    d_ptr->render = node;
}

/*!
    \since 4.2

    Returns bounding rectangle of the item with the given \a id.
    The transformation matrix of parent elements is not affecting
    the bounds of the element.

    \sa matrixForElement()
*/
//QRectF QSvgRenderer::boundsOnElement(const QString &id) const
//{
//    QRectF bounds;
//    if (d_ptr->render)
//        bounds = d_ptr->render->boundsOnElement(id);
//    return bounds;
//}


/*!
    \since 4.2

    Returns true if the element with the given \a id exists
    in the currently parsed SVG file and is a renderable
    element.

    Note: this method returns true only for elements that
    can be rendered. Which implies that elements that are considered
    part of the fill/stroke style properties, e.g. radialGradients
    even tough marked with "id" attributes will not be found by this
    method.
*/
//bool QSvgRenderer::elementExists(const QString &id) const
//{
//    bool exists = false;
//    if (d_ptr->render)
//        exists = d_ptr->render->elementExists(id);
//    return exists;
//}

/*!
    \since 4.2

    Returns the transformation matrix for the element
    with the given \a id. The matrix is a product of
    the transformation of the element's parents. The transformation of
    the element itself is not included.

    To find the bounding rectangle of the element in logical coordinates,
    you can apply the matrix on the rectangle returned from boundsOnElement().

    \sa boundsOnElement()
*/
//QMatrix QSvgRenderer::matrixForElement(const QString &id) const
//{
//    QMatrix mat;
//    if (d_ptr->render)
//        mat = d_ptr->render->matrixForElement(id);
//    return mat;
//}

QT_END_NAMESPACE

#include "moc_qsvgrenderer.cpp"

#endif // QT_NO_SVGRENDERER
