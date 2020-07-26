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

#define StateControllerInst StateController::instance()

class StateController : public QObject
{
    Q_OBJECT
private:
    explicit StateController(QObject* parent = nullptr);

public:
    static StateController& instance();

    QStateMachine& fsm() { return m_fsm; }

    QState& initState() { return m_stateInit; }

    QState& normalState() { return m_stateNormal; }

    QState& mainState() { return m_stateMain; }
    QState& mainNormalState() { return m_stateMainNormal; }
    QState& mainSingleSelectedState() { return m_stateMainSingleSelected; }
    QState& mainMultiSelectedState() { return m_stateMainMultiSelected; }
    QState& mainLayerSelectedState() { return m_stateMainLayerSelected; }
    QState& mainNewShapeState() { return m_stateMainNewShape; }

    QState& mainSingleSelectedNormalState() { return m_stateMainSingleSelectedNormal; }
    QState& mainSingleSelectedScalingState() { return m_stateMainSingleSelectedScaling; }
    QState& mainSingleSelectedRotatingState() { return m_stateMainSingleSelectedRotating; }

    QState& machiningState() { return m_stateMachining; }

    QFinalState& finalState() { return m_stateFinal; }

    static void start() { instance().fsm().start(); }

private slots:
    void onInitStateEntered();
    void onInitStateExited();
    void onNormalStateEntered();
    void onNormalStateExited();

private:
    QStateMachine m_fsm;
    QState m_stateInit;
    QState m_stateNormal;
    QState m_stateMain;
    QState m_stateMachining;

    QState m_stateMainNormal;
    QState m_stateMainSingleSelected;
    QState m_stateMainMultiSelected;
    QState m_stateMainLayerSelected;
    QState m_stateMainNewShape;
    QState m_stateMainAreaSelecting;

    QState m_stateMainSingleSelectedNormal;
    QState m_stateMainSingleSelectedScaling;
    QState m_stateMainSingleSelectedRotating;

    QFinalState m_stateFinal;
};

#endif // STATECONTROLLER_H