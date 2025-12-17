#include "multiedge/edgedevice/edgedevice.hpp"
#include "ui/ui_EdgeDevice.h"

EdgeDevice::EdgeDevice(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EdgeDevice)
{
    ui->setupUi(this);
}

EdgeDevice::~EdgeDevice()
{
    delete ui;
}
