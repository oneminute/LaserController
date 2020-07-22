#include "TestStateController.h"

#include <QDebug>
#include <QAction>
#include <QSignalSpy>
#include <QStateMachine>
#include <QTimer>

void TestStateController::initTestCase()
{
    m_toMainAction = new QAction();
    m_toDocumentAction = new QAction();
    m_toMachiningAction = new QAction();
    m_toFinishedAction = new QAction();

    StateController::instance().initState().addTransition(m_toMainAction, SIGNAL(triggered()), &StateController::instance().normalState());
    StateController::instance().normalState().addTransition(m_toDocumentAction, SIGNAL(triggered()), &StateController::instance().mainState());
    StateController::instance().mainState().addTransition(m_toMachiningAction, SIGNAL(triggered()), &StateController::instance().machiningState());
    StateController::instance().machiningState().addTransition(m_toDocumentAction, SIGNAL(triggered()), &StateController::instance().mainState());
    StateController::instance().mainState().addTransition(m_toFinishedAction, SIGNAL(triggered()), &StateController::instance().finalState());
}

void TestStateController::transitionTestCase()
{
    QSignalSpy spyStateMachineStarted(&StateController::instance().fsm(), SIGNAL(started()));
    QSignalSpy spyInitStateEntered(&StateController::instance().initState(), SIGNAL(entered()));
    QSignalSpy spyMainStateEntered(&StateController::instance().normalState(), SIGNAL(entered()));
    QSignalSpy spyDocumentStateEntered(&StateController::instance().mainState(), SIGNAL(entered()));
    QSignalSpy spyMachiningStateEntered(&StateController::instance().machiningState(), SIGNAL(entered()));
    QSignalSpy spyFinishedStateEntered(&StateController::instance().finalState(), SIGNAL(entered()));
    QSignalSpy spyStateMachineFinished(&StateController::instance().fsm(), SIGNAL(finished()));

    StateController::instance().fsm().start();
    QVERIFY(spyStateMachineStarted.wait());
    QCOMPARE(spyStateMachineStarted.count(), 1);
    QCOMPARE(spyInitStateEntered.count(), 1);

    QTimer::singleShot(100, m_toMainAction, SLOT(trigger()));
    QVERIFY(spyMainStateEntered.wait());
    QCOMPARE(spyMainStateEntered.count(), 1);

    QTimer::singleShot(100, m_toDocumentAction, SLOT(trigger()));
    QVERIFY(spyDocumentStateEntered.wait());
    QCOMPARE(spyDocumentStateEntered.count(), 1);

    QTimer::singleShot(100, m_toMachiningAction, SLOT(trigger()));
    QVERIFY(spyMachiningStateEntered.wait());
    QCOMPARE(spyMachiningStateEntered.count(), 1);

    QTimer::singleShot(100, m_toDocumentAction, SLOT(trigger()));
    QVERIFY(spyDocumentStateEntered.wait());
    QCOMPARE(spyDocumentStateEntered.count(), 2);

    QTimer::singleShot(100, m_toFinishedAction, SLOT(trigger()));
    QVERIFY(spyFinishedStateEntered.wait());
    QCOMPARE(spyFinishedStateEntered.count(), 1);
    QCOMPARE(spyStateMachineFinished.count(), 1);
}

void TestStateController::cleanupTestCase()
{
    delete m_toMainAction;
    delete m_toDocumentAction;
    delete m_toMachiningAction;
    delete m_toFinishedAction;
}
