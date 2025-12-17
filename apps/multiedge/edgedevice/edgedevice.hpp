#ifndef MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP
#define MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class EdgeDevice;
}
QT_END_NAMESPACE

class EdgeDevice : public QDialog
{
    Q_OBJECT

public:
    EdgeDevice(QWidget *parent = nullptr);
    ~EdgeDevice();

private:
    Ui::EdgeDevice* ui;
};
#endif // MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP
