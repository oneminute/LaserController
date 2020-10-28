#include "StateController.h"

#include <QDebug>
#include <QFinalState>

StateController::StateController(QObject* parent)
    :QObject(parent)
{
    STATE_MACHINE_MEMBER = new QStateMachine();

    DEFINE_TOPLEVEL_INIT_STATE(init);
    DEFINE_TOPLEVEL_STATE(working);
    workingState->setChildMode(QState::ChildMode::ParallelStates);
    DEFINE_TOPLEVEL_FINAL_STATE(finish);

    DEFINE_CHILD_STATE(working, document);
    DEFINE_CHILD_STATE(working, device);

    DEFINE_CHILD_INIT_STATE(document, documentEmpty);
    DEFINE_CHILD_STATE(document, documentWorking);
    DEFINE_CHILD_INIT_STATE(documentWorking, documentIdle);
    DEFINE_CHILD_STATE(documentWorking, documentSelecting);
    DEFINE_CHILD_STATE(documentWorking, documentSelected);
    DEFINE_CHILD_STATE(documentWorking, documentTransforming);
    DEFINE_CHILD_STATE(documentWorking, documentPrimitive);

    DEFINE_CHILD_INIT_STATE(device, deviceUnconnected);
    DEFINE_CHILD_STATE(device, deviceConnected);
    DEFINE_CHILD_INIT_STATE(deviceConnected, deviceIdle);
    DEFINE_CHILD_STATE(deviceConnected, deviceMachining);
    DEFINE_CHILD_STATE(deviceConnected, devicePaused);
    DEFINE_CHILD_STATE(deviceConnected, deviceError);
}

StateController::~StateController()
{
    STATE_MACHINE_MEMBER->deleteLater();
}

StateController& StateController::instance()
{
    static StateController controller;
    return controller;
}

bool StateController::onState(QAbstractState * state)
{
    return instance().m_currentStates.contains(state->objectName());
}

bool StateController::anyState(const QList<QAbstractState*>& states)
{
    bool result = false;
    for (const QAbstractState* state : states)
    {
        result = result || instance().m_currentStates.contains(state->objectName());
    }
    return result;
}

bool StateController::allStates(const QList<QAbstractState*>& states)
{
    bool result = true;
    for (const QAbstractState* state : states)
    {
        result = result && instance().m_currentStates.contains(state->objectName());
    }
    return result;
}

