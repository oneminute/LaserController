#include "ImportSVGDialog.h"
#include "ui_ImportSVGDialog.h"

#include "util/UnitUtils.h"

ImportSVGDialog::ImportSVGDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::ImportSVGDialog)
{
    m_ui->setupUi(this);

    m_ui->comboBoxPresetSize->clear();
    for (int i = 0; i < (int)unitUtils::presetPageSizes().length(); i++)
    {
        QPageSize::PageSizeId& id = unitUtils::presetPageSizes()[i];
        QString name = unitUtils::pageSizeName(id);
        m_ui->comboBoxPresetSize->addItem(name, id);
    }
    m_ui->comboBoxPresetSize->setCurrentText(unitUtils::pageSizeName(QPageSize::A4));

    connect(m_ui->checkBoxPageUnitFromSVG, &QCheckBox::stateChanged, this, &ImportSVGDialog::onPageUnitFromSVGStateChanged);
    connect(m_ui->comboBoxPageUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportSVGDialog::onPageUnitIndexChanged);
    connect(m_ui->checkBoxShapeUnitFromSVG, &QCheckBox::stateChanged, this, &ImportSVGDialog::onShapeUnitFromSVGStateChanged);
    connect(m_ui->comboBoxShapeUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportSVGDialog::onShapeUnitIndexChanged);
    connect(m_ui->checkBoxUseSVGOrigin, &QCheckBox::stateChanged, this, &ImportSVGDialog::onUseDocumentOriginStateChanged);
    connect(m_ui->checkBoxUseSVGPageSize, &QCheckBox::stateChanged, this, &ImportSVGDialog::onUseDocumentPageSizeStateChanged);
    connect(m_ui->checkBoxUsePresetSize, &QCheckBox::stateChanged, this, &ImportSVGDialog::onUsePresetPageSizeStateChanged);
    connect(m_ui->comboBoxPresetSize, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &ImportSVGDialog::onPresetPageSizeIndexChanged);
}

ImportSVGDialog::~ImportSVGDialog()
{

}

void ImportSVGDialog::onPageUnitFromSVGStateChanged(int state)
{
    switch (state)
    {
    case Qt::Unchecked:
        m_ui->comboBoxPageUnit->setEnabled(true);
        m_pageSizeUnit = (SizeUnit)m_ui->comboBoxPageUnit->currentIndex();
        m_pageUnitFromSVG = false;
        break;
    case Qt::Checked:
        m_ui->comboBoxPageUnit->setEnabled(false);
        m_pageUnitFromSVG = true;
        break;
    default:
        break;
    }
}

void ImportSVGDialog::onPageUnitIndexChanged(int index)
{
    m_pageSizeUnit = (SizeUnit)m_ui->comboBoxPageUnit->currentIndex();
}

void ImportSVGDialog::onShapeUnitFromSVGStateChanged(int state)
{
    switch (state)
    {
    case Qt::Unchecked:
        m_ui->comboBoxShapeUnit->setEnabled(true);
        m_shapeUnitFromSVG = false;
        m_shapeSizeUnit = (SizeUnit)m_ui->comboBoxShapeUnit->currentIndex();
        break;
    case Qt::Checked:
        m_ui->comboBoxShapeUnit->setEnabled(false);
        m_shapeUnitFromSVG = true;
        break;
    default:
        break;
    }
}

void ImportSVGDialog::onShapeUnitIndexChanged(int index)
{
    m_shapeSizeUnit = (SizeUnit)m_ui->comboBoxShapeUnit->currentIndex();
}

void ImportSVGDialog::onUseDocumentOriginStateChanged(int state)
{
    switch (state)
    {
    case Qt::Unchecked:
        m_useDocumentOrigin = false;
        break;
    case Qt::Checked:
        m_useDocumentOrigin = true;
        break;
    default:
        break;
    }
}

void ImportSVGDialog::onUseDocumentPageSizeStateChanged(int state)
{
    switch (state)
    {
    case Qt::Unchecked:
        m_ui->checkBoxUsePresetSize->setEnabled(true);
        m_ui->comboBoxPresetSize->setEnabled(m_ui->checkBoxUsePresetSize->isChecked());
        m_ui->lineEditPageWidth->setEnabled(!m_ui->checkBoxUsePresetSize->isChecked());
        m_ui->lineEditPageHeight->setEnabled(!m_ui->checkBoxUsePresetSize->isChecked());
        m_useDocumentPageSize = false;
        break;
    case Qt::Checked:
        m_ui->checkBoxUsePresetSize->setEnabled(false);
        m_ui->comboBoxPresetSize->setEnabled(false);
        m_ui->lineEditPageWidth->setEnabled(false);
        m_ui->lineEditPageHeight->setEnabled(false);
        m_useDocumentPageSize = true;
        break;
    default:
        break;
    }
}

void ImportSVGDialog::onUsePresetPageSizeStateChanged(int state)
{
    switch (state)
    {
    case Qt::Unchecked:
        m_ui->comboBoxPresetSize->setEnabled(false);
        m_ui->lineEditPageWidth->setEnabled(true);
        m_ui->lineEditPageHeight->setEnabled(true);
        break;
    case Qt::Checked:
        m_ui->comboBoxPresetSize->setEnabled(true);
        m_ui->lineEditPageWidth->setEnabled(false);
        m_ui->lineEditPageHeight->setEnabled(false);
        break;
    default:
        break;
    }
}

void ImportSVGDialog::onPresetPageSizeIndexChanged(const QString &)
{
    QPageSize::PageSizeId id = m_ui->comboBoxPresetSize->currentData().value<QPageSize::PageSizeId>();
    QPageSize ps(id);
    m_pageWidth = ps.size(QPageSize::Millimeter).width();
    m_pageHeight = ps.size(QPageSize::Millimeter).height();
    m_ui->lineEditPageWidth->setText(QString("%1").arg(m_pageWidth));
    m_ui->lineEditPageHeight->setText(QString("%1").arg(m_pageHeight));

    qDebug() << ps.name() << ps.key() << m_pageWidth << m_pageHeight;
}
