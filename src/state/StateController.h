#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

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

private:
    QStateMachine m_fsm;
    QState m_stateInit;
    QState m_stateMain;
    QState m_stateDocument;
    QState m_stateMachining;
};

#endif // STATECONTROLLER_H