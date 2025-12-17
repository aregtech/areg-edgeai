#ifndef EDGECONSUMER_HPP
#define EDGECONSUMER_HPP

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class EdgeConsumer;
}
QT_END_NAMESPACE

class EdgeConsumer : public QDialog
{
    Q_OBJECT

public:
    EdgeConsumer(QWidget *parent = nullptr);
    ~EdgeConsumer();

private:
    Ui::EdgeConsumer *ui;
};
#endif // EDGECONSUMER_HPP
