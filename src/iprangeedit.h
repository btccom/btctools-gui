#ifndef IPRANGEEDIT_H
#define IPRANGEEDIT_H

#include <QFrame>
#include <QLineEdit>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QFont>
#include <QLabel>
#include <QKeyEvent>
#include <stdint.h>

/***********************************************************************
 * Copied from https://stackoverflow.com/questions/9306335/an-ip-address-widget-for-qt-similar-to-mfcs-ip-address-control
 * And edited by Yihao Peng
 ***********************************************************************/

class IpRangeEdit : public QFrame
{
    Q_OBJECT

public:
    IpRangeEdit(QWidget *parent = 0);
    virtual bool eventFilter( QObject *obj, QEvent *event );

    bool isIpRangeValid();
    QString ipRange();
    QString beautifiedIpRange();

    QString toString();
    void setFromString(QString ipRange);

    QString value(int section);
    void setValue(int section, QString value);

public slots:
    void slotTextChanged( QLineEdit* pEdit );

signals:
    void signalTextChanged( QLineEdit* pEdit );

protected:
    QString regularIpNumber(const QString &ipNumber);

private:
    enum
    {
        QTUTL_IP_SIZE   = 8,// число октетов IP адресе
        QTUTL_IP_RANGE_SEPARATOR = 4, // IP range separator position
        MAX_DIGITS      = 3 // число символов в LineEdit
    };

    QLineEdit *m_pLineEdit[QTUTL_IP_SIZE];
    void MoveNextLineEdit (int i);
    void MovePrevLineEdit (int i);
};


#endif // IPRANGEEDIT_H
