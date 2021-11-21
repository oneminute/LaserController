#include "MultiDuplicationDialog.h"
#include "ui_MultiDuplicationDialog.h"
#include "LaserApplication.h"
#include "common/common.h"
#include "widget/LaserViewer.h"
#include "scene/LaserScene.h"
#include "scene/LaserPrimitiveGroup.h"
#include "widget/UndoCommand.h"
#include <QSpinBox>

MultiDuplicationDialog::MultiDuplicationDialog(
    LaserViewer* view, 
    int copies, int hSettingsVal, int vSettingsVal, int hDirectionVal, int vDirectionVal,
    qreal hDistanceLastVal, qreal vDistanceLastVal,
    QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::MultiDuplicationDialog)
    , m_viewer(view)
    , m_copiesVal(copies)
    , m_VDirectionVal(vDirectionVal)
    , m_HDirectionVal(hDirectionVal)
    , m_VSettingsVal(vSettingsVal)
    , m_HSettingsVal(hSettingsVal)
    , m_HDistanceVal(hDistanceLastVal)
    , m_VDistanceVal(vDistanceLastVal)
{
    m_group = m_viewer->group();
    
    m_ui->setupUi(this);
    m_ui->HSettings->addItem("No Offset");//0
    m_ui->HSettings->addItem("Offset");//1
    m_ui->HSettings->addItem("Margins");//2
    m_ui->HSettings->setCurrentIndex(1);
    m_ui->VSettings->addItem("No Offset");//0
    m_ui->VSettings->addItem("Offset");//1
    m_ui->VSettings->addItem("Margins");//2
    m_ui->VSettings->setCurrentIndex(1);
    m_ui->HDirection->addItem("Left");//0
    m_ui->HDirection->addItem("Right");//1
    m_ui->HDirection->setEnabled(false);
    m_ui->VDirection->addItem("Top");//0
    m_ui->VDirection->addItem("Bottom");//1
    m_ui->VDirection->setEnabled(false);
    //m_ui->HSettings->currentIndex();
    
    connect(m_ui->HSettings, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MultiDuplicationDialog::HorizontalComboxChanged);
    connect(m_ui->VSettings, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MultiDuplicationDialog::VerticalComboxChanged);
    //setting value
    m_ui->HSettings->setCurrentIndex(m_HSettingsVal);
    m_ui->VSettings->setCurrentIndex(m_VSettingsVal);
    m_ui->HDirection->setCurrentIndex(m_HDirectionVal);
    m_ui->VDirection->setCurrentIndex(m_VDirectionVal);
    m_ui->Copies->setValue(m_copiesVal);
    m_ui->HDistance->setValue(m_HDistanceVal);
    m_ui->VDistance->setValue(m_VDistanceVal);
}

MultiDuplicationDialog::~MultiDuplicationDialog()
{
}

void MultiDuplicationDialog::HorizontalComboxChanged(int index)
{
    switch (index) {
        case 0: {
            m_ui->HDirection->setEnabled(false);
            m_ui->HDistance->setEnabled(false);
            break;
        }
        
        case 1: {
            m_ui->HDirection->setEnabled(false);
            m_ui->HDistance->setEnabled(true);
            break;
        }
        
        case 2: {
            m_ui->HDirection->setEnabled(true);
            m_ui->HDistance->setEnabled(true);
            break;
        }
        
    }
}

void MultiDuplicationDialog::VerticalComboxChanged(int index)
{
    switch (index) {
    case 0: {
        m_ui->VDirection->setEnabled(false);
        m_ui->VDistance->setEnabled(false);
        break;
    }

    case 1: {
        m_ui->VDirection->setEnabled(false);
        m_ui->VDistance->setEnabled(true);
        break;
    }

    case 2: {
        m_ui->VDirection->setEnabled(true);
        m_ui->VDistance->setEnabled(true);
        break;
    }

    }
}

void MultiDuplicationDialog::accept()
{
    QDialog::accept();
    int copies = m_ui->Copies->value();
    if (copies <= 0) {
        return;
    }
    qreal v = m_ui->HDistance->value();
    qreal translateX = Global::mmToSceneHF(m_ui->HDistance->value());
    qreal translateY = Global::mmToSceneVF(m_ui->VDistance->value());
    
    switch (m_ui->HSettings->currentIndex())
    {
        case 0: {
            translateX = 0;
            break;
        }
        case 1: {

            translateX = translateX;
            break;
        }
        case 2: {
            qreal groupWidth = m_viewer->selectedItemsSceneBoundingRect().width();
            translateX = translateX + groupWidth;
            break;
        }
    }
    switch (m_ui->VSettings->currentIndex())
    {
        case 0: {
            translateY = 0;
            break;
        }
        case 1: {
            translateY = translateY;
            break;
        }
        case 2: {
            qreal groupHeight = m_viewer->selectedItemsSceneBoundingRect().height();
            translateY = translateY + groupHeight;
            break;
        }
    }
    if (m_ui->HDirection->isEnabled()) {
        switch (m_ui->HDirection->currentIndex()) {
            case 0:{
                translateX = -translateX;
                break;
            }
            case 1: {
                translateX = translateX;
                break;
            }
        }
    }
    if (m_ui->VDirection->isEnabled()) {
        switch (m_ui->VDirection->currentIndex()) {
            case 0: {
                translateY = -translateY;
                break;
            }
            case 1: {
                translateY = translateY;
                break;
            }
        }
    }
    //transform
    QList<QGraphicsItem*> lastGroup = m_group->childItems();
    //m_viewer->resetGroup();
    QList<LaserPrimitive*> newList;
    for (QGraphicsItem* item : lastGroup) {
        LaserPrimitive* primitive = qgraphicsitem_cast<LaserPrimitive*>(item);
        for (int i = 0; i < copies; i++) {
            int count = i + 1;
            QTransform t = primitive->sceneTransform();
            QTransform t1;
            t1 = t1.translate(count * translateX, count * translateY);      
            LaserPrimitive* newPrimitive = primitive->clone(t * t1);
            if (!m_viewer->detectBoundsInMaxRegion(newPrimitive->sceneBoundingRect())) {
                return;
            }
            newList.append(newPrimitive);
        }
    }
    AddDelUndoCommand* cmd = new AddDelUndoCommand(m_viewer->scene(), newList);
    m_viewer->undoStack()->push(cmd);
}

int MultiDuplicationDialog::HSettingsVal()
{
    //return m_HSettingsVal;
    return m_ui->HSettings->currentIndex();
}

int MultiDuplicationDialog::VSettingsVal()
{
    //return m_VSettingsVal;
    return m_ui->VSettings->currentIndex();
}

int MultiDuplicationDialog::HDirectionVal()
{
    //return m_HDirectionVal;
    return m_ui->HDirection->currentIndex();
}

int MultiDuplicationDialog::VDirectionVal()
{
    //return m_VDirectionVal;
    return m_ui->VDirection->currentIndex();
}

double MultiDuplicationDialog::HDistanceVal()
{
    //return m_HDistanceVal;
    return m_ui->HDistance->value();
}

double MultiDuplicationDialog::VDistanceVal()
{
    //return m_VDistanceVal;
    return m_ui->VDistance->value();
}

int MultiDuplicationDialog::copiesVal()
{
    //return m_copiesVal;
    return m_ui->Copies->value();
}
