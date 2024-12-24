#ifndef CHECKMESSAGEBOX_H
#define CHECKMESSAGEBOX_H

#include <QObject>
#include <QWidget>
#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>

class CheckMessageBox : public QMessageBox
{
    Q_OBJECT

public:
    CheckMessageBox(Icon icon, const QString &title, const QString &text,
                    const QString &checkBoxText, QWidget *parent = Q_NULLPTR);
    ~CheckMessageBox();
    int exec();

    static int information(QWidget *parent, const QString &title,
                                      const QString &text, const QString &checkBoxText);
    static int question(QWidget *parent, const QString &title,
                                      const QString &text, const QString &checkBoxText);
    static int warning(QWidget *parent, const QString &title,
                                      const QString &text, const QString &checkBoxText);
    static int critical(QWidget *parent, const QString &title,
                                      const QString &text, const QString &checkBoxText);

private slots:
    void changeOkBtnState(int state);

private:
    QCheckBox *chkBox_;
    QPushButton *okBtn_;
    QPushButton *cancelBtn_;
};

#endif // CHECKMESSAGEBOX_H
