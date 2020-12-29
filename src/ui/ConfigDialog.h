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

protected:
    void onTreeWidgetCatalogueCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem* previous);
    void onButtonClicked(QAbstractButton* button);

    void setCurrentPanel(QWidget* panel);
    void setCurrentPanel(const QString& title);

    void addConfigItem(QWidget* widget, Config::ConfigItem* item, QWidget* parent, const QString& exlusion = "");

protected slots:
    void onValueChanged(const QVariant& value);

private:
    QScopedPointer<Ui::ConfigDialog> m_ui;
    QMap<QString, QWidget*> m_panels;
    QList<InputWidgetWrapper*> m_wrappers;
    QString m_windowTitle;
};

#endif // CONFIG_DIALOG