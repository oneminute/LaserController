#pragma once

#include "LaserStampBase.h"

class LaserStampTextPrivate;
class LaserStampText : public LaserStampBase {
    Q_OBJECT
public:
    LaserStampText(LaserStampTextPrivate* ptr, LaserDocument* doc, LaserPrimitiveType type, 
        QString content, QTransform transform = QTransform(), int layerIndex = 0, QSize size = QSize(), qreal space = 0, 
        bool bold = false, bool italic = false, bool uppercase = false, bool stampIntaglio = false, QString family = "Times New Roman", qreal weight = 0);
    virtual~LaserStampText();
    virtual void recompute() = 0;
    virtual void draw(QPainter* painter);
    void setContent(QString content);
    QString getContent();
    void setBold(bool bold);
    bool bold();
    void setWeight(qreal w);
    qreal weight();
    void setItalic(bool italic);
    bool italic();
    
    void setUppercase(bool uppercase);
    bool uppercase();
    void setFamily(QString family);
    QString family();
    qreal space();
    virtual void setTextHeight(qreal diff) = 0;
    virtual void setTextWidth(qreal diff) = 0;
    QSize textSize();
    virtual void setSpace(qreal space) = 0;
private:
    Q_DECLARE_PRIVATE_D(ILaserDocumentItem::d_ptr, LaserStampText)
    Q_DISABLE_COPY(LaserStampText)
};