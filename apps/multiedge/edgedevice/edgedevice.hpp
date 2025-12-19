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

class QPushButton;
class QPlainTextEdit;
class QLineEdit;
class QTableView;
class QTabWidget;
class QToolButton;
class QWidget;

class EdgeDevice : public QDialog
{
    Q_OBJECT

public:
    EdgeDevice(QWidget *parent = nullptr);
    ~EdgeDevice();

private:

    inline QWidget* wndConnect(void) const;
    inline QWidget* wndChat(void) const;
    inline QPushButton* ctrlConnect(void) const;
    inline QLineEdit* ctrlAddress(void) const;
    inline QLineEdit* ctrlPort(void) const;
    inline QLineEdit* ctrlName(void) const;
    inline QTableView* ctrlTable(void) const;
    inline QPlainTextEdit* ctrlQuestion(void) const;
    inline QToolButton* ctrlSend(void) const;
    inline QPushButton* ctrlClose(void) const;

    void setupData(void);

private:
    Ui::EdgeDevice*     ui;
    QString             mAddress;
    uint16_t            mPort;
};
#endif // MULTIEDGE_EDGEDEVICE_EDGEDEVICE_HPP
