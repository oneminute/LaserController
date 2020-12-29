#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <widget/EditSlider.h>
#include <QLabel>
#include <QRegularExpression>
#include <QAbstractButton>

#include "common/Config.h"

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ConfigDialog)
    , m_windowTitle(tr("Config Dialog"))
{
    m_ui->setupUi(this);
    m_panels.insert("General", m_ui->pageGeneral);
    m_panels.insert("Ui", m_ui->pageUi);
    m_panels.insert("Layers", m_ui->pageLayers);

    m_ui->splitter->setStretchFactor(0, 0);
    m_ui->splitter->setStretchFactor(1, 1);

    for (QMap<QString, QWidget*>::ConstIterator i = m_panels.constBegin(); i != m_panels.constEnd(); i++)
    {
        QTreeWidgetItem* treeItem = new QTreeWidgetItem;
        treeItem->setText(0, i.key());
        treeItem->setData(0, Qt::UserRole, QVariant::fromValue<QWidget*>(i.value()));
        m_ui->treeWidgetCatalogue->addTopLevelItem(treeItem);
    }

    connect(m_ui->treeWidgetCatalogue, &QTreeWidget::currentItemChanged, this, &ConfigDialog::onTreeWidgetCatalogueCurrentItemChanged);
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &ConfigDialog::onButtonClicked);

    // setup general config items
    QComboBox* comboBoxLanguages = new QComboBox;
    comboBoxLanguages->addItem(tr("English"));
    comboBoxLanguages->addItem(tr("Chinese"));
    int languageItem = comboBoxLanguages->findText(Config::GeneralLanguage());
    if (languageItem == -1)
    {
        QMessageBox::warning(this, tr("Config Error"), QString(tr("The language \"%1\" reading from config file can not be recognized. Please check your config file.")).arg(Config::GeneralLanguage()));
    }
    else
    {
        comboBoxLanguages->setCurrentIndex(languageItem);
    }
    addConfigItem(comboBoxLanguages, Config::GeneralLanguageItem(), m_ui->groupBoxInternationalization);

    // setup layers
    EditSlider* editSliderMaxLayersCount = new EditSlider;
    editSliderMaxLayersCount->setMaximum(32);
    addConfigItem(editSliderMaxLayersCount, Config::LayersMaxLayersCountItem(), m_ui->groupBoxLayers);

    // setup color buttons
    EditSlider* editSliderColorButtonWidth = new EditSlider;
    editSliderColorButtonWidth->setMaximum(60);
    addConfigItem(editSliderColorButtonWidth, Config::UIColorButtonWidthItem(), m_ui->groupBoxColorButtons, "Color");

    EditSlider* editSliderColorButtonHeight = new EditSlider;
    editSliderColorButtonHeight->setMaximum(60);
    addConfigItem(editSliderColorButtonHeight, Config::UIColorButtonHeightItem(), m_ui->groupBoxColorButtons, "Color");

    // setup tool buttons
    EditSlider* editSliderToolButtonSize = new EditSlider;
    editSliderToolButtonSize->setMaximum(64);
    addConfigItem(editSliderToolButtonSize, Config::UIToolButtonSizeItem(), m_ui->groupBoxToolButtons, "Tool");

    // setup operation buttons
    EditSlider* editSliderOperationButtonIconSize = new EditSlider;
    editSliderOperationButtonIconSize->setMaximum(64);
    addConfigItem(editSliderOperationButtonIconSize, Config::UIOperationButtonIconSizeItem(), m_ui->groupBoxOperationButtons, "Operation");

    EditSlider* editSliderOperationButtonWidth = new EditSlider;
    editSliderOperationButtonWidth->setMaximum(90);
    addConfigItem(editSliderOperationButtonWidth, Config::UIOperationButtonWidthItem(), m_ui->groupBoxOperationButtons, "Operation");

    EditSlider* editSliderOperationButtonHeight = new EditSlider;
    editSliderOperationButtonHeight->setMaximum(90);
    addConfigItem(editSliderOperationButtonHeight, Config::UIOperationButtonHeightItem(), m_ui->groupBoxOperationButtons, "Operation");

    setWindowTitle(m_windowTitle);
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
                (*i)->restore();
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
        for (QList<InputWidgetWrapper*>::ConstIterator i = m_wrappers.constBegin(); i != m_wrappers.constEnd(); i++)
        {
            if ((*i)->isModified())
            {
                (*i)->updateConfigItem();
            }
        }
        Config::save();
        onValueChanged(QVariant());
    }
}

void ConfigDialog::setCurrentPanel(QWidget * panel)
{
    m_ui->stackedWidgetPanels->setCurrentWidget(panel);
}

void ConfigDialog::setCurrentPanel(const QString & title)
{
    QList<QTreeWidgetItem*> items = m_ui->treeWidgetCatalogue->findItems(title, Qt::MatchExactly);
    m_ui->treeWidgetCatalogue->setCurrentItem(items[0]);
}

void ConfigDialog::addConfigItem(QWidget * widget, Config::ConfigItem * item, QWidget* parent, const QString& exlusion)
{
    QGridLayout * layout = qobject_cast<QGridLayout*>(parent->layout());
    if (layout == nullptr)
    {
        layout = new QGridLayout;
        layout->setColumnMinimumWidth(0, 150);
        layout->setColumnMinimumWidth(1, 240);
        layout->setColumnStretch(2, 1);
        parent->setLayout(layout);
    }

    int row = layout->rowCount();

    layout->addWidget(widget, row, 1);
    widget->setParent(parent);

    QRegularExpression re("[A-Z][^A-Z]*");
    QRegularExpressionMatchIterator match = re.globalMatch(item->name);
    QStringList segs;
    while (match.hasNext())
    {
        QStringList seg = match.next().capturedTexts();
        if (seg[0] != exlusion)
            segs.append(seg[0]);
    }
    QLabel* labelName = new QLabel(parent);
    labelName->setText(segs.join(" "));
    layout->addWidget(labelName, row, 0);

    QLabel* labelDesc = new QLabel(parent);
    labelDesc->setText(item->description);
    layout->addWidget(labelDesc, row, 2);

    InputWidgetWrapper* wrapper = new InputWidgetWrapper(widget, item, labelName, labelDesc);
    connect(wrapper, &InputWidgetWrapper::valueChanged, this, &ConfigDialog::onValueChanged);
    m_wrappers.append(wrapper);
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
