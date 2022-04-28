#include "StampCircleDialog.h"
#include "ui_StampCircleDialog.h"
#include<QAbstractItemView>
#include<QListView>
#include<QStyledItemDelegate>
#include<QComboBox>
#include<QHeaderView>
#include<QCheckBox>
#include<QPushButton>
#include "ui/StampDialog.h"
#include "scene/LaserPrimitiveGroup.h"
#include"scene/LaserPrimitive.h"
#include"scene/LaserDocument.h"
#include "scene/LaserLayer.h"
#include "util/Utils.h"
#include <QFileDialog>

StampCircleDialog::StampCircleDialog(LaserScene* scene,bool isEllipse, QWidget* parent) 
   : StampDialog(scene, parent),
    m_ui(new Ui::StampCircleDialog), m_isEllipse(isEllipse)
{
    m_viewer = qobject_cast<LaserViewer*> (scene->views()[0]);
    m_ui->setupUi(this);
    LaserLayer* layer = m_scene->document()->idleLayer();
    layer->setType(LLT_STAMP);
    m_layerIndex = layer->index();
    m_preview = m_ui->graphicsView;

    m_ui->fontComboBox->setCurrentText(tr("FangSong"));
    //text size
    m_ui->textSizeSpinBox->setValue(6);
    m_ui->textSizeSpinBox->setMinimum(0);
    m_ui->textSizeSpinBox->setMaximum(DBL_MAX);
    //text margins
    m_ui->marginSpinBox->setMinimum(-DBL_MAX);
    m_ui->marginSpinBox->setMaximum(DBL_MAX);
    //
    //text space
    m_ui->hSpaceSpinBox->setMinimum(-DBL_MAX);
    m_ui->hSpaceSpinBox->setMaximum(DBL_MAX);
    m_ui->hSpaceSpinBox->setValue(0);
    //LayoutComboBox
    if (m_isEllipse) {
        m_ui->circleHeightLabel->setVisible(true);
        m_ui->circleHeightSpinBox->setVisible(true);
        m_ui->lineGroupBox->setVisible(false);
        m_ui->AdditionGroupBox->setCheckable(true);
        m_ui->AdditionGroupBox->setChecked(false);
        m_ui->circleDiameterSpinBox->setValue(50);
        m_ui->circleHeightSpinBox->setValue(35);
        m_ui->embleSizeSpinBox->setValue(6);
        m_ui->marginSpinBox->setValue(1);
        m_ui->hSpaceSpinBox->setValue(0.5);
        this->setWindowTitle(tr("Ellipse Stamp Dialog"));
        //angle
        m_ui->angleSpinBox->setValue(210);
        QPixmap normal(":/ui/icons/images/normalEliipseStamp.png");
        QPixmap horizontalText(":/ui/icons/images/horizontalTextEllipseStamp.png");
        QPixmap emblemEllipse(":/ui/icons/images/embleEllipseStamp.png");
        QPixmap newA(":/ui/icons/images/newInvoiceA.png");
        QPixmap newB(":/ui/icons/images/newInvoiceB.png");
        m_ui->circleStampLayoutComboBox->addItem(QIcon(normal), tr("Normal Ellipse Stamp"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(horizontalText), tr("Horizontal Text Ellipse Stamp"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(emblemEllipse), tr("Emblem Ellipse Stamp"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(newA), tr("New Invoice Stamp A"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(newB), tr("New Invoice Stamp B"));
        m_ui->circleStampLayoutComboBox->setItemDelegate(new QStyledItemDelegate());
    }
    else {
        m_ui->circleHeightLabel->setVisible(false);
        m_ui->circleHeightSpinBox->setVisible(false);
        m_ui->circleDiameterSpinBox->setValue(40);
        m_ui->embleSizeSpinBox->setValue(13);
        m_ui->marginSpinBox->setValue(1);
        this->setWindowTitle(tr("Circle Stamp Dialog"));
        //angle
        m_ui->angleSpinBox->setValue(225);
        QPixmap normal(":/ui/icons/images/normalCircleStamp.png");
        QPixmap horizontalText(":/ui/icons/images/HTextCircleStamp.png");
        QPixmap bottomText(":/ui/icons/images/bottomTextCircleStamp.png");
        QPixmap CCYL(":/ui/icons/images/CCYLStamp.png");
        m_ui->circleStampLayoutComboBox->addItem(QIcon(normal), tr("Normal Circle Stamp"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(horizontalText), tr("Horizontal Text Circle Stamp"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(bottomText), tr("Bottom Text Circle Stamp"));
        m_ui->circleStampLayoutComboBox->addItem(QIcon(CCYL), tr("CCYL Circle Stamp"));
        m_ui->circleStampLayoutComboBox->setItemDelegate(new QStyledItemDelegate());
    }
    
    
    //diameter
    m_ui->circleDiameterSpinBox->setMaximum(DBL_MAX);
    
    //border
    m_ui->circleBorderSpinBox->setValue(1);
    m_ui->circleBorderSpinBox->setMaximum(DBL_MAX);    
    
    //inner circle
    m_ui->innerCircleGroupBox->setChecked(false);
    connect(m_ui->innerCircleGroupBox, QOverload<bool>::of(&QGroupBox::toggled), [=](bool checked) {
        m_ui->innerCircleLayout->setEnabled(checked);
    });
    //inner frame
    m_ui->InnerCircleMarginDoubleSpinBox->setValue(0.5);
    m_ui->InnerCircleMarginDoubleSpinBox->setMaximum(DBL_MAX);
    m_ui->InnerCircleBorderDoubleSpinBox->setValue(0.5);
    m_ui->InnerCircleBorderDoubleSpinBox->setMaximum(DBL_MAX);
    //isToRightButton
    /*connect(m_ui->isToLeftBtn, QOverload<bool>::of(&QToolButton::toggled), [=](bool checked) {
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
    });*/
    //itatic
    connect(m_ui->italicBtn, QOverload<bool>::of(&QToolButton::toggled), [=](bool checked) {
        
        if (checked) {
            QPixmap icon(":/ui/icons/images/itatic.png");
            m_ui->italicBtn->setIcon(QIcon(icon));
        }
        else {
            QPixmap icon(":/ui/icons/images/straight.png");
            m_ui->italicBtn->setIcon(QIcon(icon));
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
    //list
    m_viewItemModel = new QStandardItemModel();
    QHeaderView* head = new QHeaderView(Qt::Horizontal);
    //head->setStyleSheet("QHeaderView{color:rgb(128, 128, 128)}");
    //head->setStyleSheet("QHeaderView::section {background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 #00007f, stop: 0.5 #00007f,stop: 0.6 #00007f, stop:1 #00007f);color: white;}");

    QStringList hearderLabels;
    hearderLabels.append(QString(tr("content")));
    //hearderLabels.append(QString(tr("font")));
    //iStandardItemModel->setHorizontalHeaderLabels(hearderLabels);
    m_viewItemModel->setHorizontalHeaderItem(0, new QStandardItem(tr("content")));
    m_viewItemModel->setHorizontalHeaderItem(1, new QStandardItem(tr("font")));
    m_viewItemModel->setHorizontalHeaderItem(2, new QStandardItem(tr("property")));
    m_viewItemModel->setHorizontalHeaderItem(3, new QStandardItem(tr("text spacing")));
    m_viewItemModel->setHorizontalHeaderItem(4, new QStandardItem(tr("text height")));
    //m_ui->tableView->setHorizontalHeader(head);
    m_ui->tableView->setModel(m_viewItemModel);
    QString headViewStr("QHeaderView::section{background-color:rgb(245, 245, 245);color:rgb(18, 18, 18);bold:true;}");
    m_ui->tableView->horizontalHeader()->setStyleSheet(headViewStr);
    //m_ui->tableView->verticalHeader()->setStyleSheet(headViewStr);
    m_checkedItemStyle = QString("QTableView::indicator:checked {color:rgb(85, 85, 85);}");
    m_uncheckedItemStyle = QString("QTableView::indicator:unchecked {color:rgb(185, 185, 185);}");
    m_ui->tableView->setStyleSheet("QTableView::item {color:rgb(85, 85, 85);}QTableView QTableCornerButton::section {background-color:rgb(145, 145, 145);}");
    m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //add
    /*connect(m_ui->addToolButton, QOverload<bool>::of(&QToolButton::clicked), [=](bool checked) {
        QString contentStr = m_ui->lineEdit->text().trimmed();
        if (contentStr.isEmpty()) {
            return;
        }
        int index = m_ui->circleStampLayoutComboBox->currentIndex();
        int row = m_viewItemModel->rowCount();
        //signal row
        if (index == 0 && row > 0) {
            return;
        }
        else if (index == 1 && row > 1) {
            return;
        }
        else if (index == 2 && row > 2) {
            return;
        }
        QString propertyStr;
        propertyStr = m_textRowProperty[row];
        
        //addTableViewRow(row, contentStr, m_ui->fontComboBox->currentText(), propertyStr);
    });*/
    // delete
    /*connect(m_ui->delToolButton, QOverload<bool>::of(&QToolButton::clicked), [=](bool checked) {
        QModelIndexList list = m_ui->tableView->selectionModel()->selectedRows();
        m_viewItemModel->removeRows(list[0].row(), list.size());

    });*/
    //selection change
    connect(m_ui->tableView->selectionModel(), QOverload<const QModelIndex&, const QModelIndex&>::of(&QItemSelectionModel::currentChanged), [=](const QModelIndex& current, const QModelIndex& previous) {
        if (!current.isValid()) {
            return;
        }
        QStandardItem* item0 = m_viewItemModel->item(current.row(), 0);
        QStandardItem* item1 = m_viewItemModel->item(current.row(), 1);
        QStandardItem* item3 = m_viewItemModel->item(current.row(), 3);
        QStandardItem* item4 = m_viewItemModel->item(current.row(), 4);
        m_ui->fontComboBox->blockSignals(true);
        m_ui->fontComboBox->setCurrentText(item1->text());
        m_ui->fontComboBox->blockSignals(false);
        m_ui->lineEdit->blockSignals(true);
        m_ui->lineEdit->setText(item0->text());
        m_ui->lineEdit->blockSignals(false);
        m_ui->hSpaceSpinBox->blockSignals(true);
        m_ui->hSpaceSpinBox->setValue(item3->text().toDouble());
        m_ui->hSpaceSpinBox->blockSignals(false);
        m_ui->textSizeSpinBox->blockSignals(true);
        m_ui->textSizeSpinBox->setValue(item4->text().toDouble());
        m_ui->textSizeSpinBox->blockSignals(false);
    });
    //text content
    connect(m_ui->lineEdit, QOverload<const QString&>::of(&QLineEdit::textChanged), [=](const QString& text) {
        if (m_ui->tableView->selectionModel()->selectedRows().size() == 1) {
            QModelIndex index = m_ui->tableView->selectionModel()->currentIndex();
            QStandardItem* item = m_viewItemModel->item(index.row(), 0);
            item->setText(text);
        }
    });
    //font family
    connect(m_ui->fontComboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged), [=](const QString& text) {
        QModelIndexList list = m_ui->tableView->selectionModel()->selectedRows();
        if (list.size() == 1) {
            QStandardItem* item = m_viewItemModel->item(list[0].row(), 1);
            item->setText(text);
        }
    });
    //spacing
    connect(m_ui->hSpaceSpinBox, QOverload<double>::of(&LaserDoubleSpinBox::valueChanged), [=](double v) {
        QModelIndexList list = m_ui->tableView->selectionModel()->selectedRows();
        if (list.size() == 1) {
            QStandardItem* item = m_viewItemModel->item(list[0].row(), 3);
            item->setText(QString::number(v));
        }
    });
    //text height
    connect(m_ui->textSizeSpinBox, QOverload<double>::of(&LaserDoubleSpinBox::valueChanged), [=](double v) {
        QModelIndexList list = m_ui->tableView->selectionModel()->selectedRows();
        if (list.size() == 1) {
            QStandardItem* item = m_viewItemModel->item(list[0].row(), 4);
            item->setText(QString::number(v));
        }
    });
    m_preLayoutIndex = 0;
    for (int i = 0; i < m_ui->circleStampLayoutComboBox->count(); i++) {
        m_tablesModelList.append(QMap<QModelIndex, itemStruct>());
    }
    m_ui->textSizeSpinBox->setValue(6);
    addTableViewRow(0, m_textInitRowContent[0], m_ui->fontComboBox->currentText(),
        m_textRowProperty[0], Qt::Checked, m_ui->hSpaceSpinBox->value(), m_ui->textSizeSpinBox->value());
    connect(m_ui->circleStampLayoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        
        if (m_isEllipse)   {         
            if (index == 0) {
                m_ui->circleDiameterSpinBox->setValue(50);
                m_ui->circleHeightSpinBox->setValue(35);
                m_ui->marginSpinBox->setValue(1);
                m_ui->AdditionGroupBox->setChecked(false);
                
            }
            else if (index == 1) {
                m_ui->circleDiameterSpinBox->setValue(50);
                m_ui->circleHeightSpinBox->setValue(35);
                m_ui->marginSpinBox->setValue(1);
                m_ui->AdditionGroupBox->setChecked(false);
            }
            else if (index == 2) {
                m_ui->circleDiameterSpinBox->setValue(50);
                m_ui->circleHeightSpinBox->setValue(35);
                m_ui->marginSpinBox->setValue(1);
                m_ui->AdditionGroupBox->setChecked(true);
            }
            else if (index == 3) {
                m_ui->circleDiameterSpinBox->setValue(40);
                m_ui->circleHeightSpinBox->setValue(30);
                m_ui->marginSpinBox->setValue(0.5);
                m_ui->AdditionGroupBox->setChecked(false);
            }
            else if (index == 4) {
                m_ui->circleDiameterSpinBox->setValue(40);
                m_ui->circleHeightSpinBox->setValue(30);
                m_ui->marginSpinBox->setValue(0.5);
                m_ui->AdditionGroupBox->setChecked(false);
            }
        }
        else {
            m_ui->circleDiameterSpinBox->setValue(40);
            if (index == 0) {
                m_ui->angleSpinBox->setValue(210);
                m_ui->circleBorderSpinBox->setValue(1.0);
                m_ui->embleSizeSpinBox->setValue(13);
                m_ui->lineWidthSpinBox->setValue(0);
                m_ui->lineGroupBox->setChecked(false);
            }
            else if (index == 1) {
                m_ui->angleSpinBox->setValue(210);
                m_ui->circleBorderSpinBox->setValue(1.0);
                m_ui->embleSizeSpinBox->setValue(13);
                m_ui->lineWidthSpinBox->setValue(0);
                m_ui->lineGroupBox->setChecked(false);
            }
            else if (index == 2) {
                m_ui->angleSpinBox->setValue(225);
                m_ui->circleBorderSpinBox->setValue(1.0);
                m_ui->embleSizeSpinBox->setValue(13);
                m_ui->lineWidthSpinBox->setValue(0);
                m_ui->lineGroupBox->setChecked(false);
            }
            else if (index == 3) {
                m_ui->angleSpinBox->setValue(145);
                m_ui->circleBorderSpinBox->setValue(1.5);
                m_ui->embleSizeSpinBox->setValue(6);
                m_ui->lineWidthSpinBox->setValue(1.8);
                m_ui->lineGroupBox->setChecked(true);
            }
        }
        
        int count = m_viewItemModel->rowCount();
        //存储上一个
        QMap<QModelIndex, itemStruct> itemModelMap;
        for (int i = 0; i < m_viewItemModel->rowCount(); i++) {
            QStandardItem* item0 = m_viewItemModel->item(i, 0);
            QStandardItem* item1 = m_viewItemModel->item(i, 1);
            QStandardItem* item2 = m_viewItemModel->item(i, 2);
            QStandardItem* item3 = m_viewItemModel->item(i, 3);
            QStandardItem* item4 = m_viewItemModel->item(i, 4);
            itemModelMap.insert(item0->index(), itemStruct(item0->text(), item0->checkState()));
            itemModelMap.insert(item1->index(), itemStruct(item1->text(), item0->checkState()));
            itemModelMap.insert(item2->index(), itemStruct(item2->text(), item0->checkState()));
            itemModelMap.insert(item3->index(), itemStruct(item3->text(), item0->checkState()));
            itemModelMap.insert(item4->index(), itemStruct(item4->text(), item0->checkState()));
        }
        m_tablesModelList[m_preLayoutIndex] = itemModelMap;
        //修改
        QMap<QModelIndex, itemStruct> preModelList = m_tablesModelList[m_preLayoutIndex];
        QMap<QModelIndex, itemStruct> curModelList = m_tablesModelList[index];
        if (!curModelList.isEmpty()) {
            m_viewItemModel->removeRows(0, m_viewItemModel->rowCount());
            for (QMap<QModelIndex, itemStruct>::iterator i = curModelList.begin(); i != curModelList.end(); i++) {
                QStandardItem* item = new QStandardItem(i.value().getStr());
                item->setTextAlignment(Qt::AlignCenter);
                if (i.key().column() == 0) {
                    item->setCheckable(true);
                    item->setCheckState(i.value().getCheckState());
                }
                
                m_viewItemModel->setItem(i.key().row(), i.key().column(), item);
            }
        }
        //init
        else {
            qreal initSpacing = m_ui->hSpaceSpinBox->value();
            qreal initLineSpacing = 0.5;
            if (count > 1) {
                m_viewItemModel->removeRows(1, m_viewItemModel->rowCount() - 1);
            }
            if (m_isEllipse) {
                if (index == 1 || index == 2) {
                    addTableViewRow(0, m_textInitRowContent[0], m_ui->fontComboBox->currentText(), m_textRowProperty[0], Qt::Checked, initSpacing, 6.0);
                    addTableViewRow(1, m_textInitRowContent[1], m_ui->fontComboBox->currentText(), m_textRowProperty[1], Qt::Checked, initLineSpacing, 4.7);
                }
                else if (index == 3) {
                    addTableViewRow(0, m_textInitRowContent[0], m_ui->fontComboBox->currentText(), m_textRowProperty[0], Qt::Checked, initSpacing, 4.3);
                    addTableViewRow(1, m_textInitRowContent[3], m_ui->fontComboBox->currentText(), m_textRowProperty[1], Qt::Checked, initLineSpacing, 4.6);
                    addTableViewRow(2, m_textInitRowContent[2], "Arial", m_textRowProperty[3], Qt::Checked, 0.477, 5.0);
                }
                else if (index == 4) {
                    addTableViewRow(0, m_textInitRowContent[0], m_ui->fontComboBox->currentText(), m_textRowProperty[0], Qt::Checked, initSpacing, 4.3);
                    addTableViewRow(1, m_textInitRowContent[3], m_ui->fontComboBox->currentText(), m_textRowProperty[1], Qt::Checked, initLineSpacing, 4.6);
                    addTableViewRow(2, m_textInitRowContent[2], "Arial", m_textRowProperty[3], Qt::Checked, 0.477, 5.0);
                    addTableViewRow(3, m_textInitRowContent[4], "Arial", m_textRowProperty[4], Qt::Checked, initLineSpacing, 3.1);
                }
            }
            else {
                if (index == 1 || index == 3) {
                    addTableViewRow(0, m_textInitRowContent[0], m_ui->fontComboBox->currentText(), m_textRowProperty[0], Qt::Checked, initSpacing, 6);
                    addTableViewRow(1, m_textInitRowContent[1], m_ui->fontComboBox->currentText(), m_textRowProperty[1], Qt::Checked, initLineSpacing, 4.7);
                    addTableViewRow(2, m_textInitRowContent[2], "Arial", m_textRowProperty[2], Qt::Unchecked, 0.477, 3.7);
                }
                else if (index == 2) {
                    addTableViewRow(0, m_textInitRowContent[0], m_ui->fontComboBox->currentText(), m_textRowProperty[0], Qt::Checked, initSpacing, 6);
                    addTableViewRow(1, m_textInitRowContent[2], "Arial", m_textRowProperty[2], Qt::Checked, 0.477, 3.7);
                    addTableViewRow(2, m_textInitRowContent[1], m_ui->fontComboBox->currentText(), m_textRowProperty[1], Qt::Unchecked, initSpacing, 4.7);
                }
            }
        }
        m_preLayoutIndex = index;
    });
    //item change
    connect(m_viewItemModel, QOverload<QStandardItem*>::of(&QStandardItemModel::itemChanged), [=](QStandardItem* item) {       
        Qt::CheckState state = Qt::Unchecked;
        int r = item->index().row();
        QStandardItem* content = m_viewItemModel->item(r, 0);
        QStandardItem* font = m_viewItemModel->item(r, 1);
        QStandardItem* property = m_viewItemModel->item(r, 2);
        QStandardItem* spacing = m_viewItemModel->item(r, 3);
        QStandardItem* textH = m_viewItemModel->item(r, 4);
        if (content) {
            state = content->checkState();
        }
        if (state == Qt::Checked) {
            item->setBackground(Qt::white);
            if (font) {
                font->setBackground(Qt::white);
            }
            if (property) {
                property->setBackground(Qt::white);
            }
            if (spacing) {
                spacing->setBackground(Qt::white);
            }
            if (textH) {
                textH->setBackground(Qt::white);
            }
        }
        else if (state == Qt::Unchecked) {
            //QColor color = QColor(238, 238, 238);
            QColor color = QColor(253, 253, 212);
            item->setBackground(color);
            if (font) {
                font->setBackground(color);
            }
            if (property) {
                property->setBackground(color);
            }
            if (spacing) {
                spacing->setBackground(color);
            }
            if (textH) {
                textH->setBackground(color);
            }
        }
        
    });
    //Embelm ComboBox
    QPixmap starEmblem(":/ui/icons/images/star.png");
    QPixmap partyEmblem(":/ui/icons/images/partyEmblem.png");
    m_ui->emblemComboBox->addItem(QIcon(starEmblem), tr("Star Emblem"));
    m_ui->emblemComboBox->addItem(QIcon(partyEmblem), tr("Party Emblem"));
    m_ui->emblemComboBox->addItem(QIcon(), tr("Import Image"));
    m_ui->emblemComboBox->setItemDelegate(new QStyledItemDelegate());
    connect(m_ui->emblemComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        if (index == 2) {
            QString name = QFileDialog::getOpenFileName(nullptr, "open image", ".", "Images (*.png)");           
            m_importEmblemPath = name;
            m_ui->emblemComboBox->setItemIcon(2, QIcon(name));
        }
    });
    //Embelm size
    m_ui->embleSizeSpinBox->setMaximum(DBL_MAX);
    m_ui->embleSizeSpinBox->setMinimum(0);
    
    connect(m_ui->embleSizeSpinBox, QOverload<double>::of(&LaserDoubleSpinBox::valueChanged), [=](double v) {
        qreal maxVal = m_ui->circleDiameterSpinBox->value();
        if (v > maxVal) {
            m_ui->embleSizeSpinBox->blockSignals(true);
            m_ui->embleSizeSpinBox->setValue(maxVal);
            m_ui->embleSizeSpinBox->blockSignals(false);
        }
    });
    QPushButton* previewBtn = m_ui->buttonBox->button(QDialogButtonBox::Apply);
    previewBtn->setText(tr("Preview"));
    QPushButton* okBtn = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    okBtn->setText(tr("Ok"));
    QPushButton* cancleBtn = m_ui->buttonBox->button(QDialogButtonBox::Cancel);
    cancleBtn->setText(tr("Cancel"));
    connect(okBtn, &QPushButton::clicked,this, &StampCircleDialog::okBtnAccept);
    connect(previewBtn, &QPushButton::clicked, this, &StampCircleDialog::previewBtnAccept);
}
StampCircleDialog::~StampCircleDialog()
{
}

void StampCircleDialog::addTableViewRow(int row, QString contentStr, QString fontStr, QString propertyStr, Qt::CheckState checkState, qreal textSpacing, qreal textHeight)
{
    
    QStandardItem* content = new QStandardItem(contentStr);
    QStandardItem* font = new QStandardItem(fontStr);
    QStandardItem* property = new QStandardItem(propertyStr);
    QStandardItem* spacing = new QStandardItem(QString::number(textSpacing));
    QStandardItem* textH = new QStandardItem(QString::number(textHeight));

    content->setTextAlignment(Qt::AlignCenter);
    font->setTextAlignment(Qt::AlignCenter);
    property->setTextAlignment(Qt::AlignCenter);
    spacing->setTextAlignment(Qt::AlignCenter);
    textH->setTextAlignment(Qt::AlignCenter);
    content->setCheckable(true);   

    m_viewItemModel->setItem(row, 0, content);
    m_viewItemModel->setItem(row, 1, font);
    m_viewItemModel->setItem(row, 2, property);
    m_viewItemModel->setItem(row, 3, spacing);
    m_viewItemModel->setItem(row, 4, textH);

    content->setCheckState(checkState);
}

QList<LaserPrimitive*> StampCircleDialog::createStampPrimitive()
{
    bool stampIntaglio = m_ui->stampIntaglioCheckBox->isChecked();
    QList<LaserPrimitive*> stampList;
    //create frame
    qreal circleW = m_ui->circleDiameterSpinBox->value() * 1000;
    qreal circleH = m_ui->circleHeightSpinBox->value() * 1000;
    qreal circleBorder = m_ui->circleBorderSpinBox->value() * 1000;
    //QPointF point = m_viewer->mapToScene(m_viewer->rect().center());
    QRectF allBounds = m_viewer->AllItemsSceneBoundingRect();
    QPointF point = allBounds.topRight();
    //point = QPointF(point.x() - circleW * 0.5, point.y() - circleW * 0.5);
    QRect rect;

    if (m_isEllipse) {
        rect = QRect(point.x(), point.y(), circleW, circleH);
    }
    else {
        rect = QRect(point.x(), point.y(), circleW, circleW);
    }
    LaserRing* circle = new LaserRing(m_scene->document(), rect, circleBorder, stampIntaglio, QTransform(), m_layerIndex);
    stampList.append(circle);
    LaserRing* innerCircle = nullptr;;
    QRect innerRect;
    qreal innerBorder;
    qreal innderMargin;
    if (m_ui->innerCircleGroupBox->isChecked()) {
        innderMargin = m_ui->InnerCircleMarginDoubleSpinBox->value() * 1000;
        innerBorder = m_ui->InnerCircleBorderDoubleSpinBox->value() * 1000;
        qreal innerW, innerH;
        if (m_isEllipse) {
            innerW = circleW - innderMargin * 2 - circleBorder * 2;
            innerH = circleH - innderMargin * 2 - circleBorder * 2;
        }
        else {
            innerW = circleW - innderMargin * 2 - circleBorder * 2;
            innerH = circleW - innderMargin * 2 - circleBorder * 2;
        }


        innerRect = QRect(point.x() + innderMargin + circleBorder, point.y() + innderMargin + circleBorder, innerW, innerH);
        innerCircle = new LaserRing(m_scene->document(), innerRect, innerBorder, stampIntaglio, QTransform(), m_layerIndex);
        innerCircle->setInner(true);
        stampList.append(innerCircle);
    }

    //create text
    //int layoutIndex = m_ui->frameStampLayoutComboBox->currentIndex();
    QSize signalSize;
    QSize doubleVerticalSize;
    qreal textMargin = m_ui->marginSpinBox->value() * 1000;
    qreal angle = m_ui->angleSpinBox->value();
    QRect textBounds;
    qreal diff;
    if (innerCircle) {
        qreal diff = innerBorder + textMargin;
        textBounds = QRect(innerRect.left() + diff, innerRect.top() + diff, innerRect.width() - 2 * diff, innerRect.height() - 2 * diff);
    }
    else {
        diff = circleBorder + textMargin;
        textBounds = QRect(rect.left() + diff, rect.top() + diff, rect.width() - 2 * diff, rect.height() - 2 * diff);
    }

    QString content = m_ui->lineEdit->text().trimmed();
    bool bold = m_ui->boldBtn->isChecked();
    bool italic = m_ui->italicBtn->isChecked();
    //bool isRightToLeft = m_ui->isToLeftBtn->isChecked();

    bool containsDoubleText = false;

    int textLength = content.size();
    int layoutType = m_ui->circleStampLayoutComboBox->currentIndex();
    //
    for (int i = 0; i < m_viewItemModel->rowCount(); i++) {
        QStandardItem* content = m_viewItemModel->item(i, 0);
        QStandardItem* property = m_viewItemModel->item(i, 2);
        QString family = m_viewItemModel->item(i, 1)->text();
        QString contentStr = content->text();
        int textLength = contentStr.size();
        qreal textSpace = m_viewItemModel->item(i, 3)->text().toDouble() * 1000;
        qreal textHeight = m_viewItemModel->item(i, 4)->text().toDouble() * 1000;
        if (content->checkState() == Qt::Unchecked) {
            continue;
        }
        QString propertyText = property->text();
        //Top Circle Text
        if (propertyText == m_textRowProperty[0]) {

            LaserCircleText* topCircleText = new LaserCircleText(m_scene->document(), contentStr, textBounds, angle,
                bold, italic, false, stampIntaglio, family, textSpace, true, 0.0, 0.0, QSize(), QTransform(), m_layerIndex);
            QSize textSize(topCircleText->textSize().width(), textHeight);
            topCircleText->computeTextPath(topCircleText->angle(), textSize, false);
            stampList.append(topCircleText);
        }
        //Horizontal Text
        else if (propertyText == m_textRowProperty[1]) {

            qreal h, w, centerX, centerY;
            if (m_isEllipse) {
                //h = 4700;
                h = textHeight;
                w = textBounds.width() * 0.5;
                centerY = rect.center().y() + 4200 + textHeight * 0.5;
                centerX = textBounds.left() + textBounds.width() * 0.5;
            }
            else {
                //h = textBounds.height() * (1.0 / 6);
                h = textHeight;
                w = textBounds.width() * 0.63;
                centerY = textBounds.top() + textBounds.height() * 0.788;
                centerX = textBounds.left() + textBounds.width() * 0.5;
            }


            QSize lineTextSize((w - textSpace * (textLength - 1)) / textLength, h);

            LaserHorizontalText* lineText = new LaserHorizontalText(m_scene->document(), content->text(), lineTextSize,
                QPointF(centerX, centerY), bold, italic, false, stampIntaglio, family, textSpace, QTransform(), m_layerIndex);
            stampList.append(lineText);
        }
        //Bottom Circle Text
        else if (propertyText == m_textRowProperty[2]) {
            int contentStrSize = contentStr.size();
            QString invertContentStr;
            for (QChar c : contentStr) {
                invertContentStr.prepend(c);
            }

            LaserCircleText* bottomCircleText = new LaserCircleText(m_scene->document(), invertContentStr, textBounds, 320 - angle,
                bold, italic, false, stampIntaglio, family, textSpace, true, 0.0, 0.0, QSize(), QTransform(), m_layerIndex);

            //qreal hsize = bottomCircleText->textSize().height() * 0.5;
            qreal hsize = textHeight;
            bottomCircleText->setOffsetRotateAngle(180);
            bottomCircleText->setTextSize(QSize(hsize, hsize), false);
            bottomCircleText->computeMoveTextPath(180);
            bottomCircleText->recompute();
            stampList.append(bottomCircleText);
        }
        //Horizontal Invoice Number
        else if (propertyText == m_textRowProperty[3]) {
            //qreal h = 5600;
            qreal h = textHeight;
            //qreal w = rect.width() * (2.0 / 3.0);
            qreal w = 26000;
            QSize size((w - (textLength - 1) * textSpace) / textLength, h);
            LaserHorizontalText* text = new LaserHorizontalText(m_scene->document(), contentStr, size, rect.center(),
                bold, italic, false, stampIntaglio, family, textSpace, QTransform(), m_layerIndex);
            text->setBoundingRectWidth(26000);
            stampList.append(text);
        }
        //Horizontal Bottom Number
        else if (propertyText == m_textRowProperty[4]) {
            //qreal h = 3400;
            qreal h = textHeight;
            //qreal w = 3400 * 1.5;
            qreal w = 1700;
            qreal centerY = rect.center().y() + 10000 + textHeight * 0.5;

            QSize size(w, h);
            /*if (m_ui->innerCircleGroupBox->isChecked()) {
                centerY = rect.bottom() - size.height() - innerBorder;
            }
            else {
                centerY = rect.bottom() - size.height();
            }*/
            QPoint center(rect.center().x(), centerY);
            LaserHorizontalText* text = new LaserHorizontalText(m_scene->document(), contentStr, size, center,
                bold, italic, false, stampIntaglio, family, textSpace, QTransform(), m_layerIndex);
            text->setBoundingRectHeight(h);
            stampList.append(text);
        }

    }
    //create emblem
    qreal radius = m_ui->embleSizeSpinBox->value() * 1000 * 0.5;
    int emblemIndex = m_ui->emblemComboBox->currentIndex();
    bool drawEmble = false;
    if (m_isEllipse) {
        if (m_ui->AdditionGroupBox->isChecked()) {
            drawEmble = true;
        }
        else {
            drawEmble = false;
        }

    }
    else {
        drawEmble = true;
    }
    if (drawEmble) {
        if (emblemIndex == 0) {
            LaserStar* star = new LaserStar(m_scene->document(), rect.center(), radius, stampIntaglio, QTransform(), m_layerIndex);
            stampList.append(star);
        }
        else if (emblemIndex == 1) {
            LaserPartyEmblem* party = new LaserPartyEmblem(m_scene->document(), rect.center(), radius, stampIntaglio, QTransform(), m_layerIndex);
            stampList.append(party);
        }
        else if (emblemIndex == 2) {
            QFile file(m_importEmblemPath);
            file.open(QFile::ReadOnly);
            QByteArray data = file.readAll();
            QImage img;
            bool bl = img.loadFromData(data);
            if (bl) {
                QRect bounds(rect.center().x() - radius, rect.center().y() - radius,
                    radius * 2, radius * 2);
                LaserStampBitmap* stampBitmap = new LaserStampBitmap(img, bounds, stampIntaglio, m_scene->document(), QTransform(), m_layerIndex);
                stampList.append(stampBitmap);
            }
            
        }
    }

    //create line
    if (m_ui->lineGroupBox->isChecked()) {
        qreal circleBorder = m_ui->circleBorderSpinBox->value() * 1000;
        qreal innerBorder = m_ui->InnerCircleBorderDoubleSpinBox->value() * 1000;
        qreal innerMargin = m_ui->InnerCircleMarginDoubleSpinBox->value() * 1000;
        bool needInner = m_ui->innerCircleGroupBox->isChecked();
        qreal w = rect.width() * 0.25;
        qreal h = m_ui->lineWidthSpinBox->value() * 1000;
        qreal top = rect.center().y() - h * 0.5;
        qreal diff, left1, left2;
        if (!needInner) {
            diff = circleBorder;
        }
        else {
            diff = circleBorder + innerBorder + innerMargin;
        }
        qreal offset = 200;
        if (innerCircle) {
            offset = innerBorder * 0.3;
        }
        left1 = rect.left() + diff - offset;
        left2 = rect.right() - diff + offset - w;
        QRect lineRect1(left1, top, w, h);
        QRect lineRect2(left2, top, w, h);
        qreal borderWidth = h * 0.5;
        LaserFrame* line1 = new LaserFrame(m_scene->document(), lineRect1, borderWidth, 0, stampIntaglio, QTransform(), m_layerIndex);
        line1->setInner(true);
        line1->setZValue(3);
        line1->setNeedAuxiliaryLine(false);
        LaserFrame* line2 = new LaserFrame(m_scene->document(), lineRect2, borderWidth, 0, stampIntaglio, QTransform(), m_layerIndex);
        line2->setInner(true);
        line2->setZValue(3);
        line2->setNeedAuxiliaryLine(false);
        stampList.append(line1);
        stampList.append(line2);
    }
    return stampList;
}

void StampCircleDialog::accept()
{
    QDialog::accept();
}
