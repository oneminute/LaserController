#ifndef TESTSTATECONTROLLER_H
#define TESTSTATECONTROLLER_H

#include <QtTest>

#include <state/StateController.h>

class QAction;

class TestStateController : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();

    void transitionTestCase();

    void cleanupTestCase();

private:
    QAction* m_toMainAction;
    QAction* m_toDocumentAction;
    QAction* m_toMachiningAction;
    QAction* m_toFinishedAction;
};

#endif // TESTSTATECONTROLLER_H