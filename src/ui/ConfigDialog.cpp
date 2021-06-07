#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"

#include <QCheckBox>
#include <QDebug>
#include <QDoubleSpinBox>
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
    m_panels.insert(tr("General"), m_ui->pageGeneral);
    m_panels.insert(tr("Ui"), m_ui->pageUi);
    m_panels.insert(tr("Layers"), m_ui->pageLayers);
    m_panels.insert(tr("Optimize Path"), m_ui->pageOptimizePath);
    m_panels.insert(tr("Plt Utils"), m_ui->pagePltUtils);

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

    EditSlider* editSliderCuttingLaserPower = new EditSlider;
    editSliderCuttingLaserPower->setMaximum(1000);
    addConfigItem(editSliderCuttingLaserPower, Config::CuttingLayerLaserPowerItem(), m_ui->groupBoxCuttingLayer);

    EditSlider* editSliderCuttingMinSpeed = new EditSlider;
    editSliderCuttingMinSpeed->setMaximum(1000);
    addConfigItem(editSliderCuttingMinSpeed, Config::CuttingLayerMinSpeedItem(), m_ui->groupBoxLayers);

    EditSlider* editSliderCuttingMinSpeedPower = new EditSlider;
    editSliderCuttingMinSpeedPower->setMaximum(1000);
    addConfigItem(editSliderCuttingMinSpeedPower, Config::CuttingLayerMinSpeedPowerItem(), m_ui->groupBoxCuttingLayer);

    EditSlider* editSliderCuttingRunSpeed = new EditSlider;
    editSliderCuttingRunSpeed->setMaximum(1000);
    addConfigItem(editSliderCuttingRunSpeed, Config::CuttingLayerRunSpeedItem(), m_ui->groupBoxCuttingLayer);

    EditSlider* editSliderCuttingRunSpeedPower = new EditSlider;
    editSliderCuttingRunSpeedPower->setMaximum(1000);
    addConfigItem(editSliderCuttingRunSpeedPower, Config::CuttingLayerRunSpeedPowerItem(), m_ui->groupBoxCuttingLayer);

    EditSlider* editSliderEngravingDPI = new EditSlider;
    editSliderEngravingDPI->setMaximum(1000);
    addConfigItem(editSliderEngravingDPI, Config::EngravingLayerDPIItem(), m_ui->groupBoxEngravingLayer);

    EditSlider* editSliderEngravingLPI = new EditSlider;
    editSliderEngravingLPI->setMaximum(1000);
    addConfigItem(editSliderEngravingLPI, Config::EngravingLayerLPIItem(), m_ui->groupBoxEngravingLayer);

    EditSlider* editSliderEngravingLaserPower = new EditSlider;
    editSliderEngravingLaserPower->setMaximum(1000);
    addConfigItem(editSliderEngravingLaserPower, Config::EngravingLayerLaserPowerItem(), m_ui->groupBoxEngravingLayer);

    EditSlider* editSliderEngravingMinSpeed = new EditSlider;
    editSliderEngravingMinSpeed->setMaximum(1000);
    addConfigItem(editSliderEngravingMinSpeed, Config::EngravingLayerMinSpeedItem(), m_ui->groupBoxEngravingLayer);

    EditSlider* editSliderEngravingMinSpeedPower = new EditSlider;
    editSliderEngravingMinSpeedPower->setMaximum(1000);
    addConfigItem(editSliderEngravingMinSpeedPower, Config::EngravingLayerMinSpeedPowerItem(), m_ui->groupBoxEngravingLayer);

    EditSlider* editSliderEngravingRunSpeed = new EditSlider;
    editSliderEngravingRunSpeed->setMaximum(1000);
    addConfigItem(editSliderEngravingRunSpeed, Config::EngravingLayerRunSpeedItem(), m_ui->groupBoxEngravingLayer);

    EditSlider* editSliderEngravingRunSpeedPower = new EditSlider;
    editSliderEngravingRunSpeedPower->setMaximum(1000);
    addConfigItem(editSliderEngravingRunSpeedPower, Config::EngravingLayerRunSpeedPowerItem(), m_ui->groupBoxEngravingLayer);

    QCheckBox* checkBoxEngravingUseHalftone = new QCheckBox;
    addConfigItem(checkBoxEngravingUseHalftone, Config::EngravingLayerUseHalftoneItem(), m_ui->groupBoxEngravingLayer);

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

    // setup optimization items
    EditSlider* editSliderOptimizePathMaxAnts = new EditSlider;
    editSliderOptimizePathMaxAnts->setMaximum(1000);
    addConfigItem(editSliderOptimizePathMaxAnts, Config::OptimizePathMaxAntsItem(), m_ui->groupBoxAnt, "Ant");

    EditSlider* editSliderOptimizePathMaxIterations = new EditSlider;
    editSliderOptimizePathMaxIterations->setMaximum(2000);
    addConfigItem(editSliderOptimizePathMaxIterations, Config::OptimizePathMaxIterationsItem(), m_ui->groupBoxAnt, "Ant");

    EditSlider* editSliderOptimizePathMaxStartingPointAnglesDiff = new EditSlider;
    editSliderOptimizePathMaxStartingPointAnglesDiff->setMaximum(90);
    addConfigItem(editSliderOptimizePathMaxStartingPointAnglesDiff, Config::OptimizePathMaxStartingPointAnglesDiffItem(), m_ui->groupBoxAnt, "Ant");

    EditSlider* editSliderOptimizePathMaxStartingPoints = new EditSlider;
    editSliderOptimizePathMaxStartingPoints->setMaximum(100);
    addConfigItem(editSliderOptimizePathMaxStartingPoints, Config::OptimizePathMaxStartingPointsItem(), m_ui->groupBoxAnt, "Ant");

    EditSlider* editSliderOptimizePathMaxTraverseCount = new EditSlider;
    editSliderOptimizePathMaxTraverseCount->setMaximum(10000);
    addConfigItem(editSliderOptimizePathMaxTraverseCount, Config::OptimizePathMaxTraverseCountItem(), m_ui->groupBoxAnt, "Ant");

    QCheckBox* checkBoxOptimizePathUseGreedyAlgorithm = new QCheckBox;
    addConfigItem(checkBoxOptimizePathUseGreedyAlgorithm, Config::OptimizePathUseGreedyAlgorithmItem(), m_ui->groupBoxAnt, "Ant");

    QDoubleSpinBox* doubleSpinBoxOptimizePathVolatileRate = new QDoubleSpinBox;
    doubleSpinBoxOptimizePathVolatileRate->setMaximum(1.0);
    addConfigItem(doubleSpinBoxOptimizePathVolatileRate, Config::OptimizePathVolatileRateItem(), m_ui->groupBoxAnt, "Ant");

    // setup plt utils
    EditSlider* editSliderPltUtilsMaxAnglesDiff = new EditSlider;
    editSliderPltUtilsMaxAnglesDiff->setMaximum(90);
    addConfigItem(editSliderPltUtilsMaxAnglesDiff, Config::PltUtilsMaxAnglesDiffItem(), m_ui->groupBoxPltUtils);

    EditSlider* editSliderPltUtilsMaxIntervalDistance = new EditSlider;
    editSliderPltUtilsMaxIntervalDistance->setMaximum(200);
    addConfigItem(editSliderPltUtilsMaxIntervalDistance, Config::PltUtilsMaxIntervalDistanceItem(), m_ui->groupBoxPltUtils);

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
        layout->setColumnMinimumWidth(2, 240);
        layout->setColumnStretch(0, 0);
        layout->setColumnStretch(1, 1);
        layout->setColumnStretch(2, 0);
        parent->setLayout(layout);
    }

    int row = layout->rowCount();

    layout->addWidget(widget, row, 1);
    widget->setParent(parent);

    QRegularExpression re("[A-Z][^A-Z]*");
    QRegularExpressionMatchIterator match = re.globalMatch(item->title);
    QStringList segs;
    while (match.hasNext())
    {
        QStringList seg = match.next().capturedTexts();
        if (seg[0] != exlusion)
            segs.append(seg[0]);
    }
    QLabel* labelName = new QLabel(parent);
    QString title = segs.join(" ");
    if (title.trimmed().isEmpty()) 
    {
        title = item->title;
    }
    labelName->setText(title);
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
