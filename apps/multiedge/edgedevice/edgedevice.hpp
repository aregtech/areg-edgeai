#ifndef MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP
#define MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP
/************************************************************************
 * This file is part of the Areg Edge AI project powered by AREG SDK.
 * The project contains multiple examples of using Edge AI based on Areg communication framework.
 *
 *  Areg Edge AI is available as free and open-source software under the MIT License.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2025 Aregtech UG. All rights reserved.
 *  \file        multiedge/edgedevice/edgedevice.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui {
class EdgeDevice;
}
QT_END_NAMESPACE

class QTabWidget;

class EdgeDevice : public QDialog
{
    Q_OBJECT

public:
    EdgeDevice(QWidget *parent = nullptr);
    ~EdgeDevice();

private:

    inline QTabWidget* ctrlTab() const;

private:
    Ui::EdgeDevice*     ui;
};
#endif // MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP
