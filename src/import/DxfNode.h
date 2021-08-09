#ifndef DXFNODE_H
#define DXFNODE_H

#include <QtCore>
#include <QColor>
#include <QPair>
#include <QPointF>
#include <QTextStream>
#include <QVector2D>
#include <QVector3D>

class LaserDocument;
class LaserPrimitive;

enum DxfNodeType
{
    NT_Unknown,
    NT_Document,
    NT_Header,
    NT_Classes,
    NT_Tables,
    NT_Blocks,
    NT_Entities,
    NT_Objects,
    NT_ThumbnailImage,
    NT_DrawingInterchangeFileFormats,
    NT_Class,
    NT_Table,
    NT_Block,
    NT_EndBlock,
    NT_Entity,
    NT_Arc,
    NT_LWPolyline,
    NT_Circle,
    NT_Ellipse,
    NT_Line,
    NT_Spline,
    NT_Text,
    NT_Polyline,
    NT_Object
};

struct DxfGroup
{
public:
    DxfGroup() 
        : groupCode(0)
        , variable()
        , matched(false)
    {}

    DxfGroup(int _groupCode, const QString& _variable)
        : groupCode(_groupCode)
        , variable(_variable)
        , matched(false)
        , needCache(false)
    {}

    bool match(int _groupCode, const QString& _variable)
    {
        matched = (groupCode == _groupCode && variable == _variable);
        return matched;
    }

    bool match(int _groupCode)
    {
        matched = groupCode == _groupCode;
        return matched;
    }

    bool match(const QString& _variable)
    {
        matched = variable == _variable;
        return matched;
    }

    bool assign(int& value, int base = 10) const
    {
        bool ok;
        value = variable.toInt(&ok, base);
        return ok;
    }

    bool assign(quint32& value, int base = 10) const
    {
        bool ok;
        value = variable.toUInt(&ok, base);
        return ok;
    }

    bool assign(qreal& value) const
    {
        bool ok;
        value = variable.toDouble(&ok);
        return ok;
    }

    void assign(QString& value) const
    {
        value = variable;
    }

    int groupCode;
    QString variable;
    bool matched;
    bool needCache;
};

class DxfStream : public QTextStream
{
public:
    explicit DxfStream(QIODevice* device);
    ~DxfStream()
    {
    }

    bool readGroup(DxfGroup& group);

    bool isCached() const;
    void setCached();
    int lineNumber() const;

private:
    int m_lineNumber;
    QString m_line;
    bool m_cached;
    int m_cachedGroupCode;
    QString m_cachedVariable;

    Q_DISABLE_COPY(DxfStream)
};

class DxfNode;
class DxfNodePrivate
{
    Q_DECLARE_PUBLIC(DxfNode)
public:
    DxfNodePrivate(DxfNode* ptr, DxfNodeType _nodeType)
        : q_ptr(ptr)
        , groupCode(0)
        , variable(QString())
        , name(QString())
        , parent(nullptr)
        , nodeType(_nodeType)
    {

    }

    ~DxfNodePrivate()
    {
        qDeleteAll(children);
    }

    DxfNode* q_ptr;

    int groupCode;
    QString variable;
    QString name;
    DxfNode* parent;
    QList<DxfNode*> children;
    DxfNodeType nodeType;
};

class DxfNode
{
public:
    typedef QMap<QString, QString> PropertyMap;

    ~DxfNode()
    {
    }

    int groupCode() const;
    QString variable() const;
    QString name() const;

    DxfNode* parent() const;
    void setParent(DxfNode* node);

    QList<DxfNode*>& children();
    DxfNode* addChildNode(DxfNode* node);

    DxfNodeType nodeType() const;

    virtual bool parse(DxfStream* stream) = 0;

    virtual void debugPrint() const;

protected:
    explicit DxfNode(int groupCode, const QString& variable, DxfNodePrivate* privateData);

    void setName(const QString& name);

    QScopedPointer<DxfNodePrivate> m_ptr;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfNode)
    Q_DISABLE_COPY(DxfNode)

    friend class DxfCollectionNode;
    friend class DxfHeaderNode;
    friend class DxfTablesNode;
    friend class DxfClassesNode;
    friend class DxfBlocksNode;
    friend class DxfEntitiesNode;
    friend class DxfObjectsNode;
    friend class DxfItemNode;
    friend class DxfTableNode;
    friend class DxfClassNode;
    friend class DxfBlockNode;
    friend class DxfEntityNode;
};

class DxfDocumentNodePrivate;
class DxfDocumentNode : public DxfNode
{
public:
    explicit DxfDocumentNode();
    ~DxfDocumentNode()
    {
    }

    const DxfEntitiesNode& entities() const;

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;
private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfDocumentNode)
    Q_DISABLE_COPY(DxfDocumentNode)

};

class DxfHeaderNodePrivate;
class DxfHeaderNode : public DxfNode
{
public:
    explicit DxfHeaderNode(int groupCode = 2);
    ~DxfHeaderNode()
    {

    }

    DxfNode::PropertyMap& properties();

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;
private:
    void setProperty(const QString& name, const QString& value);

    Q_DECLARE_PRIVATE_D(m_ptr, DxfHeaderNode)
    Q_DISABLE_COPY(DxfHeaderNode)
};

class DxfClassNodePrivate;
class DxfClassNode : public DxfNode
{
public:
    explicit DxfClassNode(int groupCode = 0);
    ~DxfClassNode()
    {

    }

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;
private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfClassNode)
    Q_DISABLE_COPY(DxfClassNode)
};

class DxfItemNodePrivate;
class DxfItemNode : public DxfNode
{
public:
    explicit DxfItemNode(int groupCode, const QString& variable, DxfItemNodePrivate* ptr);
    ~DxfItemNode() {}

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;
protected:
    virtual bool parseItem(DxfGroup& group) = 0;
    virtual bool isEnd(DxfGroup& group) = 0;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfItemNode)
    Q_DISABLE_COPY(DxfItemNode)
};

class DxfTableNodePrivate;
class DxfTableNode : public DxfItemNode
{
public:
    explicit DxfTableNode(int groupCode = 0);
    ~DxfTableNode() {}

    virtual void debugPrint() const override;
    
protected:
    virtual bool parseItem(DxfGroup& group) override;
    virtual bool isEnd(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfTableNode)
    Q_DISABLE_COPY(DxfTableNode)
};

class DxfBlockNodePrivate;
class DxfBlockNode : public DxfItemNode
{
public:
    explicit DxfBlockNode(int groupCode = 0);
    ~DxfBlockNode() {}

    virtual void debugPrint() const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;
    virtual bool isEnd(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfBlockNode)
    Q_DISABLE_COPY(DxfBlockNode)
};

class DxfEndBlockNodePrivate;
class DxfEndBlockNode : public DxfItemNode
{
public:
    explicit DxfEndBlockNode(int groupCode = 0);
    ~DxfEndBlockNode() {}

    virtual void debugPrint() const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;
    virtual bool isEnd(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfEndBlockNode)
    Q_DISABLE_COPY(DxfEndBlockNode)
};

class DxfEntityNodePrivate;
class DxfEntityNode : public DxfItemNode
{
public:
    explicit DxfEntityNode(int groupCode, const QString& variable, DxfEntityNodePrivate* ptr);
    explicit DxfEntityNode(int groupCode, const QString& variable);
    ~DxfEntityNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const { return nullptr; }

protected:
    virtual bool parseItem(DxfGroup& group) override;
    virtual bool isEnd(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfEntityNode)
    Q_DISABLE_COPY(DxfEntityNode)
};

class DxfLWPolylineNodePrivate;
class DxfLWPolylineNode : public DxfEntityNode
{
public:
    explicit DxfLWPolylineNode(int groupCode = 0);
    ~DxfLWPolylineNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfLWPolylineNode)
    Q_DISABLE_COPY(DxfLWPolylineNode)
};

class DxfCircleNodePrivate;
class DxfCircleNode : public DxfEntityNode
{
public:
    explicit DxfCircleNode(int groupCode = 0);
    ~DxfCircleNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfCircleNode)
    Q_DISABLE_COPY(DxfCircleNode)
};

class DxfEllipseNodePrivate;
class DxfEllipseNode : public DxfEntityNode
{
public:
    explicit DxfEllipseNode(int groupCode = 0);
    ~DxfEllipseNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfEllipseNode)
    Q_DISABLE_COPY(DxfEllipseNode)
};

class DxfLineNodePrivate;
class DxfLineNode : public DxfEntityNode
{
public:
    explicit DxfLineNode(int groupCode = 0);
    ~DxfLineNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfLineNode)
    Q_DISABLE_COPY(DxfLineNode)
};

class DxfSplineNodePrivate;
class DxfSplineNode : public DxfEntityNode
{
public:
    explicit DxfSplineNode(int groupCode = 0);
    ~DxfSplineNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const override;

    bool isClosed() const;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfSplineNode)
    Q_DISABLE_COPY(DxfSplineNode)
};

class DxfArcNodePrivate;
class DxfArcNode : public DxfEntityNode
{
public:
    explicit DxfArcNode(int groupCode = 0);
    ~DxfArcNode() {}

    virtual void debugPrint() const override;

    virtual LaserPrimitive* convertTo(LaserDocument* doc, const QTransform& t) const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfArcNode)
    Q_DISABLE_COPY(DxfArcNode)
};

class DxfImageNodePrivate;
class DxfImageNode : public DxfEntityNode
{
public:
    explicit DxfImageNode(int groupCode = 0);
    ~DxfImageNode() {}

    virtual void debugPrint() const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfImageNode)
    Q_DISABLE_COPY(DxfImageNode)
};

class DxfObjectNodePrivate;
class DxfObjectNode : public DxfItemNode
{
public:
    explicit DxfObjectNode(int groupCode, const QString& variable, DxfObjectNodePrivate* ptr);
    explicit DxfObjectNode(int groupCode, const QString& variable);
    ~DxfObjectNode() {}

    virtual void debugPrint() const override;

protected:
    virtual bool parseItem(DxfGroup& group) override;
    virtual bool isEnd(DxfGroup& group) override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfObjectNode)
    Q_DISABLE_COPY(DxfObjectNode)
};

class DxfThumbnailImageNodePrivate;
class DxfThumbnailImageNode : public DxfNode
{
public:
    explicit DxfThumbnailImageNode(int groupCode = 2);
    ~DxfThumbnailImageNode()
    {

    }

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override
    {
    }
private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfThumbnailImageNode)
    Q_DISABLE_COPY(DxfThumbnailImageNode)
};

class DxfCollectionNodePrivate;
class DxfCollectionNode : public DxfNode
{
public:
    explicit DxfCollectionNode(int groupCode, const QString& variable, DxfCollectionNodePrivate* ptr);
    ~DxfCollectionNode()
    {

    }

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfCollectionNode)
    Q_DISABLE_COPY(DxfCollectionNode)
};

class DxfClassesNodePrivate;
class DxfClassesNode : public DxfCollectionNode, public QList<DxfClassNode*>
{
public:
    explicit DxfClassesNode(int groupCode = 2);
    ~DxfClassesNode() {}

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfClassesNode)
    Q_DISABLE_COPY(DxfClassesNode)
};

class DxfTablesNodePrivate;
class DxfTablesNode : public DxfCollectionNode, public QList<DxfTableNode*>
{
public:
    explicit DxfTablesNode(int groupCode = 2);
    ~DxfTablesNode() {}

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfTablesNode)
    Q_DISABLE_COPY(DxfTablesNode)
};

class DxfBlocksNodePrivate;
class DxfBlocksNode : public DxfCollectionNode, public QList<DxfBlockNode*>
{
public:
    explicit DxfBlocksNode(int groupCode = 2);
    ~DxfBlocksNode() {}

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfBlocksNode)
    Q_DISABLE_COPY(DxfBlocksNode)
};

class DxfEntitiesNodePrivate;
class DxfEntitiesNode : public DxfCollectionNode, public QList<DxfEntityNode*>
{
public:
    explicit DxfEntitiesNode(int groupCode = 2);
    ~DxfEntitiesNode() {}

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfEntitiesNode)
    Q_DISABLE_COPY(DxfEntitiesNode)
};

class DxfObjectsNodePrivate;
class DxfObjectsNode : public DxfCollectionNode, public QList<DxfObjectNode*>
{
public:
    explicit DxfObjectsNode(int groupCode = 2);
    ~DxfObjectsNode() {}

    virtual bool parse(DxfStream* stream) override;

    virtual void debugPrint() const override;

private:
    Q_DECLARE_PRIVATE_D(m_ptr, DxfObjectsNode)
    Q_DISABLE_COPY(DxfObjectsNode)
};



#endif // DXFNODE_H