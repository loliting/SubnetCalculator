#include <QtCore/QObject>
#include <QtWidgets/QSpinBox>

#include <functional>

class IpInputEventFilter : public QObject
{
    Q_OBJECT
private:
    QSpinBox *cidrInput;
    std::function<void(void)> jumpToNextOctetFunc;
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
public:
    IpInputEventFilter(QSpinBox* cidrInput, std::function<void()> jumpToNextOctetFunc);
};
