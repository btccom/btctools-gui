#include "checkmessagebox.h"

CheckMessageBox::CheckMessageBox(QMessageBox::Icon icon, const QString &title, const QString &text,
                                 const QString &checkBoxText, QWidget *parent)
    :QMessageBox(icon, title, text, NoButton, parent)
{
    chkBox_ = new QCheckBox(checkBoxText, this);
    okBtn_ = new QPushButton(tr("OK"), this);
    cancelBtn_ = new QPushButton(tr("Cancel"), this);

    setCheckBox(chkBox_);
    addButton(okBtn_, AcceptRole);
    addButton(cancelBtn_, RejectRole);
    setDefaultButton(cancelBtn_);

    okBtn_->setEnabled(false);
    connect(chkBox_, SIGNAL(stateChanged(int)), this, SLOT(changeOkBtnState(int)));
}

CheckMessageBox::~CheckMessageBox()
{
    delete chkBox_;
    delete okBtn_;
    delete cancelBtn_;
}

int CheckMessageBox::exec()
{
    QMessageBox::exec();
    return clickedButton() == okBtn_ ? Ok : Cancel;
}

int CheckMessageBox::information(QWidget *parent, const QString &title,
                                 const QString &text, const QString &checkBoxText)
{
    CheckMessageBox msgBox(Information, title, text, checkBoxText, parent);
    return msgBox.exec();
}

int CheckMessageBox::question(QWidget *parent, const QString &title,
                              const QString &text, const QString &checkBoxText)
{
    CheckMessageBox msgBox(Question, title, text, checkBoxText, parent);
    return msgBox.exec();
}

int CheckMessageBox::warning(QWidget *parent, const QString &title,
                             const QString &text, const QString &checkBoxText)
{
    CheckMessageBox msgBox(Warning, title, text, checkBoxText, parent);
    return msgBox.exec();
}

int CheckMessageBox::critical(QWidget *parent, const QString &title,
                              const QString &text, const QString &checkBoxText)
{
    CheckMessageBox msgBox(Critical, title, text, checkBoxText, parent);
    return msgBox.exec();
}

void CheckMessageBox::changeOkBtnState(int state)
{
    okBtn_->setEnabled(state != 0);
}
