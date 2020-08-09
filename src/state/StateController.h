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

#define StateControllerInst StateController::instance()

#define DECL_STATE(state) \
    public: \
        QState* state##State() const { return reinterpret_cast<QState*>(STATES_MEMBER[#state]); } \
    public slots: \
        void on##state##StateEntered() { qDebug().noquote() << "enter state " << #state; } \
        void on##state##StateExited() { qDebug().noquote() << "exit state " << #state; }

#define DECL_FINAL_STATE(state) \
    public: \
        QFinalState* state##State() const { return reinterpret_cast<QFinalState*>(STATES_MEMBER[#state]); } \
    public slots: \
        void on##state##StateEntered() { qDebug().noquote() << "enter state " << #state; }

#define DEFINE_STATE(state) \
   QState* state##State = new QState(); \
    if (DEBUG_STATES) \
    { \
        connect(state##State, &QState::entered, this, &StateController::on##state##StateEntered); \
        connect(state##State, &QState::exited, this, &StateController::on##state##StateExited); \
    } \
    STATES_MEMBER.insert(#state, state##State); 

#define DEFINE_FINAL_STATE(state) \
   QFinalState* state##State = new QFinalState(); \
    if (DEBUG_STATES) \
    { \
        connect(state##State, &QState::entered, this, &StateController::on##state##StateEntered); \
    } \
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

    DECL_STATE(init)
    DECL_STATE(working)

    DECL_STATE(document)
    DECL_STATE(documentEmpty)
    DECL_STATE(documentWorking)
    DECL_STATE(documentNormal)
    DECL_STATE(documentSelecting)
    DECL_STATE(documentSelected)
    DECL_STATE(documentTransforming)
    DECL_STATE(documentPrimitive)

    DECL_STATE(device)
    DECL_STATE(deviceUnconnected)
    DECL_STATE(deviceConnected)
    DECL_STATE(deviceMachining)
    DECL_STATE(devicePause)
    DECL_STATE(deviceError)

    DECL_FINAL_STATE(finish)
private:
    QStateMachine* m_fsm;

    QMap<QString, QAbstractState*> m_states;
};

#endif // STATECONTROLLER_H