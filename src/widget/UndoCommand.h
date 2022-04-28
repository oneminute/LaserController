#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H
#include <QUndoCommand>
#include <QGraphicsItem>
#include "LaserViewer.h"
#include "scene/LaserPrimitive.h"
#include "widget/LaserDoubleSpinBox.h"
#include <QCheckBox>
class SelectionUndoCommand :public QUndoCommand {
public :
	SelectionUndoCommand(LaserViewer * viewer,
		QMap<QGraphicsItem*, QTransform> undoList,
		QMap<QGraphicsItem*, QTransform> redoList);
	~SelectionUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	virtual bool mergeWith(const QUndoCommand *command) override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);//����sceneTransfromת��ΪItem��transform��position
private:
	LaserViewer* m_viewer;
	QMap<QGraphicsItem*, QTransform> m_undoSelectedList;
	QMap<QGraphicsItem*, QTransform> m_redoSelectedList;
	void handle(QMap<QGraphicsItem*, QTransform> list);
	QTransform m_groupUndoTransform;
	QTransform m_groupRedoTransform;
	//ReshapeUndoCommand* m_reshapeCmd;
};
class GroupTransformUndoCommand : public QUndoCommand {
public:
    GroupTransformUndoCommand(LaserScene* scene,  QTransform lastTransform , 
        QTransform curTransform, bool updataSelectionPanel = true);
    ~GroupTransformUndoCommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserScene * m_scene;
    LaserViewer* m_viewer;
    LaserPrimitiveGroup* m_group;
    QTransform m_lastTransform;
    QTransform m_curUndoTransform;
    QTransform m_curRedoTransform;
    QuadTreeNode* m_tree;
    bool isRedo;
    bool m_upDataSelectionPanel;
};
class SingleTransformUndoCommand : public QUndoCommand {
public:
    SingleTransformUndoCommand(LaserScene* scene, QTransform lastTransform, QTransform curTransform, LaserPrimitive* primitive);
    ~SingleTransformUndoCommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserScene * m_scene;
    LaserViewer* m_viewer;
    LaserPrimitive* m_primitive;
    QTransform m_lastTransform;
    QTransform m_curTransform;
    QuadTreeNode* m_tree;
    bool isRedo;
};
class AddDelUndoCommand : public QUndoCommand {
public :
	AddDelUndoCommand(LaserScene* scene, QList<QGraphicsItem*> list, bool isDel = false);
    AddDelUndoCommand(LaserScene* scene, QList<LaserPrimitive*> list, bool isDel = false);
	~AddDelUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
private:
	LaserScene * m_scene;
	LaserViewer* m_viewer;
	QList<QGraphicsItem*> m_list;
    QList<LaserPrimitive*> m_primitiveList;
	QMap<QGraphicsItem*, QTransform> m_selectedBeforeAdd;
	//QTransform m_addRedoTransform;
	//QTransform m_delRedoTransform;
	//QMap<QList<QGraphicsItem*>, QTransform> m_selectedBeforeAdd;
	bool m_isDel;
};
//Polygon
class PolygonUndoCommand : public QUndoCommand {
public:
	PolygonUndoCommand(LaserScene* scene, LaserPrimitive* lastPrimitive, LaserPrimitive* curPrimitive);
	~PolygonUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
private:
	LaserScene * m_scene;
	LaserPrimitive* m_lastItem;
	LaserPrimitive* m_curItem;
	LaserViewer* m_viewer;
	QMap<QGraphicsItem*, QTransform> m_selectedBeforeAdd;
};
//MirrorHCommand
class MirrorHCommand : public QUndoCommand {
public:
	MirrorHCommand(LaserViewer* v);
	~MirrorHCommand();
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer* m_viewer;
};
//MirrorVCommand
class MirrorVCommand : public QUndoCommand {
public:
	MirrorVCommand(LaserViewer* v);
	~MirrorVCommand();
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer * m_viewer;
};
//PasteCommand
class PasteCommand : public QUndoCommand {
public:
	PasteCommand(LaserViewer* ,bool isPasteInline = false, bool isDuplication = false);
	~PasteCommand();
	virtual void undo() override;
	virtual void redo() override;
	void addImp(bool isAddToTreeNode = false);
    void redoImp(bool isRedo);
private :
	LaserViewer *  m_viewer;
	QMap<QGraphicsItem*, QTransform> m_pastedBeforeAdd;
	QList<LaserPrimitive*> m_pasteList;
	LaserPrimitiveGroup* m_group;
	LaserScene* m_scene;
	bool m_isDuplication;
	bool m_isPasteInline;
    QuadTreeNode* m_quadTree;
    QPointF m_mouseRedoPos;
	//QPointF m_position;

};
//�����߾���
/*class MirrorACommand : public QUndoCommand {
public:
    MirrorACommand(LaserViewer* v);
    ~MirrorACommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_viewer;
    LaserLine* m_line;
};*/
//Lock
class LockedCommand : public QUndoCommand {
public:
    LockedCommand(LaserViewer* v, QCheckBox* locked, Qt::CheckState lastState, QList<LaserPrimitive*> lockedList);
    ~LockedCommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_viewer;
    LaserScene* m_scene;
    QCheckBox* m_locked;
    QList<LaserPrimitive*> m_lastLockedList;
    Qt::CheckState m_lastCheckState;
    Qt::CheckState m_curCheckState;
};
//corner radius
class CornerRadiusCommand :public QUndoCommand {
public:
    CornerRadiusCommand(LaserViewer*  view, QList<LaserPrimitive*>& list, 
        LaserDoubleSpinBox* cornerRadius, qreal curVal, bool _isMulti = false);
    ~CornerRadiusCommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_view;
    QList<LaserPrimitive*> m_rectList;
    QMap<LaserRect*, qreal> m_lastMultiRadiusMap;
    LaserDoubleSpinBox* m_cornerRadius;
    qreal m_lastRadius;
    qreal m_curRadius;
    LaserControllerWindow* m_window;
    bool m_isMulti;
};
//width,height
/*class RectCommand :public QUndoCommand {
public:
    RectCommand(LaserViewer*  view, QList<LaserPrimitive*>& list,
        LaserDoubleSpinBox* spinBox, QRectF curRect, bool _isMulti = false);
    ~RectCommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_view;
    QList<LaserPrimitive*> m_list;
    QMap<LaserPrimitive*, QRectF> m_lastMultiMap;
    LaserDoubleSpinBox* m_spinBox;
    QRectF m_lastRectF;
    QRectF m_curRectF;
    bool m_isMulti;
    int m_type;
};*/
//layer visible
class LayerVisibleCommand : public QUndoCommand {
public :
    LayerVisibleCommand(LaserViewer* v, LaserLayer* layer, bool checked);
    ~LayerVisibleCommand() {};
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_viewer;
    LaserPrimitiveGroup* m_group;
    LaserLayer* m_layer;
    bool m_checked;
    bool m_lastIsInGroup;
    bool m_lastSelected;
};
//group/ungroup
class JoinedGroupCommand : public QUndoCommand {
public:
    JoinedGroupCommand(LaserViewer* viewer, QAction* _joinedGroupAction, QAction* _joinedUngroupAction, bool _isUngroup = false);
    ~JoinedGroupCommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserScene * m_scene;
    LaserViewer * m_viewer;
    QList<QGraphicsItem*> m_list;
    QList<QSet<LaserPrimitive*>> m_groupUndoJoinedList;
    QList<QSet<LaserPrimitive*>> m_ungroupUndoJoinedList;
    //QSet<LaserPrimitive*>* m_groupJoinedSet;
    bool m_isUngroup;
    QAction* m_joinedGroupAction;
    QAction* m_joinedUngroupAction;
    void handleGroup();
    void handleUnGroup();
    void undoGroup();
    void undoUnGroup();
    void restoreJoinedGroup();
};
//arrange align
class ArrangeAlignCommand : public QUndoCommand {
public:
    ArrangeAlignCommand(LaserViewer* viewer, int type);
    ~ArrangeAlignCommand();
private:
    LaserViewer* m_viewer;
    LaserScene* m_scene;
    LaserPrimitiveGroup* m_group;
    QMap< LaserPrimitive *, QTransform > m_undoMap;
    int m_type;
    LaserPrimitive* m_alignTarget;
    //prevent repeate compute bounds， use with  Function:joinedGroupSceneBounds(LaserPrimitive* p)
    QMap<LaserPrimitive*, QRect> joinedGroupBoundsMap;
protected:
    virtual void undo() override;
    virtual void redo() override;
    bool moveByType(LaserPrimitive* p, QRect target, QRect src);
    QRect joinedGroupSceneBounds(LaserPrimitive* p);
};
//move to page
class ArrangeMoveToPageCommand : public QUndoCommand {
public:
    ArrangeMoveToPageCommand(LaserViewer* viewer, int type);
    ~ArrangeMoveToPageCommand();
protected:
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_viewer;
    LaserScene* m_scene;
    LaserPrimitiveGroup* m_group;
    QPointF m_undoPos;
    int m_type;
};
class CommonSelectionCommand : public QUndoCommand {
public:
    CommonSelectionCommand(LaserViewer* viewer, bool isInvert = false);
    ~CommonSelectionCommand();
private:
    LaserViewer * m_viewer;
    LaserPrimitiveGroup* m_group;
    QList<LaserPrimitive*> m_undoList;
    bool m_isInvert;
protected:
    virtual void undo() override;
    virtual void redo() override;
};
//distribute v-spaced/center h-spaced
class DistributeUndoCommand : public QUndoCommand {
public:
    DistributeUndoCommand(LaserViewer* viewer, int type);
    ~DistributeUndoCommand();
    void findTopAndBottomPrimitive(LaserPrimitive* & topPrimitive, LaserPrimitive* & bottomPrimitive);
protected:
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_viewer;
    LaserPrimitiveGroup* m_group;
    LaserScene* m_scene;
    int m_type;
    QMap<LaserPrimitive*, QTransform> m_undoMap;
    LaserPrimitive* m_frontestPrimitive;
    LaserPrimitive* m_backestPrimitive;
};
class WeldShapesUndoCommand : public QUndoCommand {
public:
    WeldShapesUndoCommand(LaserViewer* viewer, int type);
    ~WeldShapesUndoCommand();
private:
    LaserViewer * m_viewer;
    LaserScene* m_scene;
    bool m_isRedo;
    int m_type;
    LaserLayer* m_minLayer;
    QMap<LaserPrimitive*, bool> m_traversedMap;
    QMap<QList<LaserPrimitive*>, QPainterPath> m_weldShapesMap;
    QMap<LaserPrimitive*, LaserLayer*> m_layerMap;
    QSet<LaserPrimitive*>* m_weldJoinedGroup;
    QMap<LaserPrimitive*, QList<LaserPrimitive*>>m_weldAndOriginalsMap;
    QList<QSet<LaserPrimitive*>> m_originalJoinedGroupList;//if have traversed,the boolean value is true

    void initeTranversedMap();
    void comuptePath();
    void createNewPath();
    void handleRedo();
    void deleteJoinedGroup(QSet<LaserPrimitive*>* joinedGroup);

protected:
    virtual void undo() override;
    virtual void redo() override;
    
};

class StampTextSpinBoxUndoCommand : public QUndoCommand {
public:
    StampTextSpinBoxUndoCommand(LaserViewer* viewer, LaserStampText* p, LaserDoubleSpinBox* spinBox,
        qreal lastValue, qreal value, int type, bool isRedo = false);
    ~StampTextSpinBoxUndoCommand();
private:
    LaserViewer* m_viewer;
    LaserDoubleSpinBox* m_spinBox;
    qreal m_redoValue;
    qreal m_undoValue;
    int m_type;
    bool m_isRedo;
    LaserStampText* m_stampTextPrimitive;
protected:
    virtual void undo() override;
    virtual void redo() override;
};

class LaserPrimitiveSpinBoxUndoCommand :public QUndoCommand {
public:
    LaserPrimitiveSpinBoxUndoCommand(LaserViewer* viewer, LaserPrimitive* p, LaserDoubleSpinBox* spinBox,
        qreal lastValue, qreal value, int type, bool isRedo = false);
    LaserPrimitiveSpinBoxUndoCommand(LaserViewer* viewer, LaserPrimitive* p, 
        LaserDoubleSpinBox* spinBox1, LaserDoubleSpinBox* spinBox2,
        qreal lastValue1, qreal lastValue2, qreal value1, qreal value2, int type, bool isRedo = false);
    ~LaserPrimitiveSpinBoxUndoCommand();
private:
    LaserViewer* m_viewer;
    LaserDoubleSpinBox* m_spinBox1;
    LaserDoubleSpinBox* m_spinBox2;
    qreal m_redoValue1;
    qreal m_redoValue2;
    qreal m_undoValue1;
    qreal m_undoValue2;
    int m_type;
    bool m_isRedo;
    LaserPrimitive* m_primitive;
    int m_primitiveType;
protected:
    virtual void undo() override;
    virtual void redo() override;
    void handle(qreal _v1, qreal _v2);
};
#endif // UNDOCOMMAND_H
