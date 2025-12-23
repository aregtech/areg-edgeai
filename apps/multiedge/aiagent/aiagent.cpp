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
 *  \file        multiedge/aiagent/aiagent.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/
#include "multiedge/aiagent/aiagent.hpp"
#include "ui/ui_AIAgent.h"

#include "areg/appbase/Application.hpp"
#include "areg/base/NEUtilities.hpp"
#include "areg/component/ComponentLoader.hpp"
#include "areg/component/NEService.hpp"
#include "areg/ipc/ConnectionConfiguration.hpp"
#include "multiedge/resources/nemultiedgesettings.hpp"
#include "multiedge/aiagent/agentprovider.hpp"

#include <any>

BEGIN_MODEL(NEMultiEdgeSettings::MODEL_PROVIDER.data())
    BEGIN_REGISTER_THREAD(NEMultiEdgeSettings::AGENT_THREAD.data())
        BEGIN_REGISTER_COMPONENT(NEMultiEdgeSettings::SERVICE_PROVIDER.data(), AgentProvider)
            REGISTER_IMPLEMENT_SERVICE(NEMultiEdge::ServiceName, NEMultiEdge::InterfaceVersion)
        END_REGISTER_COMPONENT(NEMultiEdgeSettings::SERVICE_PROVIDER.data())
    END_REGISTER_THREAD(NEMultiEdgeSettings::AGENT_THREAD.data())
END_MODEL(NEMultiEdgeSettings::MODEL_PROVIDER.data())

AIAgent::AIAgent(QWidget *parent)
    : QDialog   (parent)
    , ui        (new Ui::AIAgent)
    , mAddress  ("127.0.0.1")
    , mPort     (8181)
    // , mModel    (nullptr)
{
    ui->setupUi(this);
    setupData();
    setupWidgets();
    setupSignals();
}

AIAgent::~AIAgent()
{
    routerDisconnect();
    delete ui;
}

void AIAgent::slotAgentQueueSize(uint32_t queueSize)
{
    ui->TxtQueueSize->setText(QString::number(queueSize));
}

void AIAgent::slotAgentType(NEMultiEdge::eEdgeAgent EdgeAgent)
{
    const QString _agents[]
    {
          "Unknown"
        , "LLM"
        , "VLM"
        , "Hybrid"
    };
    
    ui->TxtAgentType->setText(_agents[static_cast<int>(EdgeAgent)]);
}

void AIAgent::slotTextProcessed(uint32_t id, QString reply)
{
#if 0
    if (mModel != nullptr)
    {
        mModel->addResponse(reply, id);
    }
#endif
}

void AIAgent::slotVideoProcessed(uint32_t id, SharedBuffer video)
{
}

void AIAgent::slotAgentProcessingFailed(NEMultiEdge::eEdgeAgent agent, NEService::eResultType reason)
{
#if 0
    if (mModel != nullptr)
    {
        QString text{NEMultiEdge::getString(agent)};
        text += ": Failed to process a request, reason = ";
        text += NEService::getString(reason);
        mModel->addFailure(text);
    }
#endif
}

void AIAgent::slotServiceAvailable(bool isConnected)
{
#if 0
    ctrlQuestion()->setEnabled(isConnected);
    ctrlSend()->setEnabled(isConnected);
    if (isConnected)
    {
        ui->tabWidget->setCurrentIndex(1);
        AgentChatHistory* oldModel = mModel;
        ctrlTable()->setModel(nullptr);
        mModel = new AgentChatHistory(ctrlTable());
        ctrlTable()->setModel(mModel);
        if (oldModel != nullptr)
            delete oldModel;
    }
#endif
}

inline QWidget* AIAgent::wndConnect(void) const
{
    return ui->WndConnect;
}

inline QWidget* AIAgent::wndChat(void) const
{
    return ui->WndChat;
}

inline QPushButton* AIAgent::ctrlConnect(void) const
{
    return ui->BtnConnect;
}

inline QLineEdit* AIAgent::ctrlAddress(void) const
{
    return ui->RouterAddress;
}

inline QLineEdit* AIAgent::ctrlPort(void) const
{
    return ui->RouterPort;
}

inline QTableView* AIAgent::ctrlTable(void) const
{
    return ui->TableHistory;
}

inline QPushButton* AIAgent::ctrlClose(void) const
{
    return ui->BtnClose;
}

inline QTabWidget* AIAgent::ctrlTab(void) const
{
    return ui->tabWidget;
}

void AIAgent::setupData(void)
{
    ConnectionConfiguration config(NERemoteService::eRemoteServices::ServiceRouter, NERemoteService::eConnectionTypes::ConnectTcpip);
    if (config.isConfigured())
    {
        mPort   = static_cast<uint16_t>(config.getConnectionPort());
        mAddress= config.getConnectionAddress();
    }
    
    String name = NEUtilities::generateName(NEMultiEdgeSettings::SERVICE_CONSUMER.data());
    ctrlAddress()->setText(mAddress);
    ctrlPort()->setText(QString::number(mPort));
    ui->TxtQueueSize->setText("N/A");
    ui->TxtAgentType->setText("N/A");

}

void AIAgent::setupWidgets(void)
{
}

void AIAgent::setupSignals(void)
{
    connect(ctrlClose()     , &QPushButton::clicked, this, [this](bool checked) {routerDisconnect(); close();});
    connect(ctrlConnect()   , &QPushButton::clicked, this, &AIAgent::onConnectClicked);
}

bool AIAgent::routerConnect(void)
{
    mAddress= ctrlAddress()->text();
    mPort   = static_cast<uint16_t>(ctrlPort()->text().toUInt());
    
    ConnectionConfiguration config(NERemoteService::eRemoteServices::ServiceRouter, NERemoteService::eConnectionTypes::ConnectTcpip);
    if (config.isConfigured())
    {
        config.setConnectionAddress(mAddress.toStdString());
        config.setConnectionPort(mPort);
        if (Application::startMessageRouting(mAddress.toStdString().c_str(), mPort))
        {
            if (ComponentLoader::setComponentData(NEMultiEdgeSettings::SERVICE_PROVIDER.data(), std::make_any<AIAgent *>(this)))
            {
                return Application::loadModel(NEMultiEdgeSettings::MODEL_PROVIDER.data());
            }
        }
    }
    
    return false;
}

void AIAgent::routerDisconnect(void)
{
    Application::unloadModel(NEMultiEdgeSettings::MODEL_CONSUMER.data());
    Application::stopMessageRouting();
}

void AIAgent::onConnectClicked(bool checked)
{
    if (Application::isRouterConnected() == false)
    {
        if (routerConnect())
        {
            ctrlAddress()->setEnabled(false);
            ctrlPort()->setEnabled(false);
            ctrlConnect()->setText(tr("&Disconnect"));
            ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
            ctrlConnect()->setShortcut(QCoreApplication::translate("AIAgent", "Alt+D", nullptr));
        }
        else
        {
            routerDisconnect();
            ctrlConnect()->setChecked(false);
        }
    }
    else
    {
        routerDisconnect();
        ctrlAddress()->setEnabled(true);
        ctrlPort()->setEnabled(true);
        ctrlConnect()->setText(tr("&Connect"));
        ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-wireless")));
        ctrlConnect()->setShortcut(QCoreApplication::translate("AIAgent", "Alt+C", nullptr));
    }
}
