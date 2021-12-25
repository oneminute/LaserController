#include "WizardDialog.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>

WizardDialogPage::WizardDialogPage(const QString& title, QWidget* parent)
    : QWidget(parent)
    , m_title(title)
    , m_state(IDLE)
{
    
}

WizardDialogPage::~WizardDialogPage()
{
}

QString WizardDialogPage::title() const
{
    return m_title;
}

void WizardDialogPage::setTitle(const QString& title)
{
    if (m_title == title)
        return;
    m_title = title;
    emit titleChanged(title);
}

WizardDialogPage::State WizardDialogPage::state() const
{
    return m_state;
}

void WizardDialogPage::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged(state);
}

WizardDialog::WizardDialog(QWidget* parent)
    : QDialog(parent)
    , m_currentPageIndex(0)
{
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);

    m_left = new QWidget;
    mainLayout->addWidget(m_left);
    m_right = new QWidget;
    mainLayout->addWidget(m_right);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    m_pageList = new QListWidget;
    m_pageList->setFrameShape(QFrame::NoFrame);
    m_pageList->setAutoFillBackground(false);
    m_pageList->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    m_pageList->setFocusPolicy(Qt::NoFocus);
    m_pageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pageList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftLayout->addWidget(m_pageList);
    m_labelCNE = new QLabel;
    m_labelCNE->setPixmap(QPixmap(":/ui/icons/images/app_icon.png").scaled(90, 100, Qt::KeepAspectRatio));
    leftLayout->addWidget(m_labelCNE);
    leftLayout->setAlignment(m_labelCNE, Qt::AlignCenter);
    leftLayout->setStretch(0, 1);
    leftLayout->setStretch(2, 0);
    m_left->setLayout(leftLayout);

    QVBoxLayout* rightLayout = new QVBoxLayout;
    m_stackLayout = new QStackedLayout;
    rightLayout->addLayout(m_stackLayout);
    m_buttonsLayout = new QHBoxLayout;
    m_buttonPrev = new QPushButton;
    m_buttonPrev->setText(tr("Prev"));
    m_buttonsLayout->addWidget(m_buttonPrev);
    connect(m_buttonPrev, &QPushButton::clicked, this, &WizardDialog::moveToPreviousPage);
    m_buttonNext = new QPushButton;
    m_buttonNext->setText(tr("Next"));
    connect(m_buttonNext, &QPushButton::clicked, this, &WizardDialog::moveToNextPage);
    m_buttonsLayout->addWidget(m_buttonNext);
    m_buttonsLayout->setAlignment(Qt::AlignRight);
    rightLayout->addLayout(m_buttonsLayout);
    rightLayout->setStretch(0, 1);
    rightLayout->setStretch(1, 0);
    m_right->setLayout(rightLayout);

    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);

    this->setLayout(mainLayout);
    this->resize(800, 600);
}

WizardDialog::~WizardDialog()
{
}

void WizardDialog::appendPage(WizardDialogPage* page)
{
    Q_ASSERT(page);
    Q_ASSERT(!m_pages.contains(page));

    m_pages.append(page);
    m_stackLayout->addWidget(page);
    updateListTable();
}

void WizardDialog::removePage(WizardDialogPage* page)
{
    Q_ASSERT(page);

    m_pages.removeOne(page);
    m_stackLayout->removeWidget(page);
    updateListTable();
}

WizardDialogPage* WizardDialog::currentPage() const
{
    if (m_pages.isEmpty())
        return nullptr;

    Q_ASSERT(m_currentPageIndex >= 0 && m_currentPageIndex < m_pages.length());

    return m_pages[m_currentPageIndex];
}

void WizardDialog::moveToNextPage()
{
    if (m_currentPageIndex >= m_pages.length())
        return;

    WizardDialogPage* prevPage = pageAt(m_currentPageIndex);
    m_currentPageIndex++;
    WizardDialogPage* currPage = pageAt(m_currentPageIndex);
    updatePage();
    emit prevPage->exited();
    emit currPage->entered();
}

void WizardDialog::moveToPreviousPage()
{
    if (m_currentPageIndex <= 0)
        return;

    WizardDialogPage* prevPage = pageAt(m_currentPageIndex);
    m_currentPageIndex--;
    WizardDialogPage* currPage = pageAt(m_currentPageIndex);
    updatePage();
    emit prevPage->exited();
    emit currPage->entered();
}

WizardDialogPage* WizardDialog::firstPage() const
{
    if (m_pages.isEmpty())
        return nullptr;

    return m_pages.first();
}

WizardDialogPage* WizardDialog::lastPage() const
{
    if (m_pages.isEmpty())
        return nullptr;

    return m_pages.last();
}

WizardDialogPage* WizardDialog::pageAt(int index)
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.length())
        return nullptr;
    return m_pages.at(index);
}

int WizardDialog::currentIndex() const
{
    return m_currentPageIndex;
}

void WizardDialog::setLeftLayoutWidth(int width)
{
    m_left->setFixedWidth(width);
}

QHBoxLayout* WizardDialog::buttonsLayout() const
{
    return m_buttonsLayout;
}

void WizardDialog::updateListTable()
{
    m_pageList->clear();

    for (WizardDialogPage* page : m_pages)
    {
        QString iconFile;
        switch (page->state())
        {
        case WizardDialogPage::IDLE:
            iconFile = ":/ui/icons/images/circle.png";
            break;
        case WizardDialogPage::PROCESSING:
            iconFile = ":/ui/icons/images/circle_point.png";
            break;
        case WizardDialogPage::DONE:
            iconFile = ":/ui/icons/images/tick.png";
            break;
        }
        QListWidgetItem* item = new QListWidgetItem(QIcon(iconFile), page->title());
        QSize sizeHint = item->sizeHint();
        sizeHint.setHeight(30);
        item->setSizeHint(sizeHint);
        m_pageList->addItem(item);
    }
}

void WizardDialog::updatePage()
{
    for (int i = 0; i < m_pages.length(); i++)
    {
        WizardDialogPage* page = m_pages.at(i);
        QListWidgetItem* item = m_pageList->item(i);
        QFont font = item->font();
        font.setPointSize(12);
        if (i < m_currentPageIndex)
        {
            page->setState(WizardDialogPage::DONE);
            item->setIcon(QIcon(":/ui/icons/images/tick.png"));
            font.setBold(false);
            item->setForeground(Qt::black);
        }
        else if (i == m_currentPageIndex)
        {
            page->setState(WizardDialogPage::PROCESSING);
            item->setIcon(QIcon(":/ui/icons/images/circle_point.png"));
            font.setBold(true);
            item->setForeground(Qt::black);
        }
        else
        {
            page->setState(WizardDialogPage::IDLE);
            item->setIcon(QIcon(":/ui/icons/images/circle.png"));
            font.setBold(false);
            item->setForeground(Qt::gray);
        }
    }

    m_stackLayout->setCurrentIndex(m_currentPageIndex);

    m_buttonPrev->setEnabled(m_currentPageIndex > 0);
    m_buttonNext->setEnabled(m_currentPageIndex < m_pages.length() - 1);
    m_pageList->clearSelection();
}
