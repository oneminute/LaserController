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
    DEFINE_CHILD_STATE(documentWorking, documentSelection);
    DEFINE_CHILD_INIT_STATE(documentSelection, documentSelecting);
    DEFINE_CHILD_STATE(documentSelection, documentSelected);
    DEFINE_CHILD_STATE(documentWorking, documentTransforming);
    DEFINE_CHILD_STATE(documentWorking, documentPrimitive);

	DEFINE_CHILD_STATE(documentPrimitive, documentPrimitiveRect);
	DEFINE_CHILD_INIT_STATE(documentPrimitiveRect, documentPrimitiveRectReady);
	DEFINE_CHILD_STATE(documentPrimitiveRect, documentPrimitiveRectCreating);

	DEFINE_CHILD_STATE(documentPrimitive, documentPrimitiveEllipse);
	DEFINE_CHILD_INIT_STATE(documentPrimitiveEllipse, documentPrimitiveEllipseReady);
	DEFINE_CHILD_STATE(documentPrimitiveEllipse, documentPrimitiveEllipseCreating);

	DEFINE_CHILD_STATE(documentPrimitive, documentPrimitiveLine);
	DEFINE_CHILD_INIT_STATE(documentPrimitiveLine, documentPrimitiveLineReady);
	DEFINE_CHILD_STATE(documentPrimitiveLine, documentPrimitiveLineCreating);

	DEFINE_CHILD_STATE(documentPrimitive, documentPrimitivePolygon);
	DEFINE_CHILD_INIT_STATE(documentPrimitivePolygon, documentPrimitivePolygonReady);
	DEFINE_CHILD_STATE(documentPrimitivePolygon, documentPrimitivePolygonCreating);
	DEFINE_CHILD_STATE(documentPrimitivePolygon, documentPrimitivePolygonStartRect);

	DEFINE_CHILD_STATE(documentPrimitive, documentPrimitiveSpline);
	DEFINE_CHILD_INIT_STATE(documentPrimitiveSpline, documentPrimitiveSplineReady);
	DEFINE_CHILD_STATE(documentPrimitiveSpline, documentPrimitiveSplineCreating);
	DEFINE_CHILD_STATE(documentPrimitiveSpline, documentPrimitiveSplineStartReady);

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

