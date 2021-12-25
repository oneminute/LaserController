#include "LaserControllerWindow.h"
#include "ui_LaserControllerWindow.h"
#include "widget/UndoCommand.h"

#include <DockAreaTabBar.h>
#include <DockAreaTitleBar.h>
#include <DockAreaWidget.h>
#include <DockComponentsFactory.h>
#include <DockContainerWidget.h>
#include <DockSplitter.h>
#include <DockWidgetTab.h>
#include <QFileDialog> 
#include <FloatingDockContainer.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QErrorMessage>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStack>
#include <QtMath>
#include <QRadioButton>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QUndoStack>
#include <QWindow>
#include <QFontComboBox>
#include <QSize>
#include <QUndoStack>
#include <QPainter>
#include <QtConcurrent>
#include <QDialogButtonBox>

#include "LaserApplication.h"
#include "algorithm/OptimizeNode.h"
#include "common/common.h"
#include "common/Config.h"
#include "import/Importer.h"
#include "laser/LaserDevice.h"
#include "scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "scene/LaserPrimitive.h"
#include "scene/LaserPrimitiveGroup.h"
#include "scene/LaserScene.h"
#include "scene/LaserPrimitive.h"
#include "state/StateController.h"
#include "task/ProgressModel.h"
#include "ui/ConfigDialog.h"
#include "ui/HalftoneDialog.h"
#include "ui/LaserLayerDialog.h"
#include "ui/MainCardInfoDialog.h"
#include "ui/PreviewWindow.h"
#include "ui/UpdateDialog.h"
#include "ui/ActivationDialog.h"
#include "ui/RegisteDialog.h"
#include "ui/UserInfoDialog.h"
#include "ui/MultiDuplicationDialog.h"
#include "util/ImageUtils.h"
#include "util/Utils.h"
#include "widget/FloatEditDualSlider.h"
#include "widget/FloatEditSlider.h"
#include "widget/LaserLayerTableWidget.h"
#include "widget/LaserViewer.h"
#include "widget/LayerButton.h"
#include "widget/RulerWidget.h"
#include "widget/ProgressBar.h"
#include "widget/Vector2DWidget.h"
#include "widget/PressedToolButton.h"
#include "widget/RadioButtonGroup.h"
#include "widget/PointPairTableWidget.h"
#include "widget/Label.h"

#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

void LaserControllerWindow::bindWidgetsProperties()
{
    // begin m_ui->actionOnlineHelp
    BIND_PROP_TO_STATE(m_ui->actionOnlineHelp, "enabled", true, initState);
    // end m_ui->actionOnlineHelp

    // begin m_ui->actionOfficialWebsite
    BIND_PROP_TO_STATE(m_ui->actionOfficialWebsite, "enabled", true, initState);
    // end m_ui->actionOfficialWebsite

    // begin m_ui->actionContactUs
    BIND_PROP_TO_STATE(m_ui->actionContactUs, "enabled", true, initState);
    // end m_ui->actionContactUs

    // begin m_ui->actionRemoteAssistance
    BIND_PROP_TO_STATE(m_ui->actionRemoteAssistance, "enabled", true, initState);
    // end m_ui->actionRemoteAssistance

    // begin m_ui->actionSettings
    BIND_PROP_TO_STATE(m_ui->actionSettings, "enabled", true, initState);
    // end m_ui->actionSettings

    // begin m_ui->actionAbout
    BIND_PROP_TO_STATE(m_ui->actionAbout, "enabled", true, initState);
    // end m_ui->actionAbout

    // begin m_ui->actionShowToolbar
    BIND_PROP_TO_STATE(m_ui->actionShowToolbar, "enabled", true, initState);
    // end m_ui->actionShowToolbar

    // begin m_ui->actionShowStatusBar
    BIND_PROP_TO_STATE(m_ui->actionShowStatusBar, "enabled", true, initState);
    // end m_ui->actionShowStatusBar

    // begin m_ui->actionShowLayersPanel
    BIND_PROP_TO_STATE(m_ui->actionShowLayersPanel, "enabled", true, initState);
    // end m_ui->actionShowLayersPanel

    // begin m_ui->actionShowPropertiesPanel
    BIND_PROP_TO_STATE(m_ui->actionShowPropertiesPanel, "enabled", true, initState);
    // end m_ui->actionShowPropertiesPanel

    // begin m_ui->actionShowOperationPanel
    BIND_PROP_TO_STATE(m_ui->actionShowOperationPanel, "enabled", true, initState);
    // end m_ui->actionShowOperationPanel

    // begin m_ui->actionShowRuler
    BIND_PROP_TO_STATE(m_ui->actionShowRuler, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionShowRuler, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionShowRuler, "enabled", true, documentIdleState);
    // end m_ui->actionShowRuler

    // begin m_ui->actionOpen
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionOpen, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionOpen

    // begin m_ui->actionCloseDocument
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCloseDocument, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionCloseDocument

    // begin m_ui->actionImport
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionImport, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionImport

    // begin m_ui->actionExportJSON
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionExportJSON, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionExportJSON

    // begin m_ui->actionSave
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSave, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionSave

    // begin m_ui->actionSaveAs
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSaveAs, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionSaveAs

    // begin m_ui->actionNew
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionNew, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionNew

    // begin m_ui->actionAddEngravingLayer
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionAddEngravingLayer, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionAddEngravingLayer

    // begin m_ui->actionAddCuttingLayer
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionAddCuttingLayer, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionAddCuttingLayer

    // begin m_ui->actionRemoveLayer
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionRemoveLayer, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionRemoveLayer

    // begin m_ui->actionAreaSelection
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionAreaSelection, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionAreaSelection

    // begin m_ui->actionCancelSelection
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCancelSelection, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionCancelSelection

    // begin m_ui->actionSelectionTool
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionSelectionTool

    // begin m_ui->actionRectangleTool
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionRectangleTool

    // begin m_ui->actionEllipseTool
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionEllipseTool

    // begin m_ui->actionPolygonTool
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionPolygonTool

    // begin m_ui->actionTextTool
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionTextTool

    // begin m_ui->actionLineTool
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionLineTool

    // begin m_ui->actionBitmapTool
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionBitmapTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionBitmapTool

    // begin m_ui->actionSplineTool
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionSplineTool

    // begin m_ui->actionEditSplineTool
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "enabled", true, documentPrimitiveSplineState);
    // end m_ui->actionEditSplineTool

    // begin m_ui->actionDragView
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "enabled", true, documentPrimitiveState);
    // end m_ui->actionDragView

    // begin m_ui->actionSelectionTool
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", true, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSelectionTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionSelectionTool

    // begin m_ui->actionRectangleTool
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionRectangleTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionRectangleTool

    // begin m_ui->actionEllipseTool
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionEllipseTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionEllipseTool

    // begin m_ui->actionPolygonTool
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionPolygonTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionPolygonTool

    // begin m_ui->actionTextTool
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionTextTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionTextTool

    // begin m_ui->actionLineTool
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionLineTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionLineTool

    // begin m_ui->actionSplineTool
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSplineTool, "checked", true, documentPrimitiveSplineState);
    // end m_ui->actionSplineTool

    // begin m_ui->actionEditSplineTool
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionEditSplineTool, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionEditSplineTool

    // begin m_ui->actionDragView
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionDragView, "checked", false, documentPrimitiveSplineState);
    // end m_ui->actionDragView

    // begin m_ui->actionImportCorelDraw
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionImportCorelDraw, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionImportCorelDraw

    // begin m_ui->actionLoadJson
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", true, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionLoadJson, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionLoadJson

    // begin m_ui->actionHalfTone
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionHalfTone, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionHalfTone

    // begin m_ui->actionSelectAll
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveSplineState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveRectReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitiveRectCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveEllipseReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitiveEllipseCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveLineReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitiveLineCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitivePolygonReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitivePolygonCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitivePolygonStartRectState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveTextReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitiveTextCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", true, documentPrimitiveSplineReadyState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitiveSplineCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionSelectAll, "enabled", false, documentPrimitiveSplineEditState);
    // end m_ui->actionSelectAll

    // begin m_ui->actionInvertSelection
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentSelectedEditingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentViewDragReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentViewDragingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveSplineState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveRectReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitiveRectCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveEllipseReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitiveEllipseCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveLineReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitiveLineCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitivePolygonReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitivePolygonCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitivePolygonStartRectState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveTextReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitiveTextCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", true, documentPrimitiveSplineReadyState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitiveSplineCreatingState);
    BIND_PROP_TO_STATE(m_ui->actionInvertSelection, "enabled", false, documentPrimitiveSplineEditState);
    // end m_ui->actionInvertSelection

    // begin m_ui->actionCut
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCut, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionCut

    // begin m_ui->actionCopy
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCopy, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionCopy

    // begin m_ui->actionPaste
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionPaste, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionPaste

    // begin m_ui->actionPasteInPlace
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionPasteInPlace, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionPasteInPlace

    // begin m_ui->actionPathOptimization
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionPathOptimization, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionPathOptimization

    // begin m_ui->actionDeletePrimitive
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionDeletePrimitive, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionDeletePrimitive

    // begin m_ui->actionMoveLayerUp
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerUp, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionMoveLayerUp

    // begin m_ui->actionMoveLayerDown
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLayerDown, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionMoveLayerDown

    // begin m_ui->actionAnalysisDocument
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionAnalysisDocument, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionAnalysisDocument

    // begin m_ui->actionLock
    // end m_ui->actionLock

    // begin m_ui->actionUnloc
    // end m_ui->actionUnloc

    // begin m_ui->actionUnitChange
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionUnitChange, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionUnitChange

    // begin m_ui->actionCameraUpdateOverlay
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCameraUpdateOverlay, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionCameraUpdateOverlay

    // begin m_ui->actionCameraTrace
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCameraTrace, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionCameraTrace

    // begin m_ui->actionCameraSaveSettings
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionCameraSaveSettings, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionCameraSaveSettings

    // begin m_ui->actionRefresh
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", true, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionRefresh, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionRefresh

    // begin m_ui->actionZoomIn
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionZoomIn, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionZoomIn

    // begin m_ui->actionZoomOut
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionZoomOut, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionZoomOut

    // begin m_ui->actionZoomToPage
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToPage, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionZoomToPage

    // begin m_ui->actionZoomToSelection
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionZoomToSelection, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionZoomToSelection

    // begin m_ui->actionUndo
    BIND_PROP_TO_STATE(m_ui->actionUndo, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUndo, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionUndo, "enabled", true, documentIdleState);
    // end m_ui->actionUndo

    // begin m_ui->actionRedo
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionRedo, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionRedo

    // begin m_ui->actionMirrorHorizontal
    BIND_PROP_TO_STATE(m_ui->actionMirrorHorizontal, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorHorizontal, "enabled", false, documentEmptyState);
    // end m_ui->actionMirrorHorizontal

    // begin m_ui->actionMirrorVertical
    BIND_PROP_TO_STATE(m_ui->actionMirrorVertical, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorVertical, "enabled", false, documentEmptyState);
    // end m_ui->actionMirrorVertical

    // begin m_ui->actionPrimitiveWeld
    BIND_PROP_TO_STATE(m_ui->actionPrimitiveWeld, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrimitiveWeld, "enabled", false, documentEmptyState);
    // end m_ui->actionPrimitiveWeld

    // begin m_ui->actionGroup
    BIND_PROP_TO_STATE(m_ui->actionGroup, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionGroup, "enabled", false, documentEmptyState);
    // end m_ui->actionGroup

    // begin m_ui->actionUngroup
    BIND_PROP_TO_STATE(m_ui->actionUngroup, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUngroup, "enabled", false, documentEmptyState);
    // end m_ui->actionUngroup

    // begin m_ui->actionDuplication
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionDuplication, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionDuplication

    // begin m_ui->actionMultiDuplication
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentViewDragState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionMultiDuplication, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionMultiDuplication

    // begin m_ui->actionMirrorAcrossLine
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentEmptyState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitiveState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitiveRectState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitiveEllipseState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitiveLineState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitivePolygonState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitiveTextState);
    BIND_PROP_TO_STATE(m_ui->actionMirrorAcrossLine, "enabled", false, documentPrimitiveSplineState);
    // end m_ui->actionMirrorAcrossLine

    // begin m_ui->actionLaserSpotShot
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionLaserSpotShot, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionLaserSpotShot

    // begin m_ui->actionMainboardInformation
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMainboardInformation, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMainboardInformation

    // begin m_ui->actionUpdateFirmware
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateFirmware, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionUpdateFirmware

    // begin m_ui->actionDownload
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionDownload, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionDownload

    // begin m_ui->actionWorkState
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionWorkState, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionWorkState

    // begin m_ui->actionBounding
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionBounding, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionBounding

    // begin m_ui->actionLaserMove
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionLaserMove, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionLaserMove

    // begin m_ui->actionMoveToOrigin
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToOrigin, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveToOrigin

    // begin m_ui->actionMoveLeft
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveLeft, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveLeft

    // begin m_ui->actionMoveRight
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveRight, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveRight

    // begin m_ui->actionMoveTop
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTop, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveTop

    // begin m_ui->actionMoveBottom
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottom, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveBottom

    // begin m_ui->actionMoveTopLeft
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopLeft, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveTopLeft

    // begin m_ui->actionMoveTopRight
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveTopRight, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveTopRight

    // begin m_ui->actionMoveBottomLeft
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomLeft, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveBottomLeft

    // begin m_ui->actionMoveBottomRight
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveBottomRight, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveBottomRight

    // begin m_ui->actionMoveUp
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveUp, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveUp

    // begin m_ui->actionMoveDown
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveDown, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveDown

    // begin m_ui->actionMachining
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMachining, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMachining

    // begin m_ui->actionPause
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPause, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionPause

    // begin m_ui->actionStop
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionStop, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionStop

    // begin m_ui->actionConnect
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionConnect, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionConnect

    // begin m_ui->actionDisconnect
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionDisconnect, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionDisconnect

    // begin m_ui->actionLoadMotor
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionLoadMotor, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionLoadMotor

    // begin m_ui->actionUnloadMotor
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionUnloadMotor, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionUnloadMotor

    // begin m_ui->actionMainCardInfo
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMainCardInfo, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMainCardInfo

    // begin m_ui->actionTemporaryLicense
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionTemporaryLicense, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionTemporaryLicense

    // begin m_ui->actionUpdateOutline
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateOutline, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionUpdateOutline

    // begin m_ui->actionReset
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionReset, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionReset

    // begin m_ui->actionShowLaserPosition
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionShowLaserPosition, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionShowLaserPosition

    // begin m_ui->actionHideLaserPosition
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionHideLaserPosition, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionHideLaserPosition

    // begin m_ui->actionMoveToUserOrigin
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionMoveToUserOrigin, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionMoveToUserOrigin

    // begin m_ui->actionFetchToUserOrigin
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionFetchToUserOrigin, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionFetchToUserOrigin

    // begin m_ui->actionRegiste
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionRegiste, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionRegiste

    // begin m_ui->actionActivate
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionActivate, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionActivate

    // begin m_ui->actionUserInfo
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionUserInfo, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionUserInfo

    // begin m_ui->actionApplyJobOriginToDocument
    BIND_PROP_TO_STATE(m_ui->actionApplyJobOriginToDocument, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionApplyJobOriginToDocument, "enabled", true, documentWorkingState);
    BIND_PROP_TO_STATE(m_ui->actionApplyJobOriginToDocument, "enabled", true, documentIdleState);
    BIND_PROP_TO_STATE(m_ui->actionApplyJobOriginToDocument, "enabled", true, documentSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionApplyJobOriginToDocument, "enabled", true, documentSelectedState);
    BIND_PROP_TO_STATE(m_ui->actionApplyJobOriginToDocument, "enabled", true, documentSelectedEditingState);
    // end m_ui->actionApplyJobOriginToDocument

    // begin m_ui->actionUpdateSoftware
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionUpdateSoftware, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionUpdateSoftware

    // begin m_ui->actionPrintAndCutNew
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutNew, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutNew

    // begin m_ui->actionPrintAndCutFetchLaser
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutFetchLaser, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutFetchLaser

    // begin m_ui->actionPrintAndCutClear
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutClear, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutClear

    // begin m_ui->actionPrintAndCutAlign
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutAlign, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutAlign

    // begin m_ui->actionPrintAndCutRestore
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", false, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRestore, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutRestore

    // begin m_ui->actionPrintAndCutRemove
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutRemove, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutRemove

    // begin m_ui->actionStartRedLightAlight
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionStartRedLightAlight, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionStartRedLightAlight

    // begin m_ui->actionFinishRedLightAlight
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, deviceConnectedState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", false, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionFinishRedLightAlight, "enabled", true, documentPrintAndCutAligningState);
    // end m_ui->actionFinishRedLightAlight

    // begin m_ui->actionPrintAndCutSelectPoint
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutSelectPoint, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutSelectPoint

    // begin m_ui->actionPrintAndCutEndSelect
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, deviceIdleState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, deviceMachiningState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, devicePausedState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, deviceErrorState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_ui->actionPrintAndCutEndSelect, "enabled", false, documentPrintAndCutAligningState);
    // end m_ui->actionPrintAndCutEndSelect

    // begin Config::UserRegister::scanLaserPowerItem()
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", false, initState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(Config::UserRegister::scanLaserPowerItem(), "enabled", true, documentPrintAndCutAligningState);
    // end Config::UserRegister::scanLaserPowerItem()

    // begin Config::UserRegister::maxScanGrayRatioItem()
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", false, initState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(Config::UserRegister::maxScanGrayRatioItem(), "enabled", true, documentPrintAndCutAligningState);
    // end Config::UserRegister::maxScanGrayRatioItem()

    // begin Config::UserRegister::minScanGrayRatioItem()
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", false, initState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(Config::UserRegister::minScanGrayRatioItem(), "enabled", true, documentPrintAndCutAligningState);
    // end Config::UserRegister::minScanGrayRatioItem()

    // begin Config::UserRegister::defaultMaxCuttingPowerItem()
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", false, initState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMaxCuttingPowerItem(), "enabled", true, documentPrintAndCutAligningState);
    // end Config::UserRegister::defaultMaxCuttingPowerItem()

    // begin Config::UserRegister::defaultMinCuttingPowerItem()
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", false, initState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(Config::UserRegister::defaultMinCuttingPowerItem(), "enabled", true, documentPrintAndCutAligningState);
    // end Config::UserRegister::defaultMinCuttingPowerItem()

    // begin Config::UserRegister::spotShotPowerItem()
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", false, initState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(Config::UserRegister::spotShotPowerItem(), "enabled", true, documentPrintAndCutAligningState);
    // end Config::UserRegister::spotShotPowerItem()

    // begin m_doubleSpinBoxDistanceX
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceX, "enabled", true, documentPrintAndCutAligningState);
    // end m_doubleSpinBoxDistanceX

    // begin m_doubleSpinBoxDistanceY
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceY, "enabled", true, documentPrintAndCutAligningState);
    // end m_doubleSpinBoxDistanceY

    // begin m_doubleSpinBoxDistanceZ
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", false, initState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", false, deviceUnconnectedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, deviceConnectedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, deviceIdleState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, deviceMachiningState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, devicePausedState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, deviceErrorState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, documentPrintAndCutSelectingState);
    BIND_PROP_TO_STATE(m_doubleSpinBoxDistanceZ, "enabled", true, documentPrintAndCutAligningState);
    // end m_doubleSpinBoxDistanceZ


}
