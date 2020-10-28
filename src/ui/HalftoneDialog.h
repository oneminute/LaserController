#ifndef HALFTONEDIALOG_H
#define HALFTONEDIALOG_H

#include <QDialog>
#include <QScopedPointer>

namespace Ui
{
    class HalftoneDialog;
}

class HalftoneDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HalftoneDialog(QWidget* parent = nullptr);
    virtual ~HalftoneDialog();

    float lpi() const { return m_lpi; }
    float degrees() const { return m_degrees; }
    float dpi() const { return m_dpi; }
    float pixelInterval() const { return m_pixelInterval; }
    float yPulseLength() const { return m_yPulseLength; }

protected slots:
    void accept() override;

private:
    QScopedPointer<Ui::HalftoneDialog> m_ui;
    float m_lpi;
    float m_degrees;
    float m_dpi;
    float m_pixelInterval;
    float m_yPulseLength;
};

#endif // HALFTONEDIALOG_H