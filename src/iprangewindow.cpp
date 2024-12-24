#include <QMessageBox>
#include <QDebug>
#include "iprangewindow.h"
#include "ui_iprangewindow.h"
#include "utils.h"

IpRangeWindow::IpRangeWindow(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::IpRangeWindow)
{
    ui_->setupUi(this);

    // hide the question button from the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

IpRangeWindow::~IpRangeWindow()
{
    delete ui_;
}

QString IpRangeWindow::comment()
{
    return ui_->commentEdit->text();
}

void IpRangeWindow::setComment(const QString &comment)
{
    ui_->commentEdit->setText(comment);
}

int IpRangeWindow::ipRangeNum()
{
    return ui_->ipRangeBox->count();
}

void IpRangeWindow::accept()
{
    int count = ui_->ipRangeBox->count();

    //********** allow empty **********
    /*
    if (count <= 0)
    {
        QMessageBox::information(this, tr("IP Range is Empty"), tr("Please add at least one IP range."));
        return;
    }
    */

    for (int row=0; row<count; row++)
    {
        IpRangeEditItem *item = dynamic_cast<IpRangeEditItem *>(ui_->ipRangeBox->item(row));

        if (!item->edit()->isIpRangeValid())
        {
            QMessageBox::information(this, tr("IP Range is not Complete"), tr("Please fill all parts of the IP range."));
            return;
        }
    }

    QDialog::accept();
}

QStringList IpRangeWindow::enabledIpRanges()
{
    QStringList enabledIpRange;

    int count = ui_->ipRangeBox->count();

    for (int row=0; row<count; row++)
    {
        IpRangeEditItem *item = dynamic_cast<IpRangeEditItem *>(ui_->ipRangeBox->item(row));

        if (item->isEnabled())
        {
            enabledIpRange.append(item->edit()->ipRange());
        }
    }

    return enabledIpRange;
}

QStringList IpRangeWindow::beautifiedEnabledIpRange()
{
    QStringList enabledIpRange;

    int count = ui_->ipRangeBox->count();

    for (int row=0; row<count; row++)
    {
        IpRangeEditItem *item = dynamic_cast<IpRangeEditItem *>(ui_->ipRangeBox->item(row));

        if (item->isEnabled())
        {
            enabledIpRange.append(item->edit()->beautifiedIpRange());
        }
    }

    return enabledIpRange;
}

void IpRangeWindow::addIpRange(const QString &ipRange, bool enabled)
{
    addIpRange(nullptr, ipRange, enabled);
}

void IpRangeWindow::clearIpRange()
{
    ui_->ipRangeBox->clear();
}

QString IpRangeWindow::toString()
{
    QStringList ipRangeList;

    int count = ui_->ipRangeBox->count();

    for (int row=0; row<count; row++)
    {
        IpRangeEditItem *item = dynamic_cast<IpRangeEditItem *>(ui_->ipRangeBox->item(row));

        QString ipRangeStr = item->edit()->toString();

        if (!item->isEnabled())
        {
            ipRangeStr = QString("!") + ipRangeStr;
        }

        ipRangeList.append(ipRangeStr);
    }

    // comment cannot contains ':'
    QString commentStr = comment().replace(':', ' ');

    return commentStr + ":" + ipRangeList.join(",");
}

void IpRangeWindow::setFromString(QString str)
{
    int pos = str.indexOf(':');

    if (pos < 0)
    {
        return;
    }

    setComment(str.left(pos));
    clearIpRange();

    QString ipRangeStr = str.right(str.size() - pos - 1);
    QStringList ipRangeList = ipRangeStr.split(',');

    foreach (QString range, ipRangeList)
    {
        if (range.isEmpty())
        {
            continue;
        }

        bool enabled = range.at(0).toLatin1() != '!';

        if (!enabled)
        {
            range = range.right(range.size() - 1);
        }

        addIpRange(range, enabled);
    }
}

void IpRangeWindow::addIpRange(IpRangeEditItem *position, const QString &ipRange, bool enabled)
{
    int row = ui_->ipRangeBox->count();

    if (position != nullptr)
    {
        row = ui_->ipRangeBox->row(position) + 1;
    }

    auto item = new IpRangeEditItem(ui_->ipRangeBox, row);

    item->setEnabled(enabled);

    if (!ipRange.isEmpty())
    {
        item->edit()->setFromString(ipRange);
    }
}

void IpRangeWindow::removeIpRange(IpRangeEditItem *ipRange)
{
    ui_->ipRangeBox->removeItemWidget(ipRange);
    delete ipRange;
}

void IpRangeWindow::on_addIpRangeButton_clicked()
{
    auto selectedIpRange = ui_->ipRangeBox->selectedItems();
    IpRangeEditItem *position = nullptr;

    if (!selectedIpRange.isEmpty())
    {
        position = dynamic_cast<IpRangeEditItem *>(selectedIpRange.first());
    }

    addIpRange(position, Utils::getInitIpRange());
}

void IpRangeWindow::on_removeIpRangeButton_clicked()
{
    auto selectedIpRange = ui_->ipRangeBox->selectedItems();

    if (!selectedIpRange.isEmpty())
    {
        removeIpRange(dynamic_cast<IpRangeEditItem *>(selectedIpRange.first()));
    }
    else
    {
        removeIpRange(dynamic_cast<IpRangeEditItem *>(ui_->ipRangeBox->item(ui_->ipRangeBox->count() - 1)));
    }
}

void IpRangeWindow::on_ipRangeEnableAll_clicked()
{
    bool enabled = ui_->ipRangeEnableAll->isChecked();
    int size = ui_->ipRangeBox->count();

    for (int i=0; i<size; i++)
    {
        IpRangeEditItem *item = dynamic_cast<IpRangeEditItem *>(ui_->ipRangeBox->item(i));
        item->setEnabled(enabled);
    }
}
