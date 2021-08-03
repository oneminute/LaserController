#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QScopedPointer>
#include <QMap>
#include "common/Config.h"
#include "widget/InputWidgetWrapper.h"

class QTreeWidgetItem;
class QAbstractButton;

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
    void setCurrentPanel(const QString& title);

protected:
    void onTreeWidgetCatalogueCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem* previous);
    void onButtonClicked(QAbstractButton* button);

    void setCurrentPanel(QWidget* panel);

    void addConfigItem(ConfigItem* item, QWidget* parent, const QString& exlusion = "");

    //virtual void closeEvent(QCloseEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* e) override;

protected slots:
    void onValueChanged(const QVariant& value);
    void onManufacturePasswordVerified(bool pass);

private:
    QScopedPointer<Ui::ConfigDialog> m_ui;
    QMap<QString, QWidget*> m_panels;
    QList<InputWidgetWrapper*> m_wrappers;
    QString m_windowTitle;
    QWidget* m_systemRegisterPage;
};

#endif // CONFIG_DIALOG