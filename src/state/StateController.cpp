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
    DEFINE_CHILD_INIT_STATE(documentWorking, documentNormal);
    DEFINE_CHILD_STATE(documentWorking, documentSelecting);
    DEFINE_CHILD_STATE(documentWorking, documentSelected);
    DEFINE_CHILD_STATE(documentWorking, documentTransforming);
    DEFINE_CHILD_STATE(documentWorking, documentPrimitive);

    DEFINE_CHILD_INIT_STATE(device, deviceUnconnected);
    DEFINE_CHILD_STATE(device, deviceConnected);
    DEFINE_CHILD_STATE(device, deviceMachining);
    DEFINE_CHILD_STATE(device, devicePause);
    DEFINE_CHILD_STATE(device, deviceError);

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

