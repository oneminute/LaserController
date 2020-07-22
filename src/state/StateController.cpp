#include "StateController.h"

#include <QDebug>

StateController::StateController(QObject* parent)
    :QObject(parent)
{
    m_stateMainNormal.setParent(&m_stateMain);
    m_stateMainSingleSelected.setParent(&m_stateMain);
    m_stateMainMultiSelected.setParent(&m_stateMain);
    m_stateMainLayerSelected.setParent(&m_stateMain);
    m_stateMainNewShape.setParent(&m_stateMain);
    m_stateMain.setInitialState(&m_stateMainNormal);

    m_fsm.addState(&m_stateInit);
    m_fsm.addState(&m_stateNormal);
    m_fsm.addState(&m_stateMain);
    m_fsm.addState(&m_stateMachining);
    m_fsm.addState(&m_stateFinal);
    m_fsm.setInitialState(&m_stateInit);

    connect(&m_stateInit, &QState::entered, this, &StateController::onInitStateEntered);
    connect(&m_stateInit, &QState::exited, this, &StateController::onInitStateExited);
    connect(&m_stateNormal, &QState::entered, this, &StateController::onNormalStateEntered);
    connect(&m_stateNormal, &QState::exited, this, &StateController::onNormalStateExited);

    //m_stateInit.addTransition(this, "initToMain", &m_stateMain);
    //m_fsm.addTransition(this, SIGNAL(initToMain()), m_stateMain);
}

StateController& StateController::instance()
{
    static StateController controller;
    return controller;
}

void StateController::onInitStateEntered()
{
    qDebug() << "init state entered.";
}

void StateController::onInitStateExited()
{
    qDebug() << "init state exited.";
}

void StateController::onNormalStateEntered()
{
    qDebug() << "normal state entered.";
}

void StateController::onNormalStateExited()
{
    qDebug() << "normal state exited.";
}

//void StateController::onInitToMain()
//{
//    emit initToMain();
//}

