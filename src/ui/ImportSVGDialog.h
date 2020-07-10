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

    LengthType lengthType() const { return m_lengthType; }
    qreal pageWidth() const { return m_pageWidth; }
    qreal pageHeight() const { return m_pageHeight; }

private:
    QScopedPointer<Ui::ImportSVGDialog> m_ui;
    LengthType m_lengthType;
    qreal m_pageWidth;
    qreal m_pageHeight;
    bool m_useDocumentOrigin;
};

#endif // IMPORTSVGDIALOG_H
