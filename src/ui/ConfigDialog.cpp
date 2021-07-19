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
#include <QTreeWidgetItem>
#include <widget/EditSlider.h>

#include "LaserApplication.h"
#include "common/Config.h"
#include "laser/LaserDevice.h"
#include "widget/EditSlider.h"

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ConfigDialog)
    , m_windowTitle(tr("Config Dialog"))
    , m_systemRegisterPage(nullptr)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 0);
    m_ui->splitter->setStretchFactor(1, 1);

    QList<ConfigItemGroup*> groups;
    groups  
        //<< Config::General::group
        //<< Config::Layers::group
        << Config::Ui::group
        //<< Config::CuttingLayer::group
        //<< Config::EngravingLayer::group
        //<< Config::PathOptimization::group
        //<< Config::Export::group
        //<< Config::Device::group
        //<< Config::UserRegister::group
        //<< Config::SystemRegister::group
        ;

    for (ConfigItemGroup* group : groups)
    {
        QWidget* page = new QWidget(this);
        page->setWindowTitle(group->name());
        m_ui->stackedWidgetPanels->addWidget(page);

        QVBoxLayout* pageLayout = new QVBoxLayout(page);
        pageLayout->setMargin(0);
        page->setLayout(pageLayout);

        QGroupBox* groupBox = new QGroupBox(group->title());
        QGridLayout* gridLayout = new QGridLayout(groupBox);
        gridLayout->setColumnMinimumWidth(0, 150);
        gridLayout->setColumnMinimumWidth(1, 240);
        gridLayout->setColumnMinimumWidth(2, 240);
        gridLayout->setColumnStretch(0, 0);
        gridLayout->setColumnStretch(1, 1);
        gridLayout->setColumnStretch(2, 0);
        groupBox->setLayout(gridLayout);
        pageLayout->addWidget(groupBox);

        QTreeWidgetItem* treeItem = new QTreeWidgetItem;
        treeItem->setText(0, group->title());
        treeItem->setData(0, Qt::UserRole, QVariant::fromValue<QWidget*>(page));
        m_ui->treeWidgetCatalogue->addTopLevelItem(treeItem);

        for (ConfigItem* item : group->items())
        {
            addConfigItem(item, groupBox);
        }
        int row = gridLayout->rowCount();
        gridLayout->addWidget(new QWidget, row, 0);
        gridLayout->setRowStretch(row, 1);

        if (group == Config::SystemRegister::group)
        {
            m_systemRegisterPage = page;
        }
    }
    m_ui->stackedWidgetPanels->setCurrentIndex(0);
    m_ui->treeWidgetCatalogue->setCurrentItem(m_ui->treeWidgetCatalogue->topLevelItem(0));

    connect(m_ui->treeWidgetCatalogue, &QTreeWidget::currentItemChanged, this, &ConfigDialog::onTreeWidgetCatalogueCurrentItemChanged);
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &ConfigDialog::onButtonClicked);
    connect(LaserApplication::device, &LaserDevice::manufacturePasswordVerified, this, &ConfigDialog::onManufacturePasswordVerified);

    setWindowTitle(m_windowTitle);
    LaserApplication::device->readUserRegisters();
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
    setCurrentPanel(current->data(0, Qt::UserRole).value<QWidget*>());
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
        Config::save(LaserApplication::device->mainCardId());
        LaserApplication::device->writeUserRegisters();
        LaserApplication::device->writeSystemRegisters();
        onValueChanged(QVariant());
    }
}

void ConfigDialog::setCurrentPanel(QWidget * panel)
{
    /*if (panel == m_systemRegisterPage)
    {
        QString password = QInputDialog::getText(
            this,
            tr("Manufacture Password"),
            tr("Password"),
            QLineEdit::Normal
        );

        password = password.trimmed();
        if (password.isEmpty() || password.isNull())
        {
            return;
        }

        LaserApplication::device->verifyManufacturePassword(password);
    }
    else
    {
        m_ui->stackedWidgetPanels->setCurrentWidget(panel);
        m_ui->scrollAreaConfigs->verticalScrollBar()->setValue(0);
    }*/
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
    QGridLayout * layout = qobject_cast<QGridLayout*>(parent->layout());

    int row = layout->rowCount();

    QWidget* widget = InputWidgetWrapper::createWidget(item->inputWidgetType(), Qt::Horizontal);

    if (!widget)
        return;
    for (QMap<QString, QVariant>::ConstIterator i = item->inputWidgetProperties().constBegin(); i != item->inputWidgetProperties().constEnd(); i++)
    {
        widget->setProperty(i.key().toStdString().c_str(), i.value());
    }
    item->initWidget(widget);
    layout->addWidget(widget, row, 1);
    widget->setParent(parent);

    QLabel* labelName = new QLabel(parent);
    labelName->setText(item->title());
    layout->addWidget(labelName, row, 0);

    QLabel* labelDesc = new QLabel(parent);
    labelDesc->setText(item->description());
    layout->addWidget(labelDesc, row, 2);
    layout->setRowStretch(row, 0);

    InputWidgetWrapper* wrapper = item->createInputWidgetWrapper(widget);
    wrapper->setNameLabel(labelName);
    wrapper->setDescriptionLabel(labelDesc);
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
