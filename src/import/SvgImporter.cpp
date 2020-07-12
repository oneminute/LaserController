#include "SvgImporter.h"

#include <QDebug>
#include <QStack>

#include "svg/qsvgtinydocument.h"
#include "svg/qsvgrenderer.h"
#include "svg/qsvggraphics.h"
#include "ui/ImportSVGDialog.h"
#include "scene/LaserItem.h"

SvgImporter::SvgImporter(QObject* parent)
    : Importer(parent)
{

}

SvgImporter::~SvgImporter()
{
}

LaserDocument SvgImporter::import(const QString & filename)
{
    LaserDocument* ldoc = new LaserDocument;
    ImportSVGDialog dialog;
    dialog.exec();

    QSvgTinyDocument* doc = QSvgTinyDocument::load(filename);
    QSize svgSize = doc->size();
    qDebug() << svgSize;

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
        case QSvgNode::ARC:
        case QSvgNode::CIRCLE:
        case QSvgNode::ELLIPSE:
        {
            QSvgEllipse* svgEllipseNode = reinterpret_cast<QSvgEllipse*>(node);
            item = new LaserEllipseItem(svgEllipseNode->bounds(), ldoc);
        }
            break;
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
                qDebug() << "    type:" << node->type() << ", display:" << node->displayMode() 
                    << node->styleProperty(QSvgStyleProperty::STROKE)
                    << node->transformedBounds();
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
        }
        if (item)
        {
            ldoc.addItem(item);
        }
    }
    
    
    return ldoc;
}
