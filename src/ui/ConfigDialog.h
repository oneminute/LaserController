#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QScopedPointer>
#include <QTimer>
#include <QMap>
#include "common/Config.h"
#include "widget/InputWidgetWrapper.h"

class QTreeWidgetItem;
class QAbstractButton;
class QGroupBox;
class QTabWidget;

namespace Ui
{
    class ConfigDialog;
}

class ConfigDialog: public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget* parent = nullptr);
    virtual ~ConfigDialog();

    bool isModified();
    bool isDirty();
    void setCurrentPanel(const QString& title);

    void restoreToDefault();
    void restoreToSystemDefault();
    void reset();
    void applyToDefault();
    void save();
    void load();
    void apply();

protected:
    void onTreeWidgetCatalogueCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem* previous);

    void setCurrentPanel(QWidget* panel);

    void addConfigItem(ConfigItem* item, QWidget* parent, const QString& exlusion = "");

    void updatePanelsStatus();

    virtual void closeEvent(QCloseEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* e) override;

    void handlePasswordError();

    void showPasswordWidgets(bool show);

protected slots:
    void updateTitle(const QVariant& value = QVariant());
    void onDeviceConnected();
    void onDeviceDisconnected();
    void onSystemRegistersConfirmed();
    void onUserRegistersConfirmed();

    void onButtonRestoreToSystemDefault(bool checked);
    void onButtonRestoreToDefault(bool checked);
    void onButtonApplyToDefault(bool checked);
    void onButtonImport(bool checked);
    void onButtonExport(bool checked);
    void onButtonSave(bool checked);
    void onButtonReset(bool checked);
    void onButtonReload(bool checked);
    void onButtonSaveAndClose(bool checked);
    void onButtonResetAndClose(bool checked);

    void onButtonPasswordClicked(bool checked);
    void onCheckBoxChangePasswordStateChanged(int state);
    void onButtonChangedPasswordClicked(bool checked);
    void onPasswordTimerTimeout();

    void onPasswordChangeOk();
    void onPasswordChangeFailed();

    void retranslate();

    void onConfigItemUpdated();

private:
    QScopedPointer<Ui::ConfigDialog> m_ui;
    QMap<QString, QWidget*> m_panels;
    QList<InputWidgetWrapper*> m_wrappers;
    QString m_windowTitle;
    QWidget* m_systemRegisterPage;
    QWidget* m_userRegisterPage;
    QMap<QTreeWidgetItem*, QWidget*> m_pages;
    QMap<QTreeWidgetItem*, ConfigItemGroup*> m_groups;
    QMap<QTreeWidgetItem*, QGroupBox*> m_groupBoxes;
    QTabWidget* m_systemPage;
    QWidget* m_passwordPage;
    QLabel* m_labelPassword;
    QLabel* m_labelNewPassword;
    QLabel* m_labelConfirm;
    QPushButton* m_buttonPassword;
    QPushButton* m_buttonChangePassword;
    QLineEdit* m_editPassword;
    QLineEdit* m_editNewPassword;
    QLineEdit* m_editConfirm;
    QCheckBox* m_checkBoxChangePassword;
    bool m_done;
    bool m_closing;
    bool m_needUserRegisterConfirm;
    bool m_needSystemRegisterConfirm;
    int m_errorCount;
    bool m_changingPassword;
    QDateTime m_lastUserInformTime;
    QDateTime m_lastSysInformTime;

    QTimer m_passwordTimer;
};

#endif // CONFIG_DIALOG