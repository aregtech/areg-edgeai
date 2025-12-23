#ifndef MULTIEDGE_AIAGENT_AIAGENT_HPP
#define MULTIEDGE_AIAGENT_AIAGENT_HPP
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
 *  \file        multiedge/aiagent/aiagent.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDialog>

#include "multiedge/resources/NEMultiEdge.hpp"
QT_BEGIN_NAMESPACE
namespace Ui {
class AIAgent;
}
QT_END_NAMESPACE

class AgentChatHistory;
class QPushButton;
class QLineEdit;
class QTableView;
class QTabWidget;
class QWidget;

class AIAgent : public QDialog
{
    Q_OBJECT

public:
    AIAgent(QWidget *parent = nullptr);
    ~AIAgent();
    
public slots:
    
    void slotServiceAvailable(bool isConnected);
    
    void slotAgentQueueSize(uint32_t queueSize);
    
    void slotAgentType(NEMultiEdge::eEdgeAgent EdgeAgent);

    void slotTextRequested(uint32_t seqId, uint32_t id, const QString& question);
    
    void slotTextProcessed(uint32_t seqId, uint32_t id, QString reply);
    
    void slotVideoProcessed(uint32_t seqId, uint32_t id, SharedBuffer video);
    
    void slotAgentProcessingFailed(NEMultiEdge::eEdgeAgent agent, NEService::eResultType reason);

private:

    inline QWidget* wndConnect(void) const;
    inline QWidget* wndChat(void) const;
    inline QPushButton* ctrlConnect(void) const;
    inline QLineEdit* ctrlAddress(void) const;
    inline QLineEdit* ctrlPort(void) const;
    inline QTableView* ctrlTable(void) const;
    inline QPushButton* ctrlClose(void) const;
    inline QTabWidget* ctrlTab(void) const;
    
private slots:
    
    void onConnectClicked(bool checked);
    
private:
    void setupData(void);
    
    void setupWidgets(void);
    
    void setupSignals(void);
    
    bool routerConnect(void);
    
    void routerDisconnect(void);

private:
    Ui::AIAgent*        ui;
    QString             mAddress;
    uint16_t            mPort;
    AgentChatHistory*   mModel;
};
#endif // MULTIEDGE_AIAGENT_AIAGENT_HPP
