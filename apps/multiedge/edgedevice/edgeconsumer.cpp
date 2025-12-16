#include "edgeconsumer.hpp"
#include "./ui_edgeconsumer.h"

EdgeConsumer::EdgeConsumer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EdgeConsumer)
{
    ui->setupUi(this);
}

EdgeConsumer::~EdgeConsumer()
{
    delete ui;
}
