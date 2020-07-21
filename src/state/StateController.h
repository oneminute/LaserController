#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

#define STATE_INIT 0
#define STATE_MAIN 100
#define STATE_DOCUMENT 200
#define STATE_MACHINING 300

class StateController : public QObject
{
    Q_OBJECT
private:
    explicit StateController(QObject* parent = nullptr);

public:
    static StateController& instance();

    QStateMachine& fsm() { return m_fsm; }

    QState& initState() { return m_stateInit; }
    QState& mainState() { return m_stateMain; }
    QState& documentState() { return m_stateDocument; }
    QState& machiningState() { return m_stateMachining; }
    QFinalState& finishedState() { return m_stateFinished; }

    static void start() { instance().fsm().start(); }

private:
    QStateMachine m_fsm;
    QState m_stateInit;
    QState m_stateMain;
    QState m_stateDocument;
    QState m_stateMachining;
    QFinalState m_stateFinished;
};

#endif // STATECONTROLLER_H