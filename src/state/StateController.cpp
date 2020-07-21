#include "StateController.h"


StateController::StateController(QObject* parent)
    :QObject(parent)
{
    m_fsm.addState(&m_stateInit);
    m_fsm.addState(&m_stateMain);
    m_fsm.addState(&m_stateDocument);
    m_fsm.addState(&m_stateMachining);
    m_fsm.setInitialState(&m_stateInit);

    //m_fsm.addTransition(this, SIGNAL(initToMain()), m_stateMain);
}

StateController& StateController::instance()
{
    static StateController controller;
    return controller;
}

