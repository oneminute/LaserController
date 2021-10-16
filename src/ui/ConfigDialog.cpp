#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTableWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <widget/EditSlider.h>

#include "LaserApplication.h"
#include "common/Config.h"
#include "laser/LaserDevice.h"
#include "widget/EditSlider.h"

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ConfigDialog)
    , m_windowTitle(ltr("Config Dialog"))
    , m_systemRegisterPage(nullptr)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 0);
    m_ui->splitter->setStretchFactor(1, 1);

    QList<ConfigItemGroup*> groups;
    groups  
        << Config::General::group
        << Config::Layers::group
        << Config::Ui::group
        << Config::CuttingLayer::group
        << Config::EngravingLayer::group
        << Config::FillingLayer::group
        << Config::PathOptimization::group
        << Config::Export::group
        << Config::Device::group
        << Config::Debug::group
        << Config::UserRegister::group
        //<< Config::SystemRegister::group
        ;

    for (ConfigItemGroup* group : groups)
    {
        QWidget* page = new QWidget(this);
        page->setWindowTitle(group->title());
        m_ui->stackedWidgetPanels->addWidget(page);

        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setMargin(0);
        page->setLayout(pageLayout);

        QGroupBox* groupBox = new QGroupBox(group->title());
        QFormLayout* contentLayout = new QFormLayout(groupBox);
        groupBox->setLayout(contentLayout);
        pageLayout->addWidget(groupBox);

        QTreeWidgetItem* treeItem = new QTreeWidgetItem;
        treeItem->setText(0, group->title());
        m_pages.insert(treeItem, page);
        m_groups.insert(treeItem, group);
        m_groupBoxes.insert(treeItem, groupBox);
        //treeItem->setData(0, Qt::UserRole, QVariant::fromValue<QWidget*>(page));
        //treeItem->setData(0, Qt::UserRole + 1, QVariant::fromValue<ConfigItemGroup*>(group));
        //treeItem->setData(0, Qt::UserRole + 2, QVariant::fromValue<QGroupBox*>(groupBox));
        m_ui->treeWidgetCatalogue->addTopLevelItem(treeItem);

        for (ConfigItem* item : group->items())
        {
            if (item->isVisible())
                addConfigItem(item, groupBox);
        }

        if (group == Config::SystemRegister::group)
        {
            m_systemRegisterPage = page;
        }
    }
    Config::Export::smallDiagonalLimitationItem()->setEnabled(Config::Export::enableSmallDiagonal());

    // 生成系统寄存器面板
    {
        m_systemPage = new QTabWidget(this);
        m_systemRegisterPage = m_systemPage;
        m_systemPage->setWindowTitle(Config::SystemRegister::group->name());
        m_ui->stackedWidgetPanels->addWidget(m_systemPage);

        QWidget* generalPanel = new QWidget(m_systemPage);
        QFormLayout* generalLayout = new QFormLayout(generalPanel);
        generalPanel->setLayout(generalLayout);
        m_systemPage->addTab(generalPanel, tr("General"));

        QWidget* xPanel = new QWidget(m_systemPage);
        QFormLayout* xLayout = new QFormLayout(xPanel);
        xPanel->setLayout(xLayout);
        m_systemPage->addTab(xPanel, tr("X"));

        QWidget* yPanel = new QWidget(m_systemPage);
        QFormLayout* yLayout = new QFormLayout(yPanel);
        yPanel->setLayout(yLayout);
        m_systemPage->addTab(yPanel, tr("Y"));

        QWidget* zPanel = new QWidget(m_systemPage);
        QFormLayout* zLayout = new QFormLayout(zPanel);
        zPanel->setLayout(zLayout);
        m_systemPage->addTab(zPanel, tr("Z"));

        for (ConfigItem* item : Config::SystemRegister::group->items())
        {
            if (!item->isVisible())
                continue;

            if (item->name().startsWith("x"))
                addConfigItem(item, xPanel);
            else if (item->name().startsWith("y"))
                addConfigItem(item, yPanel);
            else if (item->name().startsWith("z"))
                addConfigItem(item, zPanel);
            else
                addConfigItem(item, generalPanel);
        }

        QTreeWidgetItem* treeItem = new QTreeWidgetItem;
        treeItem->setText(0, Config::SystemRegister::group->title());
        //treeItem->setData(0, Qt::UserRole, QVariant::fromValue<QWidget*>(page));
        m_pages.insert(treeItem, m_systemPage);
        m_groups.insert(treeItem, Config::SystemRegister::group);
        m_groupBoxes.insert(treeItem, nullptr);
        m_ui->treeWidgetCatalogue->addTopLevelItem(treeItem);
    }

    m_ui->stackedWidgetPanels->setCurrentIndex(0);
    m_ui->treeWidgetCatalogue->setCurrentItem(m_ui->treeWidgetCatalogue->topLevelItem(0));

    connect(m_ui->treeWidgetCatalogue, &QTreeWidget::currentItemChanged, this, &ConfigDialog::onTreeWidgetCatalogueCurrentItemChanged);
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &ConfigDialog::onButtonClicked);
    connect(LaserApplication::device, &LaserDevice::manufacturePasswordVerified, this, &ConfigDialog::onManufacturePasswordVerified);
    connect(LaserApplication::app, &LaserApplication::languageChanged, this, &ConfigDialog::retranslate);

    setWindowTitle(m_windowTitle);
    LaserApplication::device->readUserRegisters();
    LaserApplication::device->readSystemRegisters();

    retranslate();
}

ConfigDialog::~ConfigDialog()
{
}

bool ConfigDialog::isModified()
{
    bool modified = false;
    for (QList<InputWidgetWrapper*>::ConstIterator i = m_wrappers.constBegin(); i != m_wrappers.constEnd(); i++)
    {
        if ((*i)->isModified())
        {
            modified = true;
            break;
        }
    }
    return modified;
}

void ConfigDialog::onTreeWidgetCatalogueCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    setCurrentPanel(m_pages[current]);
}

void ConfigDialog::onButtonClicked(QAbstractButton * button)
{
    QDialogButtonBox::StandardButton stdButton = m_ui->buttonBox->standardButton(button);
    if (stdButton == QDialogButtonBox::Reset)
    {
        for (QList<InputWidgetWrapper*>::ConstIterator i = m_wrappers.constBegin(); i != m_wrappers.constEnd(); i++)
        {
            if ((*i)->isModified())
            {
                (*i)->reset();
            }
        }
    }
    else if (stdButton == QDialogButtonBox::RestoreDefaults)
    {
        for (QList<InputWidgetWrapper*>::ConstIterator i = m_wrappers.constBegin(); i != m_wrappers.constEnd(); i++)
        {
            if ((*i)->isModified())
            {
                (*i)->restoreDefault();
            }
        }
    }
    else if (stdButton == QDialogButtonBox::Save)
    {
        if (Config::UserRegister::group->isModified())
            LaserApplication::device->writeUserRegisters();
        if (Config::SystemRegister::group->isModified())
        {
            // show password input dialog
            QString password = QInputDialog::getText(
                this,
                tr("Manufacture Password"),
                tr("Password"),
                QLineEdit::Normal
            );
            Config::SystemRegister::passwordItem()->setValue(password, MB_Manual);
            LaserApplication::device->writeSystemRegisters();
        }
        Config::save(LaserApplication::device->mainCardId());
        onValueChanged(QVariant());
    }
}

void ConfigDialog::setCurrentPanel(QWidget * panel)
{
    m_ui->stackedWidgetPanels->setCurrentWidget(panel);
    m_ui->scrollAreaConfigs->verticalScrollBar()->setValue(0);
}

void ConfigDialog::setCurrentPanel(const QString & title)
{
    QList<QTreeWidgetItem*> items = m_ui->treeWidgetCatalogue->findItems(title, Qt::MatchExactly);
    m_ui->treeWidgetCatalogue->setCurrentItem(items[0]);
}

void ConfigDialog::addConfigItem(ConfigItem * item, QWidget* parent, const QString& exlusion)
{
    QFormLayout * layout = qobject_cast<QFormLayout*>(parent->layout());

    QWidget* widget = InputWidgetWrapper::createWidget(item, Qt::Horizontal);

    if (!widget)
        return;

    widget->setParent(parent);
    QLabel* labelName = new QLabel(parent);

    layout->addRow(labelName, widget);

    InputWidgetWrapper* wrapper = item->bindWidget(widget);
    wrapper->setNameLabel(labelName);
    m_wrappers.append(wrapper);
}

void ConfigDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(e);
}

void ConfigDialog::onManufacturePasswordVerified(bool pass)
{
    if (pass)
    {
        LaserApplication::device->readSystemRegisters();
        m_ui->stackedWidgetPanels->setCurrentWidget(m_systemRegisterPage);
        m_ui->scrollAreaConfigs->verticalScrollBar()->setValue(0);
    }
    else
    {
        QMessageBox::warning(this, tr("Invalid Manufacture Password"), tr("Invalid manufacture password!"));
    }
}

void ConfigDialog::retranslate()
{
    for (InputWidgetWrapper* wrapper : m_wrappers)
    {
        wrapper->retranslate();
    }
    m_ui->retranslateUi(this);
    for (int i = 0; i < m_ui->treeWidgetCatalogue->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_ui->treeWidgetCatalogue->topLevelItem(i);
        QWidget* page = m_pages[item];
        ConfigItemGroup* group = m_groups[item];
        QGroupBox* groupBox = m_groupBoxes[item];
        item->setText(0, group->title());
        page->setWindowTitle(group->title());
        if (groupBox)
            groupBox->setTitle(group->title());
    }

    m_systemPage->setTabText(0, tr("General"));
    m_systemPage->setTabText(1, tr("X"));
    m_systemPage->setTabText(2, tr("Y"));
    m_systemPage->setTabText(3, tr("Z"));

    m_ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("Restore Defaults"));
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Reset"));
}

void ConfigDialog::onValueChanged(const QVariant& value)
{
    if (isModified())
    {
        this->setWindowTitle(m_windowTitle + " *");
    }
    else
    {
        this->setWindowTitle(m_windowTitle);
    }
}
