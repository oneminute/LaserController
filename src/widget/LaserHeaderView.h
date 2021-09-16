#ifndef LASERHEADERVIEW_H
#define LASERHEADERVIEW_H
#include <QHeaderView>
class LaserHeaderView : public QHeaderView {
    Q_OBJECT
public:
    LaserHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~LaserHeaderView();
    protected:
        void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
};
#endif 