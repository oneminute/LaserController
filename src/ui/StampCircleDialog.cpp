#include "StampCircleDialog.h"
#include "ui_StampCircleDialog.h"
#include<QAbstractItemView>
#include<QListView>
#include<QStyledItemDelegate>
#include<QComboBox>
#include"scene/LaserPrimitive.h"

StampCircleDialog::StampCircleDialog(LaserScene* scene, QWidget* parent) 
   : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint),
    m_ui(new Ui::StampCircleDialog), m_scene(scene)
{
    m_viewer = qobject_cast<LaserViewer*> (scene->views()[0]);
    m_ui->setupUi(this);
    //LayoutComboBox
    QPixmap fourPm(":/ui/icons/images/frameFour.png");
    QPixmap threePm(":/ui/icons/images/frameThreeL.png");
    QPixmap threeRPm(":/ui/icons/images/frameThreeR.png");
    QPixmap threeLPm(":/ui/icons/images/frameTwo.png");
    m_ui->circleStampLayoutComboBox->addItem(QIcon(fourPm), tr("Four Character"));
    m_ui->circleStampLayoutComboBox->addItem(QIcon(threePm), tr("Three Character Left"));
    m_ui->circleStampLayoutComboBox->addItem(QIcon(threeRPm), tr("Three Character Right"));
    m_ui->circleStampLayoutComboBox->addItem(QIcon(threeLPm), tr("Two Character"));
    m_ui->circleStampLayoutComboBox->setItemDelegate(new QStyledItemDelegate());
    connect(m_ui->circleStampLayoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
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
    m_ui->circleCornerTypeComboBox->addItem(tr("Round Corner"));
    m_ui->circleCornerTypeComboBox->addItem(tr("Cutted Corner"));
    m_ui->circleCornerTypeComboBox->addItem(tr("Inner Corner"));
    connect(m_ui->circleCornerTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
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
    m_ui->circleWidthSpinBox->setValue(40);
    m_ui->circleWidthSpinBox->setMaximum(DBL_MAX);
    //frame height
    m_ui->circleHeightSpinBox->setValue(40);
    m_ui->circleHeightSpinBox->setMaximum(DBL_MAX);
    //frame border
    m_ui->circleBorderSpinBox->setValue(1);
    m_ui->circleBorderSpinBox->setMaximum(DBL_MAX);
    //frame CornerSize
    m_ui->circleCornerSizeSpinBox->setValue(2);
    m_ui->circleCornerSizeSpinBox->setMaximum(DBL_MAX);
    //frame CornerType
    m_ui->circleCornerTypeComboBox->setCurrentIndex(0);
    //inner frame
    m_ui->innerCircleGroupBox->setChecked(false);
    connect(m_ui->innerCircleGroupBox, QOverload<bool>::of(&QGroupBox::toggled), [=](bool checked) {
        m_ui->innerCircleLayout->setEnabled(checked);
    });
    //inner frame
    m_ui->InnerCircleMarginDoubleSpinBox->setValue(0.5);
    m_ui->InnerCircleMarginDoubleSpinBox->setMaximum(DBL_MAX);
    m_ui->InnerCircleBorderDoubleSpinBox->setValue(0.5);
    m_ui->InnerCircleBorderDoubleSpinBox->setMaximum(DBL_MAX);

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
    //fill
    connect(m_ui->fillBtn, QOverload<bool>::of(&QToolButton::toggled), [=](bool checked) {
        
        if (checked) {
            QPixmap icon(":/ui/icons/images/blank.png");
            m_ui->fillBtn->setIcon(QIcon(icon));
        }
        else {
            QPixmap icon(":/ui/icons/images/fill.png");
            m_ui->fillBtn->setIcon(QIcon(icon));
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
StampCircleDialog::~StampCircleDialog()
{
}

void StampCircleDialog::accept()
{
    
    QDialog::accept();
    QList<LaserPrimitive*> stampList;
    //create frame
    qreal circleW = m_ui->circleWidthSpinBox->value() * 1000;
    qreal circleH = m_ui->circleHeightSpinBox->value() * 1000;
    //qreal cornerRadius = m_ui->circleCornerSizeSpinBox->value() * 1000;
    qreal circleBorder = m_ui->circleBorderSpinBox->value() * 1000;
    //int circleType = m_ui->circleCornerTypeComboBox->currentIndex();
    QPointF point = m_viewer->mapToScene(m_viewer->rect().center());
    point = QPointF(point.x() - circleW * 0.5, point.y() - circleH * 0.5);
    QRect rect(point.x(), point.y(), circleW, circleH);
    LaserRing* circle = new LaserRing(m_scene->document(), rect, circleBorder, QTransform(), m_viewer->curLayerIndex());
    stampList.append(circle);
    LaserRing* innerCircle = nullptr;;
    QRect innerRect;
    qreal innerBorder;
    if (m_ui->innerCircleGroupBox->isChecked()) {
        qreal innderMargin = m_ui->InnerCircleMarginDoubleSpinBox->value() * 1000;
        innerBorder = m_ui->InnerCircleBorderDoubleSpinBox->value() * 1000;
        qreal innerW = circleW - innderMargin*2 - circleBorder*2;
        qreal innerH = circleH - innderMargin*2 - circleBorder*2;
        innerRect = QRect (point.x() + innderMargin + circleBorder, point.y() + innderMargin + circleBorder, innerW, innerH);
        innerCircle = new LaserRing(m_scene->document(), innerRect, innerBorder, QTransform(), m_viewer->curLayerIndex());
        stampList.append(innerCircle);
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
    if (innerCircle) {
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
        qreal vOffset = circleBorder + textVMargin;
        qreal hOffset = circleBorder + textHMargin;
        textBoundsTop = rect.top() + vOffset;
        textBoundsBottom = rect.bottom() - vOffset;
        textBoundsLeft = rect.left() + hOffset;
        textBoundsRight = rect.right() - hOffset;
        qreal width = (circleW - 2 * hOffset - textHSpace) * 0.5;
        qreal height = circleH - 2 * vOffset;
        signalSize = QSize(width, (height - textVSpace) * 0.5);
        doubleVerticalSize = QSize(width, height);
    }
    QString content = m_ui->lineEdit->text().trimmed();
    bool bold = m_ui->boldBtn->isChecked();
    bool itatic = m_ui->itaticBtn->isChecked();
    bool fill = !m_ui->fillBtn->isChecked();
    bool isRightToLeft = m_ui->isToLeftBtn->isChecked();
    QString family = m_ui->fontComboBox->currentText();
    bool containsDoubleText = false;
    
    int textLength = content.size();
    int layoutType = m_ui->circleStampLayoutComboBox->currentIndex();
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
            bold, itatic, false, fill, family, 0.0, QTransform(), m_viewer->curLayerIndex());
        stampList.append(text);
    }
    m_viewer->addPrimitiveAndExamRegionByBounds(stampList);
}
