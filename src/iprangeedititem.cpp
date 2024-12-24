#include "iprangeedititem.h"

IpRangeEditItem::IpRangeEditItem(QListWidget *parent, int row)
{
    parent_ = parent;

    child_ = new QWidget(this);
    layout_ = new QHBoxLayout(child_);

    enable_ = new QCheckBox(child_);
    ipRangeEdit_ = new IpRangeEdit(child_);

    layout_->setMargin(1);

    layout_->addWidget(enable_);
    layout_->addWidget(ipRangeEdit_);

    enable_->setChecked(true);

    child_->setLayout(layout_);

    setSizeHint(QSize(0, 24));

    if (row < 0)
    {
        row = parent_->count();
    }

    parent_->insertItem(row, this);
    parent_->setItemWidget(this, child_);
}

bool IpRangeEditItem::isEnabled()
{
    return enable_->isChecked();
}

void IpRangeEditItem::setEnabled(bool enabled)
{
    enable_->setChecked(enabled);
}

IpRangeEdit *IpRangeEditItem::edit()
{
    return ipRangeEdit_;
}
