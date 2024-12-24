#include <QDebug>
#include "iprangelistitem.h"
#include "utils.h"

IpRangeListItem::IpRangeListItem(QListWidget *parent, int row, QWidget *editWindowParent) :
    ipRangeWindow_(editWindowParent)
{
    parent_ = parent;

    child_ = new QWidget(this);
    layout_ = new QHBoxLayout(child_);

    enabled_ = new QCheckBox(child_);
    enabled_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    descText_ = new QLabel(child_);

    layout_->setMargin(1);

    layout_->addWidget(enabled_);
    layout_->addWidget(descText_);

    enabled_->setChecked(true);

    child_->setLayout(layout_);

    setSizeHint(QSize(0, 24));

    if (row < 0)
    {
        row = parent_->count();
    }

    parent_->insertItem(row, this);
    parent_->setItemWidget(this, child_);
}

bool IpRangeListItem::isEnabled()
{
    return enabled_->isChecked();
}

void IpRangeListItem::setEnabled(bool enabled)
{
    enabled_->setChecked(enabled);
}

bool IpRangeListItem::editWithNewWindow()
{
    QString oldItemStr = ipRangeWindow_.toString();

    // init IP range
    if (ipRangeWindow_.ipRangeNum() <= 0)
    {
        ipRangeWindow_.addIpRange(Utils::getInitIpRange());
    }

    int result = ipRangeWindow_.exec();
    bool accepted = (result == QDialog::Accepted);

    if (!accepted)
    {
        ipRangeWindow_.setFromString(oldItemStr);
    }

    syncDataAndDisplay();
    return accepted;
}

QStringList IpRangeListItem::enabledIpRanges()
{
    return enabledIpRanges_;
}

QString IpRangeListItem::toString()
{
    QString itemStr = ipRangeWindow_.toString();

    if (!enabled_->isChecked())
    {
        itemStr = QString("#") + itemStr;
    }

    return itemStr;
}

void IpRangeListItem::setFromString(QString str)
{
    bool isEnabled = str.at(0).toLatin1() != '#';

    setEnabled(isEnabled);

    if (!isEnabled)
    {
        str = str.right(str.size() - 1);
    }

    ipRangeWindow_.setFromString(str);
    syncDataAndDisplay();
}

void IpRangeListItem::syncDataAndDisplay()
{
    // sync data
    enabledIpRanges_ = ipRangeWindow_.enabledIpRanges();

    // sync display
    QString desc = "";

    QString comment = ipRangeWindow_.comment().trimmed();

    if (!comment.isEmpty())
    {
        desc += comment + ": ";
    }

    QString ipRanges = ipRangeWindow_.beautifiedEnabledIpRange().join(", ");

    if (ipRanges.isEmpty())
    {
        ipRanges = tr("Empty", "There is no IP range of the item.");
    }

    desc += ipRanges;

    descText_->setText(desc);
}
