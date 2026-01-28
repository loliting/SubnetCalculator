#include "IpInputEventFilter.hpp"

#include <QtGui/QKeyEvent>

IpInputEventFilter::IpInputEventFilter(
        QSpinBox* cidrInput,
        std::function<void()> jumpToNextOctetFunc) :
    cidrInput(cidrInput),
    jumpToNextOctetFunc(jumpToNextOctetFunc),
    QObject(nullptr)
{

}

bool IpInputEventFilter::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if(keyEvent->key() == Qt::Key_Slash 
        || keyEvent->key() == Qt::Key_Return
        || keyEvent->key() == Qt::Key_Enter)
        {
            cidrInput->setFocus();
            cidrInput->selectAll();
            return true;
        }
        else if(keyEvent->key() == ' ') {
            jumpToNextOctetFunc();
            return true;
        }
    }
    
    return QObject::eventFilter(obj, event);
}