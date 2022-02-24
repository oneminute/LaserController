#include "StampStripDialog.h"
#include "ui_StampStripDialog.h"
#include<QAbstractItemView>
#include<QListView>
#include<QStyledItemDelegate>
#include<QComboBox>
#include<QStringList>
#include<QHeaderView>
#include"scene/LaserPrimitive.h"

StampStripDialog::StampStripDialog(LaserScene* scene, QWidget* parent) 
   : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint),
    m_ui(new Ui::StampStripDialog), m_scene(scene)
{
    m_viewer = qobject_cast<LaserViewer*> (scene->views()[0]);
    m_ui->setupUi(this);
    //LayoutComboBox
    QPixmap fourPm(":/ui/icons/images/frameFour.png");
    QPixmap threePm(":/ui/icons/images/frameThreeL.png");
    QPixmap threeRPm(":/ui/icons/images/frameThreeR.png");
    m_ui->frameStampLayoutComboBox->addItem(QIcon(fourPm), tr("Signal Row"));
    m_ui->frameStampLayoutComboBox->addItem(QIcon(threePm), tr("Multi Row"));
    m_ui->frameStampLayoutComboBox->addItem(QIcon(threeRPm), tr("Multi Column"));
    m_ui->frameStampLayoutComboBox->setItemDelegate(new QStyledItemDelegate());
    m_preLayoutIndex = 0;
    for (int i = 0; i < m_ui->frameStampLayoutComboBox->count(); i++) {
        m_tablesModelList.append(QMap<QModelIndex, QString>());
    }
    connect(m_ui->frameStampLayoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        int count = m_viewItemModel->rowCount();
        //存储上一个
        QMap<QModelIndex, QString> itemModelMap;
        for (int i = 0; i < m_viewItemModel->rowCount(); i++) {
            QStandardItem* item0 = m_viewItemModel->item(i, 0);
            QStandardItem* item1 = m_viewItemModel->item(i, 1);
            itemModelMap.insert(item0->index(), item0->text());
            itemModelMap.insert(item1->index(), item1->text());
        }
        m_tablesModelList[m_preLayoutIndex] = itemModelMap;
        //修改
        QMap<QModelIndex, QString> preModelList = m_tablesModelList[m_preLayoutIndex];
        QMap<QModelIndex, QString> curModelList = m_tablesModelList[index];
        if (!curModelList.isEmpty()) {
            m_viewItemModel->removeRows(0, m_viewItemModel->rowCount());
            for (QMap<QModelIndex, QString>::iterator i = curModelList.begin(); i != curModelList.end(); i ++) {
                QStandardItem* item = new QStandardItem(i.value());
                item->setTextAlignment(Qt::AlignCenter);
                m_viewItemModel->setItem(i.key().row(), i.key().column(), item);
            }
        }
        else {
            if (index == 0 && count > 1) {
                m_viewItemModel->removeRows(1, m_viewItemModel->rowCount() - 1);
            }
        }
        
        m_preLayoutIndex = index;
        
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
    m_ui->frameWidthSpinBox->setValue(30);
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
    //list
    m_viewItemModel = new QStandardItemModel();
    QHeaderView* head = new QHeaderView(Qt::Horizontal);
    //head->setStyleSheet("QHeaderView{color:rgb(128, 128, 128)}");
    //head->setStyleSheet("QHeaderView::section {background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 #00007f, stop: 0.5 #00007f,stop: 0.6 #00007f, stop:1 #00007f);color: white;}");

    QStringList hearderLabels;
    hearderLabels.append(QString(tr("content")));
    hearderLabels.append(QString(tr("font")));
    //iStandardItemModel->setHorizontalHeaderLabels(hearderLabels);
    m_viewItemModel->setHorizontalHeaderItem(0, new QStandardItem(tr("content")));
    m_viewItemModel->setHorizontalHeaderItem(1, new QStandardItem(tr("font")));
    //m_ui->tableView->setHorizontalHeader(head);
    m_ui->tableView->setModel(m_viewItemModel);
    QString headViewStr("QHeaderView::section{background-color:rgb(245, 245, 245);color:rgb(18, 18, 18);bold:true;}");
    m_ui->tableView->horizontalHeader()->setStyleSheet(headViewStr);
    //m_ui->tableView->verticalHeader()->setStyleSheet(headViewStr);
    m_ui->tableView->setStyleSheet("QTableView::item{color:rgb(85, 85, 85);}QTableView QTableCornerButton::section {background-color:rgb(145, 145, 145);}");
    m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //text content
    
    connect(m_ui->lineEdit, QOverload<const QString&>::of(&QLineEdit::textChanged), [=](const QString& text) {
        /*与后面功能冲突
        bool b = text.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
        if (b) {
            m_ui->fontComboBox->setWritingSystem(QFontDatabase::SimplifiedChinese);
        }
        else {
            m_ui->fontComboBox->setWritingSystem(QFontDatabase::Any);
        }*/
        if (m_ui->tableView->selectionModel()->selectedRows().size() == 1) {
             QModelIndex index = m_ui->tableView->selectionModel()->currentIndex();
             QStandardItem* item = m_viewItemModel->item(index.row(), index.column());
             item->setText(text);
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
    //add
    connect(m_ui->addToolButton, QOverload<bool>::of(&QToolButton::clicked), [=](bool checked) {
        QString str = m_ui->lineEdit->text().trimmed();
        if (str.isEmpty()) {
            return;
        }
        int index = m_ui->frameStampLayoutComboBox->currentIndex();
        int row = m_viewItemModel->rowCount();
        //signal row
        if (index == 0 && row > 0) {
            return;
        }
        QStandardItem* content = new QStandardItem(str);
        QStandardItem* font = new QStandardItem(m_ui->fontComboBox->currentText());
        content->setTextAlignment(Qt::AlignCenter);
        font->setTextAlignment(Qt::AlignCenter);
        
        m_viewItemModel->setItem(row, 0, content);
        m_viewItemModel->setItem(row, 1, font);
        
    });
    // delete
    connect(m_ui->delToolButton, QOverload<bool>::of(&QToolButton::clicked), [=](bool checked) {
        QModelIndexList list = m_ui->tableView->selectionModel()->selectedRows();
        m_viewItemModel->removeRows(list[0].row(), list.size());
        
    });
    //selection change
    connect(m_ui->tableView->selectionModel(), QOverload<const QModelIndex&, const QModelIndex&>::of(&QItemSelectionModel::currentChanged), [=](const QModelIndex& current, const QModelIndex& previous) {
        if (!current.isValid()) {
            return;
        }
        QStandardItem* item0 = m_viewItemModel->item(current.row(), 0);
        QStandardItem* item1 = m_viewItemModel->item(current.row(), 1);
        m_ui->fontComboBox->blockSignals(true);
        m_ui->fontComboBox->setCurrentText(item1->text());
        m_ui->fontComboBox->blockSignals(false);
        m_ui->lineEdit->setText(item0->text());
        
    });
    //font family
    connect(m_ui->fontComboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged), [=](const QString& text) {
        QModelIndexList list = m_ui->tableView->selectionModel()->selectedRows();
        if (list.size() == 1) {
            QStandardItem* item = m_viewItemModel->item(list[0].row(), 1);
            item->setText(text);
        }
    });
    //text margins
    m_ui->hMarginSpinBox->setMaximum(DBL_MAX);
    m_ui->vMarginSpinBox->setMaximum(DBL_MAX);
    //text space
    m_ui->hSpaceSpinBox->setMaximum(DBL_MAX);
    m_ui->vSpaceSpinBox->setMaximum(DBL_MAX);
}
StampStripDialog::~StampStripDialog()
{
}

void StampStripDialog::accept()
{
    QDialog::accept();
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
    bool needAuxiliary = false;
    LaserFrame* frame = new LaserFrame(m_scene->document(), rect, frameBorder, cornerRadius, QTransform(), m_viewer->curLayerIndex(), frameType);
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
        innerFrame = new LaserFrame(m_scene->document(), innerRect, innerBorder, cornerRadius, QTransform(), m_viewer->curLayerIndex(), frameType);
        innerFrame->setNeedAuxiliaryLine(false);
        stampList.append(innerFrame);
    }
    
    //create text
    int layoutIndex = m_ui->frameStampLayoutComboBox->currentIndex();
    bool bold = m_ui->boldBtn->isChecked();
    bool itatic = m_ui->itaticBtn->isChecked();
    bool fill = !m_ui->fillBtn->isChecked();
    bool isRightToLeft = m_ui->isToLeftBtn->isChecked();
    //text size
    qreal textHMargin = m_ui->hMarginSpinBox->value() * 1000;
    qreal textVMargin = m_ui->vMarginSpinBox->value() * 1000;
    qreal textHSpace = m_ui->hSpaceSpinBox->value() * 1000;
    qreal textVSpace = m_ui->vSpaceSpinBox->value() * 1000;
    QPointF center = rect.center();
    QRect textRect;
    if (innerFrame) {
        textRect = QRect(innerRect.left() + innerBorder + textHMargin,
            innerRect.top() + innerBorder + textVMargin,
            innerRect.width() - 2 * (innerBorder + textHMargin),
            innerRect.height() - 2 * (innerBorder + textHMargin));
    }
    else {
        textRect = QRect(rect.left() + frameBorder + textHMargin, 
            rect.top() + frameBorder + textVMargin, 
            rect.width() - 2*(frameBorder + textHMargin), 
            rect.height() - 2 * (frameBorder + textHMargin));
    }

    for (int r = 0; r < m_viewItemModel->rowCount(); r++) {
        QStandardItem* item1 = m_viewItemModel->item(r, 0);
        QString content = item1->text();
        QStandardItem* item2 = m_viewItemModel->item(r, 1);
        QString family = item2->text();
        QSize textSize;
        if (layoutIndex == 0) {
            textSize = QSize((textRect.width() - (content.size() - 1) * textHSpace) / content.size(),
                textRect.height());
            LaserHorizontalText* text = new LaserHorizontalText(m_scene->document(), content, textSize, center,
                bold, itatic, false, fill, family, textHSpace, QTransform(), m_viewer->curLayerIndex());
            QRectF pathBounds = text->getPath().boundingRect();
            qreal diffW = textRect.width() - pathBounds.width();
            qreal diffH = textRect.height() - pathBounds.height();
            if (diffW != 0) {
                text->setTextWidth(textSize.width() + (diffW / content.size()));
            }
            if (diffH != 0) {
                text->setTextHeight(textSize.height() + diffH * 0.5);
            }

            stampList.append(text);
        }
        else if (layoutIndex == 1) {
            LaserHorizontalText* text = new LaserHorizontalText(m_scene->document(), content, textSize, center,
                bold, itatic, false, fill, family, 0.0, QTransform(), m_viewer->curLayerIndex());
            stampList.append(text);
        }
        else if (layoutIndex == 2) {

        }
        
    }
    
    
        
    

    m_viewer->addPrimitiveAndExamRegionByBounds(stampList);
}
