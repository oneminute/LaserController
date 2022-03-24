#include "StampFrameDialog.h"
#include "ui_StampFrameDialog.h"
#include<QAbstractItemView>
#include<QListView>
#include<QStyledItemDelegate>
#include<QComboBox>
#include"scene/LaserPrimitive.h"
#include "scene/LaserDocument.h"
#include"scene/LaserDocument.h"
#include "scene/LaserLayer.h"

StampFrameDialog::StampFrameDialog(LaserScene* scene, QWidget* parent) 
   : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint),
    m_ui(new Ui::StampFrameDialog), m_scene(scene)
{
    m_viewer = qobject_cast<LaserViewer*> (scene->views()[0]);
    m_ui->setupUi(this);
    m_layerIndex = m_scene->document()->idleLayer()->index();
    //LayoutComboBox
    QPixmap fourPm(":/ui/icons/images/frameFour.png");
    QPixmap threePm(":/ui/icons/images/frameThreeL.png");
    QPixmap threeRPm(":/ui/icons/images/frameThreeR.png");
    QPixmap threeLPm(":/ui/icons/images/frameTwo.png");
    m_ui->frameStampLayoutComboBox->addItem(QIcon(fourPm), tr("Four Character"));
    m_ui->frameStampLayoutComboBox->addItem(QIcon(threePm), tr("Three Character Left"));
    m_ui->frameStampLayoutComboBox->addItem(QIcon(threeRPm), tr("Three Character Right"));
    m_ui->frameStampLayoutComboBox->addItem(QIcon(threeLPm), tr("Two Character"));
    m_ui->frameStampLayoutComboBox->setItemDelegate(new QStyledItemDelegate());
    connect(m_ui->frameStampLayoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        switch (index) {
        case 0: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(fourPm);
            m_ui->lineEdit->setMaxLength(4);
            break;
        }
        case 1: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(threePm);
            m_ui->lineEdit->setMaxLength(3);
            break;
        }
        case 2: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(threeRPm);
            m_ui->lineEdit->setMaxLength(3);
            break;
        }
        case 3: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(threeLPm);
            m_ui->lineEdit->setMaxLength(2);
            break;
        }
        }
    });
    //corner type
    m_ui->frameCornerTypeComboBox->addItem(tr("Round Corner"));
    m_ui->frameCornerTypeComboBox->addItem(tr("Cutted Corner"));
    m_ui->frameCornerTypeComboBox->addItem(tr("Inner Corner"));
    connect(m_ui->frameCornerTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        switch (index) {
        case 0: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(fourPm);
            break;
        }
        case 1: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(threePm);
            break;
        }
        case 2: {
            //m_ui->frameStampLayoutLabelIcon->setPixmap(threePm);
            break;
        }
        
        }
    });
    //frame width
    m_ui->frameWidthSpinBox->setValue(20);
    m_ui->frameWidthSpinBox->setMaximum(DBL_MAX);
    //frame height
    m_ui->frameHeightSpinBox->setValue(20);
    m_ui->frameHeightSpinBox->setMaximum(DBL_MAX);
    //frame border
    m_ui->frameBorderSpinBox->setValue(1);
    m_ui->frameBorderSpinBox->setMaximum(DBL_MAX);
    //frame CornerSize
    m_ui->frameCornerSizeSpinBox->setValue(2);
    m_ui->frameCornerSizeSpinBox->setMaximum(DBL_MAX);
    //frame CornerType
    m_ui->frameCornerTypeComboBox->setCurrentIndex(0);
    //inner frame
    m_ui->innerFrameGroupBox->setChecked(false);
    connect(m_ui->innerFrameGroupBox, QOverload<bool>::of(&QGroupBox::toggled), [=](bool checked) {
        m_ui->innerFrameLayout->setEnabled(checked);
    });
    //inner frame
    m_ui->InnerFrameMarginDoubleSpinBox->setValue(0.5);
    m_ui->InnerFrameMarginDoubleSpinBox->setMaximum(DBL_MAX);
    m_ui->InnerFrameBorderDoubleSpinBox->setValue(0.5);
    m_ui->InnerFrameBorderDoubleSpinBox->setMaximum(DBL_MAX);
    //text
    m_ui->hMarginSpinBox->setValue(0.5);
    m_ui->hMarginSpinBox->setMinimum(-DBL_MAX);
    m_ui->hMarginSpinBox->setMaximum(DBL_MAX);
    m_ui->vMarginSpinBox->setValue(0.0);
    m_ui->vMarginSpinBox->setMinimum(-DBL_MAX);
    m_ui->vMarginSpinBox->setMaximum(DBL_MAX);
    m_ui->hSpaceSpinBox->setValue(0.5);
    m_ui->hSpaceSpinBox->setMinimum(-DBL_MAX);
    m_ui->hSpaceSpinBox->setMaximum(DBL_MAX);
    m_ui->vSpaceSpinBox->setValue(0.5);
    m_ui->vSpaceSpinBox->setMinimum(-DBL_MAX);
    m_ui->vSpaceSpinBox->setMaximum(DBL_MAX);

    //text content
    connect(m_ui->lineEdit, QOverload<const QString&>::of(&QLineEdit::textChanged), [=](const QString& text) {
        bool b = text.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
        if (b) {
            m_ui->fontComboBox->setWritingSystem(QFontDatabase::SimplifiedChinese);
        }
        else {
            m_ui->fontComboBox->setWritingSystem(QFontDatabase::Any);
        }
    });
    //isToRightButton
    connect(m_ui->isToLeftBtn, QOverload<bool>::of(&QToolButton::toggled), [=](bool checked) {
        QPixmap icon(":/ui/icons/images/toRightArrow.png");
        if (checked) {
            
            QTransform rotate;
            rotate.rotate(180);
            QPixmap iconR = icon.transformed(rotate);
            m_ui->isToLeftBtn->setIcon(QIcon(iconR));
        }
        else {
            m_ui->isToLeftBtn->setIcon(QIcon(icon));
        }
    });
    //itatic
    connect(m_ui->itaticBtn, QOverload<bool>::of(&QToolButton::toggled), [=](bool checked) {
        
        if (checked) {
            QPixmap icon(":/ui/icons/images/itatic.png");
            m_ui->itaticBtn->setIcon(QIcon(icon));
        }
        else {
            QPixmap icon(":/ui/icons/images/straight.png");
            m_ui->itaticBtn->setIcon(QIcon(icon));
        }
    });
    //bold
    connect(m_ui->boldBtn, QOverload<bool>::of(&QToolButton::toggled), [=](bool checked) {
        
    if (checked) {
        QPixmap icon(":/ui/icons/images/bold1.png");
        m_ui->boldBtn->setIcon(QIcon(icon));
    }
    else {
        QPixmap icon(":/ui/icons/images/bold.png");
        m_ui->boldBtn->setIcon(QIcon(icon));
    }
    });
    //text margins
    m_ui->hMarginSpinBox->setMaximum(DBL_MAX);
    m_ui->vMarginSpinBox->setMaximum(DBL_MAX);
    //text space
    m_ui->hSpaceSpinBox->setMaximum(DBL_MAX);
    m_ui->vSpaceSpinBox->setMaximum(DBL_MAX);
}
StampFrameDialog::~StampFrameDialog()
{
}

void StampFrameDialog::accept()
{
    
    QDialog::accept();
    bool stampIntaglio = m_ui->stampIntaglioCheckBox->isChecked();
    QList<LaserPrimitive*> stampList;
    //create frame
    qreal frameW = m_ui->frameWidthSpinBox->value() * 1000;
    qreal frameH = m_ui->frameHeightSpinBox->value() * 1000;
    qreal cornerRadius = m_ui->frameCornerSizeSpinBox->value() * 1000;
    qreal frameBorder = m_ui->frameBorderSpinBox->value() * 1000;
    int frameType = m_ui->frameCornerTypeComboBox->currentIndex();
    QPointF point = m_viewer->mapToScene(m_viewer->rect().center());
    point = QPointF(point.x() - frameW * 0.5, point.y() - frameH * 0.5);
    QRect rect(point.x(), point.y(), frameW, frameH);
    bool needAuxiliary = true;
    LaserFrame* frame = new LaserFrame(m_scene->document(), rect, frameBorder, cornerRadius, stampIntaglio, QTransform(), m_layerIndex, frameType);
    frame->setNeedAuxiliaryLine(needAuxiliary);
    stampList.append(frame);
    LaserFrame* innerFrame = nullptr;;
    QRect innerRect;
    qreal innerBorder;
    if (m_ui->innerFrameGroupBox->isChecked()) {
        qreal innderMargin = m_ui->InnerFrameMarginDoubleSpinBox->value() * 1000;
        innerBorder = m_ui->InnerFrameBorderDoubleSpinBox->value() * 1000;
        qreal innerW = frameW - innderMargin*2 - frameBorder*2;
        qreal innerH = frameH - innderMargin*2 - frameBorder*2;
        innerRect = QRect (point.x() + innderMargin + frameBorder, point.y() + innderMargin + frameBorder, innerW, innerH);
        innerFrame = new LaserFrame(m_scene->document(), innerRect, innerBorder, cornerRadius, stampIntaglio, QTransform(), m_layerIndex, frameType);
        innerFrame->setNeedAuxiliaryLine(needAuxiliary);
        innerFrame->setInner(true);
        stampList.append(innerFrame);
    }
    
    //create text
    //int layoutIndex = m_ui->frameStampLayoutComboBox->currentIndex();
    QSize signalSize;
    QSize doubleVerticalSize;
    qreal textHMargin = m_ui->hMarginSpinBox->value() * 1000;
    qreal textVMargin = m_ui->vMarginSpinBox->value() * 1000;
    qreal textHSpace = m_ui->hSpaceSpinBox->value() * 1000;
    qreal textVSpace = m_ui->vSpaceSpinBox->value() * 1000;
    qreal textBoundsTop, textBoundsBottom, textBoundsLeft, textBoundsRight;
    if (innerFrame) {
        qreal vOffset = innerBorder + textVMargin;
        qreal hOffset = innerBorder + textHMargin;
        textBoundsTop = innerRect.top() + vOffset;
        textBoundsBottom = innerRect.bottom() - vOffset;
        textBoundsLeft = innerRect.left() + hOffset;
        textBoundsRight = innerRect.right() - hOffset;
        qreal width = (innerRect.width() - 2 * hOffset - textHSpace) * 0.5;
        qreal height = innerRect.height() - 2 * vOffset;
        signalSize = QSize(width,(height - textVSpace) * 0.5);
        doubleVerticalSize = QSize(width, height);
    }
    else {
        qreal vOffset = frameBorder + textVMargin;
        qreal hOffset = frameBorder + textHMargin;
        textBoundsTop = rect.top() + vOffset;
        textBoundsBottom = rect.bottom() - vOffset;
        textBoundsLeft = rect.left() + hOffset;
        textBoundsRight = rect.right() - hOffset;
        qreal width = (frameW - 2 * hOffset - textHSpace) * 0.5;
        qreal height = frameH - 2 * vOffset;
        signalSize = QSize(width, (height - textVSpace) * 0.5);
        doubleVerticalSize = QSize(width, height);
    }
    QString content = m_ui->lineEdit->text().trimmed();
    bool bold = m_ui->boldBtn->isChecked();
    bool itatic = m_ui->itaticBtn->isChecked();
    
    bool isRightToLeft = m_ui->isToLeftBtn->isChecked();
    QString family = m_ui->fontComboBox->currentText();
    bool containsDoubleText = false;
    
    int textLength = content.size();
    int layoutType = m_ui->frameStampLayoutComboBox->currentIndex();
    QSize textSize;
    for (int i = 0; i < textLength; i++) {
        QString signalText(content[i]);
        QPointF center;
        if (layoutType == 0) {
            textSize = signalSize;
            switch (i) {
            case 0: {
                
                if (isRightToLeft) {
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            case 1: {
                if (isRightToLeft) {
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                else {
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                break;
            }
            case 2: {
                if (isRightToLeft) {
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            case 3: {
                
                if (isRightToLeft) {
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                else {
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                break;
            }
            }
        }
        else if (layoutType == 1) {
            switch (i) {
            case 0: {
                
                if (isRightToLeft) {
                    textSize = signalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    textSize = doubleVerticalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            case 1: {
                
                if (isRightToLeft) {
                    textSize = signalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                else {
                    textSize = signalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            case 2: {
                if (isRightToLeft) {
                    textSize = doubleVerticalSize;
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                    break;
                }
                else {
                    textSize = signalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                    break;
                }
                
            }
            }
        }
        else if (layoutType == 2) {
            switch (i) {
            case 0: {
                if (isRightToLeft) {
                    textSize = doubleVerticalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    textSize = signalSize;
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            case 1: {
                if (isRightToLeft) {
                    textSize = signalSize;
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    textSize = signalSize;
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                
                break;
            }
            case 2: {
                
                if (isRightToLeft) {
                    textSize = signalSize;
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsBottom - textSize.height() * 0.5);
                }
                else {
                    textSize = doubleVerticalSize;
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            }
        }
        else if (layoutType == 3) {
            switch (i) {
            case 0: {
                textSize = doubleVerticalSize;
                if (isRightToLeft) {
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            case 1: {
                textSize = doubleVerticalSize;
                
                if (isRightToLeft) {
                    center = QPointF(textBoundsLeft + textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                else {
                    center = QPointF(textBoundsRight - textSize.width() * 0.5, textBoundsTop + textSize.height() * 0.5);
                }
                break;
            }
            }
        }
        LaserHorizontalText* text = new LaserHorizontalText(m_scene->document(), signalText, textSize, center,
            bold, itatic, false, stampIntaglio, family, 0.0, QTransform(), m_layerIndex);
        stampList.append(text);
    }
    m_viewer->addPrimitiveAndExamRegionByBounds(stampList, frame);
}
