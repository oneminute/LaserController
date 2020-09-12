#ifndef IMPORTSVGDIALOG_H
#define IMPORTSVGDIALOG_H

#include "common/common.h"
#include <QDialog>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class ImportSVGDialog; }
QT_END_NAMESPACE

class ImportSVGDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportSVGDialog(QWidget* parent = nullptr);
    ~ImportSVGDialog();

    bool pageUnitFromSVG() const { return m_pageUnitFromSVG; }
    SizeUnit pageSizeUnit() const { return m_pageSizeUnit; }
    qreal pageWidth() const { return m_pageWidth; }
    qreal pageHeight() const { return m_pageHeight; }
    bool shapeUnitFromSVG() const { return m_shapeUnitFromSVG; }
    SizeUnit shapeSizeUnit() const { return m_shapeSizeUnit; }
    bool useDocumentOrigin() const { return m_useDocumentOrigin; }
    PageType pageType() const { return m_pageType; }

protected slots:
    void onPageUnitFromSVGStateChanged(int state);
    void onPageUnitIndexChanged(int index);
    void onShapeUnitFromSVGStateChanged(int state);
    void onShapeUnitIndexChanged(int index);
    void onUseDocumentOriginStateChanged(int state);
    void onUseDocumentPageSizeStateChanged(int state);
    void onUsePresetPageSizeStateChanged(int state);
    void onPresetPageSizeIndexChanged(const QString& text);
    virtual void accept();

private:
    QScopedPointer<Ui::ImportSVGDialog> m_ui;
    bool m_pageUnitFromSVG;
    SizeUnit m_pageSizeUnit;
    qreal m_pageWidth;
    qreal m_pageHeight;
    bool m_shapeUnitFromSVG;
    SizeUnit m_shapeSizeUnit;
    bool m_useDocumentOrigin;
    bool m_useDocumentPageSize;
    PageType m_pageType;
};

#endif // IMPORTSVGDIALOG_H
