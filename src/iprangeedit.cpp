#include "iprangeedit.h"
#include <QRegExp>

IpRangeEdit::IpRangeEdit(QWidget *parent) : QFrame(parent)
{
    setFrameShape( QFrame::StyledPanel );
    setFrameShadow( QFrame::Sunken );

    QHBoxLayout* pLayout = new QHBoxLayout( this );
    setLayout( pLayout );
    pLayout->setContentsMargins( 0, 0, 0, 0 );
    pLayout->setSpacing( 0 );

    for ( int i = 0; i < QTUTL_IP_SIZE; ++i )
    {
        if ( i != 0 )
        {
            QLabel* pDot = new QLabel( (i == QTUTL_IP_RANGE_SEPARATOR) ? "~" : ".", this );
            pDot->setStyleSheet( (i == QTUTL_IP_RANGE_SEPARATOR) ?
                                     "background: white; font-family: Arial, \"Microsoft YaHei\", sans-serif" :
                                     "background: white; font-family: Arial, \"Microsoft YaHei\", sans-serif; font-size: 18px");
            pLayout->addWidget( pDot );
            pLayout->setStretch( pLayout->count(), 0 );
        }

        m_pLineEdit[i] = new QLineEdit( this );
        QLineEdit* pEdit = m_pLineEdit[i];
        pEdit->installEventFilter( this );

        pLayout->addWidget( pEdit );
        pLayout->setStretch( pLayout->count(), 1 );

        pEdit->setFrame( false );
        pEdit->setAlignment( Qt::AlignCenter );

        QFont font = pEdit->font();
        font.setStyleHint( QFont::Monospace );
        font.setFixedPitch( true );
        pEdit->setFont( font );

        QRegExp rx ( "^(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))$" );
        QValidator *validator = new QRegExpValidator(rx, pEdit);
        pEdit->setValidator( validator );

    }

    setMaximumWidth( 50 * QTUTL_IP_SIZE );

    connect( this, SIGNAL(signalTextChanged(QLineEdit*)),
             this, SLOT(slotTextChanged(QLineEdit*)),
             Qt::QueuedConnection );
}

void IpRangeEdit::slotTextChanged( QLineEdit* pEdit )
{
    for ( unsigned int i = 0; i < QTUTL_IP_SIZE; ++i )
    {
        if ( pEdit == m_pLineEdit[i] )
        {
            // auto filling with the end part
            if (i < QTUTL_IP_RANGE_SEPARATOR)
            {
                m_pLineEdit[i + QTUTL_IP_RANGE_SEPARATOR]->setPlaceholderText(pEdit->text());
            }

            if ( ( pEdit->text().size() == MAX_DIGITS &&  pEdit->text().size() == pEdit->cursorPosition() ) || ( pEdit->text() == "0") )
            {
                // auto-move to next item
                if ( i+1 < QTUTL_IP_SIZE )
                {
                   m_pLineEdit[i+1]->setFocus();
                   m_pLineEdit[i+1]->selectAll();
                }
            }
        }
    }
}

QString IpRangeEdit::regularIpNumber(const QString &ipNumber)
{
    // trim it
    QString ipNumberTrimmed = ipNumber.trimmed();

    // empty, not regular
    if (ipNumberTrimmed.isEmpty())
    {
        return ipNumberTrimmed;
    }

    //not empty, convert to uint8_t and convert back.
    uint8_t ipNumUint8 = ipNumberTrimmed.toUShort();
    return QString::number(ipNumUint8);
}

bool IpRangeEdit::isIpRangeValid()
{
    for ( unsigned int i = 0; i < QTUTL_IP_RANGE_SEPARATOR; ++i )
    {
        if (m_pLineEdit[i]->text().isEmpty())
        {
            return false;
        }
    }

    return true;
}

QString IpRangeEdit::ipRange()
{
    QString ipRange = "";

    for ( int i = 0; i < QTUTL_IP_SIZE; ++i )
    {
        if ( i != 0 )
        {
            ipRange += (i == QTUTL_IP_RANGE_SEPARATOR) ? "-" : ".";
        }

        QString ipPart = m_pLineEdit[i]->text();

        if (ipPart.isEmpty() && i >= QTUTL_IP_RANGE_SEPARATOR)
        {
            ipPart = m_pLineEdit[i - QTUTL_IP_RANGE_SEPARATOR]->text();
        }

        ipRange += ipPart;
    }

    return ipRange;
}

QString IpRangeEdit::beautifiedIpRange()
{
    QString ipRange = "";

    for ( int i = 0; i < QTUTL_IP_RANGE_SEPARATOR; ++i )
    {
        if (i != 0)
        {
            ipRange += '.';
        }

        ipRange += m_pLineEdit[i]->text();
    }

    QStringList ipRangeEndList;
    bool allSame = true;

    for ( int i = QTUTL_IP_RANGE_SEPARATOR; i < QTUTL_IP_SIZE; ++i )
    {
        QString ipPartEnd = m_pLineEdit[i]->text();
        QString ipPartBegin = m_pLineEdit[i - QTUTL_IP_RANGE_SEPARATOR]->text();

        if (ipPartEnd.isEmpty())
        {
            ipPartEnd = ipPartBegin;
        }

        if (ipPartEnd != ipPartBegin)
        {
            allSame = false;
            ipRangeEndList.append(ipPartEnd);
        }
        else if (!allSame)
        {
            ipRangeEndList.append(ipPartEnd);
        }
    }

    if (!ipRangeEndList.isEmpty())
    {
        ipRange += '~';
        ipRange += ipRangeEndList.join('.');
    }

    return ipRange;
}

QString IpRangeEdit::toString()
{
    QString ipRange = "";

    for ( int i = 0; i < QTUTL_IP_SIZE; ++i )
    {
        if ( i != 0 )
        {
            ipRange += (i == QTUTL_IP_RANGE_SEPARATOR) ? "-" : ".";
        }

        ipRange += m_pLineEdit[i]->text();
    }

    return ipRange;
}

void IpRangeEdit::setFromString(QString ipRange)
{
    QStringList sections = ipRange.trimmed().split(QRegExp("[^0-9\\.]"));
    int count = sections.count();

    if (count >= 1)
    {
        QStringList beginParts = sections.at(0).split(QRegExp("[^0-9]"));

        for (int i=0; i<QTUTL_IP_RANGE_SEPARATOR && i<beginParts.size(); i++)
        {
            setValue(i, beginParts[i]);
        }
    }

    if (count >= 2)
    {
        QStringList endParts = sections.at(1).split(QRegExp("[^0-9]"));

        for (int i=QTUTL_IP_SIZE-1; i>=QTUTL_IP_RANGE_SEPARATOR && !endParts.isEmpty(); i--)
        {
            QString ipPart = endParts.back();
            endParts.pop_back();
            setValue(i, ipPart);
        }
    }
}

QString IpRangeEdit::value(int section)
{
    if (section >= QTUTL_IP_SIZE)
    {
        throw QString("IpRangeEdit::value(): section %1 out of bounds!").arg(section);
    }

    return m_pLineEdit[section]->text();
}

void IpRangeEdit::setValue(int section, QString value)
{
    if (section >= QTUTL_IP_SIZE)
    {
        throw QString("IpRangeEdit::value(): section %1 out of bounds!").arg(section);
    }

    m_pLineEdit[section]->setText(regularIpNumber(value));

    if (section < QTUTL_IP_RANGE_SEPARATOR)
    {
        m_pLineEdit[section + QTUTL_IP_RANGE_SEPARATOR]->setPlaceholderText(m_pLineEdit[section]->text());
    }
}

bool IpRangeEdit::eventFilter(QObject *obj, QEvent *event)
{
    bool bRes = QFrame::eventFilter(obj, event);

    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* pEvent = dynamic_cast<QKeyEvent*>( event );
        if ( pEvent )
        {
            for ( unsigned int i = 0; i < QTUTL_IP_SIZE; ++i )
            {
                QLineEdit* pEdit = m_pLineEdit[i];
                if ( pEdit == obj )
                {
                    switch ( pEvent->key() )
                    {
                    case Qt::Key_Left:
                        if ( pEdit->cursorPosition() == 0 )
                        {
                            // user wants to move to previous item
                            MovePrevLineEdit(i);
                        }
                        break;

                    case Qt::Key_Right:
                        if ( pEdit->text().isEmpty() || (pEdit->text().size() == pEdit->cursorPosition()) )
                        {
                            // user wants to move to next item
                            MoveNextLineEdit(i);
                        }
                        break;

                    case Qt::Key_0:
                        if ( pEdit->text().isEmpty() || pEdit->text() == "0" )
                        {
                            pEdit->setText("0");
                            // user wants to move to next item
                            MoveNextLineEdit(i);
                        }
                        emit signalTextChanged( pEdit );
                        break;

                    case Qt::Key_Backspace:
                        if ( pEdit->text().isEmpty() || pEdit->cursorPosition() == 0)
                        {
                            // user wants to move to previous item
                            MovePrevLineEdit(i);
                        }
                        break;

                    case Qt::Key_Comma:
                    case Qt::Key_Period:
                        MoveNextLineEdit(i);
                        break;

                    default:
                        emit signalTextChanged( pEdit );
                        break;

                    }
                }
            }
        }
    }

    return bRes;
}

void IpRangeEdit::MoveNextLineEdit(int i)
{
    if ( i+1 < QTUTL_IP_SIZE )
    {
        m_pLineEdit[i+1]->setFocus();
        m_pLineEdit[i+1]->setCursorPosition( 0 );
        m_pLineEdit[i+1]->selectAll();
    }
}

void IpRangeEdit::MovePrevLineEdit(int i)
{
    if ( i != 0 )
    {
        m_pLineEdit[i-1]->setFocus();
        m_pLineEdit[i-1]->setCursorPosition( m_pLineEdit[i-1]->text().size() );
        m_pLineEdit[i-1]->selectAll();
    }
}
