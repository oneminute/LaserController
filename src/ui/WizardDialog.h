#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include <QDialog>
#include "camera/CameraController.h"

class QLabel;
class QHBoxLayout;
class QStackedLayout;
class QListWidget;
class QPushButton;

class WizardDialogPage : public QWidget
{
    Q_OBJECT
public:
    enum State
    {
        IDLE,
        PROCESSING,
        DONE
    };

    explicit WizardDialogPage(const QString& title, QWidget* parent = nullptr);
    ~WizardDialogPage();

    QString title() const;
    void setTitle(const QString& title);

    State state() const;
    void setState(State state);

signals:
    void titleChanged(const QString& title);
    void stateChanged(int state);

    void entered();
    void exited();

private:
    QString m_title;
    State m_state;
};

class WizardDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WizardDialog(QWidget* parent = nullptr);
    ~WizardDialog();

    void appendPage(WizardDialogPage* page);
    void removePage(WizardDialogPage* page);
    WizardDialogPage* currentPage() const;
    WizardDialogPage* firstPage() const;
    WizardDialogPage* lastPage() const;
    WizardDialogPage* pageAt(int index);
    int currentIndex() const;

    void setLeftLayoutWidth(int width);

protected:
    QHBoxLayout* buttonsLayout() const;

public slots:
    void moveToNextPage();
    void moveToPreviousPage();

protected:
    void updateListTable();
    void updatePage();

private:
    QList<WizardDialogPage*> m_pages;
    int m_currentPageIndex;
    QListWidget* m_pageList;
    QStackedLayout* m_stackLayout;
    QLabel* m_labelCNE;
    QPushButton* m_buttonNext;
    QPushButton* m_buttonPrev;
    QHBoxLayout* m_buttonsLayout;

    QWidget* m_left;
    QWidget* m_right;
};

#endif // WIZARDDIALOG_H