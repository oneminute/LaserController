#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H
#include <QUndoCommand>
#include <QGraphicsItem>
#include "LaserViewer.h"
#include "scene/LaserPrimitive.h"
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
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);//根据sceneTransfrom转化为Item的transform和position
private:
	LaserViewer* m_viewer;
	QMap<QGraphicsItem*, QTransform> m_undoSelectedList;
	QMap<QGraphicsItem*, QTransform> m_redoSelectedList;
	void handle(QMap<QGraphicsItem*, QTransform> list);
	QTransform m_groupUndoTransform;
	QTransform m_groupRedoTransform;
	//ReshapeUndoCommand* m_reshapeCmd;
};
class TranformUndoCommand :public QUndoCommand {
public:
	TranformUndoCommand(LaserViewer* viewer,
		QMap<QGraphicsItem*, QTransform> undoList,
		QMap<QGraphicsItem*, QTransform> redoList);
	~TranformUndoCommand();
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);//根据sceneTransfrom转化为Item的transform和position
	virtual void undo() override;
	virtual void redo() override;
	void handle(QMap<QGraphicsItem*, QTransform> list);
private:
	LaserViewer * m_viewer;
	//LaserPrimitive* m_item;
	QMap<QGraphicsItem*, QTransform> m_undoList;
	QMap<QGraphicsItem*, QTransform> m_redoList;
	//QTransform m_undoTransform;
	//QTransform m_redoTransform;
};
class AddDelUndoCommand : public QUndoCommand {
public :
	AddDelUndoCommand(LaserScene* scene, QList<QGraphicsItem*> list, bool isDel = false);
	~AddDelUndoCommand();
	virtual void undo() override;
	virtual void redo() override;
	void sceneTransformToItemTransform(QTransform sceneTransform, QGraphicsItem* item);
private:
	LaserScene * m_scene;
	LaserViewer* m_viewer;
	QList<QGraphicsItem*> m_list;
	QMap<QGraphicsItem*, QTransform> m_selectedBeforeAdd;
	//QTransform m_addRedoTransform;
	//QTransform m_delRedoTransform;
	//QMap<QList<QGraphicsItem*>, QTransform> m_selectedBeforeAdd;
	bool m_isDel;
};
//多边形
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
//水平镜像
class MirrorHCommand : public QUndoCommand {
public:
	MirrorHCommand(LaserViewer* v);
	~MirrorHCommand();
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer* m_viewer;
};
//垂直镜像
class MirrorVCommand : public QUndoCommand {
public:
	MirrorVCommand(LaserViewer* v);
	~MirrorVCommand();
	virtual void undo() override;
	virtual void redo() override;
private:
	LaserViewer * m_viewer;
};
//黏贴
class PasteCommand : public QUndoCommand {
public:
	PasteCommand(LaserViewer* ,bool isPasteInline = false, bool isDuplication = false);
	~PasteCommand();
	virtual void undo() override;
	virtual void redo() override;
	void redoImp();
	void duplicationRedo();
private :
	LaserViewer *  m_viewer;
	QMap<QGraphicsItem*, QTransform> m_pastedBeforeAdd;
	QList<LaserPrimitive*> m_pasteList;
	LaserPrimitiveGroup* m_group;
	LaserScene* m_scene;
	bool m_isDuplication;
	bool m_isPasteInline;
	//QPointF m_position;

};
//交叉线镜像
class MirrorACommand : public QUndoCommand {
public:
    MirrorACommand(LaserViewer* v);
    ~MirrorACommand();
    virtual void undo() override;
    virtual void redo() override;
private:
    LaserViewer * m_viewer;
    LaserLine* m_line;
};
//Lock
class LockedCommand : public QUndoCommand {
public:
    LockedCommand(LaserViewer* v, QCheckBox* locked, Qt::CheckState lastState, QList<LaserPrimitive*> lockedList);
    ~LockedCommand();
    virtual void undo() override;
    virtual void redo() override;
    virtual void handle(Qt::CheckState state);
private:
    LaserViewer * m_viewer;
    LaserScene* m_scene;
    QCheckBox* m_locked;
    QList<LaserPrimitive*> m_lastLockedList;
    Qt::CheckState m_lastCheckState;
    Qt::CheckState m_curCheckState;
};
#endif // UNDOCOMMAND_H