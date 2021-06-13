#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include "common/common.h"
#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

#define STATE_INIT 0
#define STATE_MAIN 100
#define STATE_DOCUMENT 200
#define STATE_MACHINING 300

#define DEBUG_STATES true
#define STATE_MACHINE_MEMBER m_fsm
#define STATES_MEMBER m_states
#define STATES_CURRENT m_currentStates

#define StateControllerInst StateController::instance()

#define DECL_STATE(state) \
    public: \
        QState* state##State() const { return reinterpret_cast<QState*>(STATES_MEMBER[#state]); } \
    public slots: \
        void on##state##StateEntered() \
        { \
            if (DEBUG_STATES) \
                qDebug().noquote() << "enter state " << #state; \
            STATES_CURRENT.insert(state##State()->objectName()); \
        } \
        void on##state##StateExited() \
        { \
            if (DEBUG_STATES) \
                qDebug().noquote() << "exit state " << #state; \
            STATES_CURRENT.remove(state##State()->objectName()); \
        }

#define DECL_FINAL_STATE(state) \
    public: \
        QFinalState* state##State() const { return reinterpret_cast<QFinalState*>(STATES_MEMBER[#state]); } \
    public slots: \
        void on##state##StateEntered() \
        { \
            if (DEBUG_STATES) \
                qDebug().noquote() << "enter state " << #state; \
            STATES_CURRENT.insert(state##State()->objectName()); \
        }

#define DEFINE_STATE(state) \
    QState* state##State = new QState(); \
    state##State->setObjectName(#state"State"); \
    connect(state##State, &QState::entered, this, &StateController::on##state##StateEntered); \
    connect(state##State, &QState::exited, this, &StateController::on##state##StateExited); \
    STATES_MEMBER.insert(#state, state##State); 

#define DEFINE_FINAL_STATE(state) \
    QFinalState* state##State = new QFinalState(); \
    state##State->setObjectName(#state"State"); \
    connect(state##State, &QState::entered, this, &StateController::on##state##StateEntered); \
    STATES_MEMBER.insert(#state, state##State); 

#define DEFINE_CHILD_STATE(parent, state) \
    DEFINE_STATE(state) \
    state##State->setParent(parent##State) 

#define DEFINE_CHILD_INIT_STATE(parent, state) \
    DEFINE_STATE(state) \
    state##State->setParent(parent##State); \
    parent##State->setInitialState(state##State)

#define DEFINE_CHILD_ERROR_STATE(parent, state) \
    DEFINE_STATE(state) \
    state##State->setParent(parent##State); \
    parent##State->setErrorState(state##State)

#define DEFINE_CHILD_FINAL_STATE(parent, state) \
    DEFINE_FINAL_STATE(state) \
    state##State->setParent(parent##State) 

#define DEFINE_TOPLEVEL_STATE(state) \
    DEFINE_STATE(state) \
    STATE_MACHINE_MEMBER->addState(state##State)

#define DEFINE_TOPLEVEL_INIT_STATE(state) \
    DEFINE_STATE(state) \
    STATE_MACHINE_MEMBER->addState(state##State); \
    STATE_MACHINE_MEMBER->setInitialState(state##State)

#define DEFINE_TOPLEVEL_ERROR_STATE(state) \
    DEFINE_STATE(state) \
    STATE_MACHINE_MEMBER->addState(state##State); \
    STATE_MACHINE_MEMBER->setErrorState(state##State)

#define DEFINE_TOPLEVEL_FINAL_STATE(state) \
    DEFINE_FINAL_STATE(state) \
    STATE_MACHINE_MEMBER->addState(state##State)

#define ADD_TRANSITION(from, to, sender, signal) \
    StateControllerInst.from()->addTransition(sender, signal, StateControllerInst.to())

#define REMOVE_TRANSITION(state, transition) \
    StateControllerInst.state()->removeTransition(reinterpret_cast<QAbstractTransition*>(transition))

#define BIND_PROP_TO_STATE(obj, prop, value, state) \
    StateControllerInst.state()->assignProperty(obj, prop, value)

class StateController : public QObject
{
    Q_OBJECT
private:
    explicit StateController(QObject* parent = nullptr);
    virtual ~StateController();

public:
    static StateController& instance();

    QStateMachine* fsm() { return STATE_MACHINE_MEMBER; }

    static void start() { instance().fsm()->start(); }
    static void stop() { instance().fsm()->stop(); }

    static bool isInState(QAbstractState* state);
    static bool anyState(const QList<QAbstractState*>& states);
    static bool allStates(const QList<QAbstractState*>& states);
    QSet<QString> currentStates();

#pragma region top level states
    DECL_STATE(init)

#pragma region working state
    DECL_STATE(working)

#pragma region document state
    DECL_STATE(documentEmpty)
    DECL_STATE(document)

#pragma region documentWorking state
    DECL_STATE(documentWorking)
    DECL_STATE(documentIdle)
    DECL_STATE(documentSelection)
    DECL_STATE(documentSelecting)
    DECL_STATE(documentSelected)
	DECL_STATE(documentSelectedEditing)
    DECL_STATE(documentTransforming)
    DECL_STATE(documentPrimitive)
	DECL_STATE(documentPrimitiveRect)
	DECL_STATE(documentPrimitiveRectReady)
	DECL_STATE(documentPrimitiveRectCreating)
	DECL_STATE(documentPrimitiveEllipse)
	DECL_STATE(documentPrimitiveEllipseReady)
	DECL_STATE(documentPrimitiveEllipseCreating)
	DECL_STATE(documentPrimitiveLine)
	DECL_STATE(documentPrimitiveLineReady)
	DECL_STATE(documentPrimitiveLineCreating)
	DECL_STATE(documentPrimitivePolygon)
	DECL_STATE(documentPrimitivePolygonReady)
	DECL_STATE(documentPrimitivePolygonStartRect)
	DECL_STATE(documentPrimitivePolygonCreating)
	DECL_STATE(documentPrimitiveSpline)
	DECL_STATE(documentPrimitiveSplineReady)
	DECL_STATE(documentPrimitiveSplineCreating)
	DECL_STATE(documentPrimitiveSplineEdit)
	DECL_STATE(documentPrimitiveText)
	DECL_STATE(documentPrimitiveTextReady)
	DECL_STATE(documentPrimitiveTextCreating)
#pragma endregion documentWorking state
#pragma region document state

#pragma region device state
    DECL_STATE(device)
    DECL_STATE(deviceUnconnected)

    DECL_STATE(deviceConnected)
    DECL_STATE(deviceIdle)
    //DECL_STATE(deviceDownloading)
    //DECL_STATE(deviceDownloaded)
    DECL_STATE(deviceMachining)
    DECL_STATE(devicePaused)
    DECL_STATE(deviceError)
#pragma endregion device state
#pragma endregion working state

    DECL_FINAL_STATE(finish)
#pragma endregion top level states

private:
    QStateMachine* m_fsm;

    QMap<QString, QAbstractState*> m_states;
    QSet<QString> m_currentStates;
};

#endif // STATECONTROLLER_H