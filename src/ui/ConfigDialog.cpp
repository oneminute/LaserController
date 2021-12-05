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
#include <QFileDialog>
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
    , m_done(false)
    , m_closing(false)
    , m_needUserRegisterConfirm(false)
    , m_needSystemRegisterConfirm(false)
    , m_changingPassword(false)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 0);
    m_ui->splitter->setStretchFactor(1, 1);

    QList<ConfigItemGroup*> groups;
    groups  
        << Config::General::group
        //<< Config::Layers::group
        << Config::Ui::group
        << Config::CuttingLayer::group
        << Config::EngravingLayer::group
        << Config::FillingLayer::group
        << Config::PathOptimization::group
        << Config::Export::group
        << Config::Device::group
#ifdef _DEBUG
        << Config::Debug::group
#endif
        << Config::UserRegister::group
        //<< Config::SystemRegister::group
        ;

    for (ConfigItemGroup* group : groups)
    {
        QWidget* page = new QWidget(this);
        if (group == Config::UserRegister::group)
            m_userRegisterPage = page;
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
        treeItem->setData(0, Qt::UserRole, QVariant::fromValue<ConfigItemGroup*>(group));
        m_pages.insert(treeItem, page);
        m_groups.insert(treeItem, group);
        m_groupBoxes.insert(treeItem, groupBox);

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
        treeItem->setData(0, Qt::UserRole, QVariant::fromValue<ConfigItemGroup*>(Config::SystemRegister::group));
        m_pages.insert(treeItem, m_systemPage);
        m_groups.insert(treeItem, Config::SystemRegister::group);
        m_groupBoxes.insert(treeItem, nullptr);
        m_ui->treeWidgetCatalogue->addTopLevelItem(treeItem);
    }

    Config::UserRegister::group->setPreSaveHook(
        [=]() {
            if (Config::UserRegister::group->isModified())
                return LaserApplication::device->writeUserRegisters();
            return false;
        }
    );
    Config::SystemRegister::group->setPreSaveHook(
        [=]() {
            if (Config::SystemRegister::group->isModified())
            {
                // show password input dialog
                QString password = QInputDialog::getText(
                    this,
                    tr("Manufacture Password"),
                    tr("Password"),
                    QLineEdit::Normal
                );
                if (password.isEmpty())
                    return false;
                return LaserApplication::device->writeSystemRegisters(password);
            }
            return false;
        }
    );

    m_ui->stackedWidgetPanels->setCurrentIndex(0);
    m_ui->treeWidgetCatalogue->setCurrentItem(m_ui->treeWidgetCatalogue->topLevelItem(0));

    // 创建密码相关的面板
    m_passwordPage = new QWidget(this);
    QGridLayout* passwordLayout = new QGridLayout;
    m_labelPassword = new QLabel(m_passwordPage);
    if (Config::General::language() == QLocale::Chinese)
        m_labelPassword->setMinimumWidth(60);
    else
        m_labelPassword->setMinimumWidth(80);
    m_labelNewPassword = new QLabel(m_passwordPage);
    m_labelConfirm = new QLabel(m_passwordPage);
    m_buttonPassword = new QPushButton(m_passwordPage);
    m_buttonChangePassword = new QPushButton(m_passwordPage);
    m_editPassword = new QLineEdit(m_passwordPage);
    m_editNewPassword = new QLineEdit(m_passwordPage);
    m_editConfirm = new QLineEdit(m_passwordPage);
    m_checkBoxChangePassword = new QCheckBox(m_passwordPage);
    passwordLayout->addWidget(m_labelPassword, 0, 0);
    passwordLayout->addWidget(m_editPassword, 0, 1);
    passwordLayout->addWidget(m_buttonPassword, 0, 2);
    passwordLayout->addWidget(m_checkBoxChangePassword, 1, 0, 1, 3);
    passwordLayout->addWidget(m_labelNewPassword, 2, 0);
    passwordLayout->addWidget(m_editNewPassword, 2, 1);
    passwordLayout->addWidget(m_labelConfirm, 3, 0);
    passwordLayout->addWidget(m_editConfirm, 3, 1);
    passwordLayout->addWidget(m_buttonChangePassword, 3, 2);
    passwordLayout->setColumnStretch(0, 0);
    passwordLayout->setColumnStretch(1, 1);
    passwordLayout->setColumnStretch(2, 0);
    passwordLayout->setRowStretch(4, 1);
    m_passwordPage->setLayout(passwordLayout);
    m_ui->stackedWidgetPanels->addWidget(m_passwordPage);

    connect(m_ui->treeWidgetCatalogue, &QTreeWidget::currentItemChanged, this, &ConfigDialog::onTreeWidgetCatalogueCurrentItemChanged);
    connect(LaserApplication::device, &LaserDevice::connected, this, &ConfigDialog::onDeviceConnected);
    connect(LaserApplication::device, &LaserDevice::disconnected, this, &ConfigDialog::onDeviceDisconnected);
    connect(LaserApplication::device, &LaserDevice::userRegistersConfirmed, this, &ConfigDialog::onUserRegistersConfirmed);
    connect(LaserApplication::device, &LaserDevice::systemRegistersConfirmed, this, &ConfigDialog::onSystemRegistersConfirmed);
    connect(LaserApplication::app, &LaserApplication::languageChanged, this, &ConfigDialog::retranslate);
    connect(m_ui->pushButtonRstoreToSystemDefault, &QPushButton::clicked,
        this, &ConfigDialog::onButtonRestoreToSystemDefault);
    connect(m_ui->pushButtonRestoreToDefault, &QPushButton::clicked,
        this, &ConfigDialog::onButtonRestoreToDefault);
    connect(m_ui->pushButtonApplyToDefault, &QPushButton::clicked,
        this, &ConfigDialog::onButtonApplyToDefault);
    connect(m_ui->pushButtonImport, &QPushButton::clicked,
        this, &ConfigDialog::onButtonImport);
    connect(m_ui->pushButtonExport, &QPushButton::clicked,
        this, &ConfigDialog::onButtonExport);
    connect(m_ui->pushButtonSave, &QPushButton::clicked,
        this, &ConfigDialog::onButtonSave);
    connect(m_ui->pushButtonReset, &QPushButton::clicked,
        this, &ConfigDialog::onButtonReset);
    connect(m_ui->pushButtonReload, &QPushButton::clicked,
        this, &ConfigDialog::onButtonReload);
    connect(m_ui->pushButtonSaveClose, &QPushButton::clicked,
        this, &ConfigDialog::onButtonSaveAndClose);
    connect(m_ui->pushButtonResetClose, &QPushButton::clicked,
        this, &ConfigDialog::onButtonResetAndClose);
    connect(m_buttonPassword, &QPushButton::clicked, this, &ConfigDialog::onButtonPasswordClicked);
    connect(m_buttonChangePassword, &QPushButton::clicked, this, &ConfigDialog::onButtonChangedPasswordClicked);
    connect(m_checkBoxChangePassword, &QCheckBox::stateChanged, this, &ConfigDialog::onCheckBoxChangePasswordStateChanged);
    connect(&m_passwordTimer, &QTimer::timeout, this, &ConfigDialog::onPasswordTimerTimeout);
    connect(LaserApplication::device, &LaserDevice::manufacturePasswordChangeOk,
        this, &ConfigDialog::onPasswordChangeOk);
    connect(LaserApplication::device, &LaserDevice::manufacturePasswordChangeFailed,
        this, &ConfigDialog::onPasswordChangeFailed);

    setWindowTitle(m_windowTitle);
    LaserApplication::device->readUserRegisters();
    LaserApplication::device->readSystemRegisters();
    updatePanelsStatus();

    m_errorCount = 0;
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
        if ((*i)->configItem()->isModified())
        {
            modified = true;
            break;
        }
    }
    return modified;
}

bool ConfigDialog::isDirty()
{
    bool dirty = false;
    for (QList<InputWidgetWrapper*>::ConstIterator i = m_wrappers.constBegin(); i != m_wrappers.constEnd(); i++)
    {
        if ((*i)->configItem()->isDirty())
        {
            dirty = true;
            break;
        }
    }
    return dirty;
}

void ConfigDialog::onTreeWidgetCatalogueCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    ConfigItemGroup* group = current->data(0, Qt::UserRole).value<ConfigItemGroup*>();
    if (group == Config::SystemRegister::group)
    {
        // 判断当前密码验证计时器是否正在工作
        if (m_passwordTimer.isActive())
        {
            m_ui->stackedWidgetPanels->setCurrentWidget(m_systemPage);
            m_passwordTimer.start();
        }
        else
        {
            m_ui->stackedWidgetPanels->setCurrentWidget(m_passwordPage);
            showPasswordWidgets(false);
        }
    }
    else
    {
        setCurrentPanel(m_pages[current]);
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

void ConfigDialog::restoreToDefault()
{
    for (ConfigItemGroup* group : Config::groups)
    {
        for (ConfigItem* item : group->items())
        {
            item->restoreToDefault();
        }
    }
}

void ConfigDialog::restoreToSystemDefault()
{
    for (ConfigItemGroup* group : Config::groups)
    {
        for (ConfigItem* item : group->items())
        {
            item->restoreToSystemDefault();
        }
    }
}

void ConfigDialog::reset()
{
    for (ConfigItemGroup* group : Config::groups)
    {
        for (ConfigItem* item : group->items())
        {
            item->reset();
        }
    }
}

void ConfigDialog::applyToDefault()
{
    for (ConfigItemGroup* group : Config::groups)
    {
        for (ConfigItem* item : group->items())
        {
            item->applyToDefault();
        }
    }
}

void ConfigDialog::save()
{
    bool needRelaunch = false;
    for (ConfigItemGroup* group : Config::groups)
    {
        if (group->needRelaunch())
        {
            needRelaunch = true;
            break;
        }
    }
    Config::save(false, false);
    updateTitle();

    if (needRelaunch)
    {
        QMessageBox dlg(QMessageBox::Question, tr("Need restart"), tr("The modification you are currently making needs to restart the software immediately. Do you want to restart the software now?"),
            QMessageBox::Yes | QMessageBox::No);
        dlg.setButtonText(QMessageBox::Yes, tr("Yes"));
        dlg.setButtonText(QMessageBox::No, tr("No"));
        int result = dlg.exec();
        if (result == QMessageBox::StandardButton::Yes)
            LaserApplication::restart();
    }
}

void ConfigDialog::load()
{
    Config::load();
    updateTitle();
}

void ConfigDialog::apply()
{
    for (ConfigItemGroup* group : Config::groups)
    {
        for (ConfigItem* item : group->items())
        {
            item->apply();
        }
    }
}

void ConfigDialog::addConfigItem(ConfigItem * item, QWidget* parent, const QString& exlusion)
{
    QFormLayout * layout = qobject_cast<QFormLayout*>(parent->layout());

    QWidget* widget = InputWidgetWrapper::createWidget(item, Qt::Horizontal);

    if (!widget)
        return;

    if (item->name() == "xStepLength")
        qLogD << item->name() << ", " << item->value();
    InputWidgetWrapper* wrapper = item->bindWidget(widget, SS_NORMAL);
    connect(wrapper, &InputWidgetWrapper::updated, this, &ConfigDialog::onConfigItemUpdated);
    widget->setParent(parent);
    QLabel* labelName = new QLabel(parent);
    layout->addRow(labelName, widget);

    wrapper->setNameLabel(labelName);
    m_wrappers.append(wrapper);
}

void ConfigDialog::updatePanelsStatus()
{
    if (LaserApplication::device->isConnected())
    {
        m_systemRegisterPage->setEnabled(true);
        m_userRegisterPage->setEnabled(true);
    }
    else
    {
        m_systemRegisterPage->setEnabled(false);
        m_userRegisterPage->setEnabled(false);
    }
}

void ConfigDialog::closeEvent(QCloseEvent* e)
{
    if (!m_done)
    {
        reset();
    }
}

void ConfigDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(e);
}

void ConfigDialog::handlePasswordError()
{
    m_errorCount++;
    // 验证失败
    QMessageBox dlg(QMessageBox::Warning, tr("Wrong password"), tr(""), QMessageBox::Ok);
    dlg.setButtonText(QMessageBox::Ok, tr("Ok"));
    if (m_errorCount < 3)
    {
        dlg.setText(tr("Wrong password! You have %1 chances to check password.").arg(3 - m_errorCount));
    }
    else
    {
        dlg.setText(tr("Too many incorrect passwords, please restart the software and try again."));
    }
    dlg.exec();
}

void ConfigDialog::showPasswordWidgets(bool show)
{
    m_labelNewPassword->setVisible(show);
    m_labelConfirm->setVisible(show);
    m_editNewPassword->setVisible(show);
    m_editConfirm->setVisible(show);
    m_buttonChangePassword->setVisible(show);
}

void ConfigDialog::onDeviceConnected()
{
    updatePanelsStatus();
}

void ConfigDialog::onDeviceDisconnected()
{
    updatePanelsStatus();
}

void ConfigDialog::onSystemRegistersConfirmed()
{
    QStringList errors;
    bool success = true;
    QMap<int, LaserRegister*> registers = LaserApplication::device->systemRegisters(true);
    for (QMap<int, LaserRegister*>::Iterator i = registers.begin(); i != registers.end(); i++)
    {
        LaserRegister* laserRegister = i.value();
        ConfigItem* configItem = laserRegister->configItem();

        if (!configItem->confirm(laserRegister->value()))
        {
            errors.append(tr("Register '%1' save failure, expected value is '%2', actual value is '%3'.")
            .arg(laserRegister->name()).arg(configItem->value().toString()).arg(laserRegister->value().toString()));
            success = false;
        }
    }
    if (success)
    {
        QMessageBox::information(this, tr("Success"), tr("Save system registers successfully!"));
        Config::SystemRegister::group->save(true, true);
    }
    else
    {
        errors.insert(0, tr("Save system registers failure:"));
        QString info = errors.join("\n");
        QMessageBox::warning(this, tr("Failure"), info);
    }

    m_needSystemRegisterConfirm = false;
    if (success && m_closing)
    {
        if (!m_needSystemRegisterConfirm && !m_needUserRegisterConfirm)
            this->close();
    }
}

void ConfigDialog::onUserRegistersConfirmed()
{
    QStringList errors;
    bool success = true;
    QMap<int, LaserRegister*> registers = LaserApplication::device->userRegisters(true);
    for (QMap<int, LaserRegister*>::Iterator i = registers.begin(); i != registers.end(); i++)
    {
        LaserRegister* laserRegister = i.value();
        ConfigItem* configItem = laserRegister->configItem();

        if (!configItem->confirm(laserRegister->value()))
        {
            errors.append(tr("Register '%1' save failure, expected value is '%2', actual value is '%3'.")
            .arg(laserRegister->name()).arg(configItem->value().toString()).arg(laserRegister->value().toString()));
            success = true;
        }
    }
    if (success)
    {
        QMessageBox::information(this, tr("Success"), tr("Save user registers successfully!"));
        Config::UserRegister::group->save(true, true);
    }
    else
    {
        errors.insert(0, tr("Save user registers failure:"));
        QString info = errors.join("\n");
        QMessageBox::warning(this, tr("Failure"), info);
    }
    
    m_needUserRegisterConfirm = false;
    if (success && m_closing)
    {
        if (!m_needSystemRegisterConfirm && !m_needUserRegisterConfirm)
            this->close();
    }
}

void ConfigDialog::onButtonRestoreToSystemDefault(bool checked)
{
    restoreToSystemDefault();
}

void ConfigDialog::onButtonRestoreToDefault(bool checked)
{
    restoreToDefault();
}

void ConfigDialog::onButtonApplyToDefault(bool checked)
{
    applyToDefault();
}

void ConfigDialog::onButtonImport(bool checked)
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Settings"), QString(),
        "Config (*.config *.json)");
    if (filename.isEmpty() || filename.isNull())
        return;
    Config::importFrom(filename);
    QString password = QInputDialog::getText(
        this,
        tr("Manufacture Password"),
        tr("Password"),
        QLineEdit::Normal
    );
    LaserApplication::device->writeSystemRegisters(password);
    LaserApplication::device->writeUserRegisters();
}

void ConfigDialog::onButtonExport(bool checked)
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Settings"), QString(),
        "Config ((*.config *.json)");
    if (filename.isEmpty() || filename.isNull())
        return;
    Config::exportTo(filename);
}

void ConfigDialog::onButtonSave(bool checked)
{
    save();
    m_done = true;
}

void ConfigDialog::onButtonReset(bool checked)
{
    // 提示用户当前修改会丢失
    if (QMessageBox::question(this, tr("Confirm"),
        tr("If you have modified settings, clicked 'OK' button will lose these modifications. Do you want to reset?"),
        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        reset();
    }
    m_done = true;
}

void ConfigDialog::onButtonReload(bool checked)
{
    // 提示用户当前修改会被覆盖
    if (QMessageBox::question(this, tr("Confirm"),
        tr("If you have modified settings, clicked 'OK' button will lose these modifications. Do you want to reset?"),
        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        load();
    }
}

void ConfigDialog::onButtonSaveAndClose(bool checked)
{
    save();
    m_done = true;
    m_closing = true;
    if (Config::SystemRegister::group->isModified())
        m_needSystemRegisterConfirm = true;
    if (Config::UserRegister::group->isModified())
        m_needUserRegisterConfirm = true;
    if (!m_needUserRegisterConfirm && !m_needSystemRegisterConfirm)
        this->close();
}

void ConfigDialog::onButtonResetAndClose(bool checked)
{
    reset();
    m_done = true;
    this->close();
}

void ConfigDialog::onButtonPasswordClicked(bool checked)
{
    if (LaserApplication::device->verifyManufacturePassword(m_editPassword->text(), m_errorCount))
    {
        // 验证成功
        m_passwordTimer.start(10000);
        m_ui->stackedWidgetPanels->setCurrentWidget(m_systemPage);
        m_editPassword->clear();
        m_errorCount = 0;
    }
    else
    {
        handlePasswordError();
    }
}

void ConfigDialog::onCheckBoxChangePasswordStateChanged(int state)
{
    showPasswordWidgets(m_checkBoxChangePassword->isChecked());
}

void ConfigDialog::onButtonChangedPasswordClicked(bool checked)
{
    if (LaserApplication::device->verifyManufacturePassword(m_editPassword->text(), m_errorCount))
    {
        // 验证成功
        m_errorCount = 0;
        QMessageBox dlg(QMessageBox::Warning, tr("Failed"), "", QMessageBox::Ok);
        dlg.setButtonText(QMessageBox::Ok, tr("Ok"));
        if (m_editNewPassword->text().length() < 6 || m_editConfirm->text().length() < 6)
        {
            dlg.setText(tr("Failed to modify the factory password, the password or confirmation password cannot be less than 6 characters!"));
            dlg.exec();
            return;
        }
        if (m_editNewPassword->text() == m_editConfirm->text())
        {
            if (LaserApplication::device->changeManufacturePassword(
                m_editPassword->text(),
                m_editNewPassword->text()
            ))
            {
                m_changingPassword = true;
            }
            else
            {
                dlg.setText(tr("Failed to modify the factory password, please confirm that the device is connected normally."));
                dlg.exec();
                return;
            }
        }
        else
        {
            dlg.setText(tr("The new password dose not match the confirmed password!"));
            dlg.exec();
            return;
        }
    }
    else
    {
        handlePasswordError();
    }
}

void ConfigDialog::onPasswordTimerTimeout()
{
    m_passwordTimer.stop();
}

void ConfigDialog::onPasswordChangeOk()
{
    if (!m_changingPassword)
        return;

    QMessageBox dlg(QMessageBox::Information, tr("Successful"), tr("Successfully change password!"),
        QMessageBox::Ok);
    dlg.setButtonText(QMessageBox::Ok, tr("Ok"));
    dlg.exec();
    m_changingPassword = false;

    showPasswordWidgets(false);
    m_editPassword->setText("");
}

void ConfigDialog::onPasswordChangeFailed()
{
    if (!m_changingPassword)
        return;

    QMessageBox dlg(QMessageBox::Warning, tr("Failed"), tr("Failed to modify password!"), QMessageBox::Ok);
    dlg.setButtonText(QMessageBox::Ok, tr("Ok"));
    dlg.exec();
    m_changingPassword = false;
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

    m_labelPassword->setText(tr("Password"));
    m_labelNewPassword->setText(tr("New Password"));
    m_labelConfirm->setText(tr("Confirm Password"));
    m_buttonPassword->setText(tr("OK"));
    m_buttonChangePassword->setText(tr("Change Password"));
    m_checkBoxChangePassword->setText(tr("Change Password"));
}

void ConfigDialog::onConfigItemUpdated()
{
    for (int i = 0; i < m_ui->treeWidgetCatalogue->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_ui->treeWidgetCatalogue->topLevelItem(i);
        ConfigItemGroup* group = m_groups[item];
        if (group->isModified())
        {
            item->setTextColor(0, Qt::red);
        }
        else
        {
            item->setTextColor(0, Qt::black);
        }
    }
}

void ConfigDialog::updateTitle(const QVariant& value)
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
