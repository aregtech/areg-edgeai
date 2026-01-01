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
 *  \file        multiedge/edgedevice/edgedevice.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/
#include "multiedge/edgedevice/edgedevice.hpp"
#include "ui/ui_EdgeDevice.h"

#include "areg/appbase/Application.hpp"
#include "areg/base/NEUtilities.hpp"
#include "areg/component/ComponentLoader.hpp"
#include "areg/ipc/ConnectionConfiguration.hpp"
#include "multiedge/resources/NEMultiEdgeSettings.hpp"
#include "multiedge/edgedevice/agentconsumer.hpp"
#include "multiedge/edgedevice/agentchathistory.hpp"

EdgeDevice::EdgeDevice(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EdgeDevice)
    , mAddress("127.0.0.1")
    , mPort(8181)
    , mModel(nullptr)
{
    ui->setupUi(this);
    setupData();
    setupWidgets();
    setupSignals();
}

EdgeDevice::~EdgeDevice()
{
    routerDisconnect();
    delete ui;
}

void EdgeDevice::slotActiveModelChanged(const QString modelName)
{
    ctrlActiveModel()->setText(modelName);
}


void EdgeDevice::slotAgentQueueSize(uint32_t queueSize)
{
    ui->TxtQueueSize->setText(QString::number(queueSize));
}

void EdgeDevice::slotAgentType(NEMultiEdge::eEdgeAgent EdgeAgent)
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

void EdgeDevice::slotTextProcessed(uint32_t id, QString reply, uint64_t stamp)
{
    if (mModel != nullptr)
    {
        mModel->addResponse(reply, id, stamp);
    }
}

void EdgeDevice::slotVideoProcessed(uint32_t id, SharedBuffer video)
{
}

void EdgeDevice::slotAgentProcessingFailed(NEMultiEdge::eEdgeAgent agent, NEService::eResultType reason)
{
    if (mModel != nullptr)
    {
        QString text{ NEMultiEdge::getString(agent) };
        text += ": Failed to process a request, reason = ";
        text += NEService::getString(reason);
        mModel->addFailure(text);
    }
}

void EdgeDevice::slotServiceAvailable(bool isConnected)
{
    ctrlQuestion()->setEnabled(isConnected);
    ctrlSend()->setEnabled(isConnected);
    if (isConnected)
    {
        ctrlTab()->setCurrentIndex(1);
        ctrlQuestion()->setFocus();
        mModel->resetHistory();
    }
}

inline QWidget* EdgeDevice::wndConnect(void) const
{
    return ui->WndConnect;
}

inline QWidget* EdgeDevice::wndChat(void) const
{
    return ui->WndChat;
}

inline QPushButton* EdgeDevice::ctrlConnect(void) const
{
    return ui->BtnConnect;
}

inline QLineEdit* EdgeDevice::ctrlAddress(void) const
{
    return ui->RouterAddress;
}

inline QLineEdit* EdgeDevice::ctrlPort(void) const
{
    return ui->RouterPort;
}

inline QLineEdit* EdgeDevice::ctrlName(void) const
{
    return ui->DeviceName;
}

inline QTableView* EdgeDevice::ctrlTable(void) const
{
    return ui->TableHistory;
}

inline QPlainTextEdit* EdgeDevice::ctrlQuestion(void) const
{
    return ui->TxtAsk;
}

inline QToolButton* EdgeDevice::ctrlSend(void) const
{
    return ui->BtnSend;
}

inline QPushButton* EdgeDevice::ctrlClose(void) const
{
    return ui->BtnClose;
}

inline QTabWidget* EdgeDevice::ctrlTab(void) const
{
    return ui->tabWidget;
}

inline QLineEdit* EdgeDevice::ctrlActiveModel(void) const
{
    return ui->TxtActiveModel;
}

void EdgeDevice::setupData(void)
{
    ConnectionConfiguration config(NERemoteService::eRemoteServices::ServiceRouter, NERemoteService::eConnectionTypes::ConnectTcpip);
    if (config.isConfigured())
    {
        mPort = static_cast<uint16_t>(config.getConnectionPort());
        mAddress = config.getConnectionAddress();
    }

    String name = NEUtilities::generateName(NEMultiEdgeSettings::SERVICE_CONSUMER.data());
    mName = QString::fromStdString(name.getData());
    ctrlAddress()->setText(mAddress);
    ctrlPort()->setText(QString::number(mPort));
    ctrlName()->setText(mName);
    ui->TxtQueueSize->setText("N/A");
    ui->TxtAgentType->setText("N/A");
    mModel = new AgentChatHistory(this);
    ctrlTable()->setModel(mModel);
}

void EdgeDevice::setupWidgets(void)
{
    QIcon icon(":/icons/icon-edge-device");
    setWindowIcon(icon);

    // Ensure the header is explicitly shown. Designer settings / style sheets can keep it hidden,
    // and changing resize mode on a hidden header has no visible effect.
    if (QTableView* table = ctrlTable())
    {
        table->setCornerButtonEnabled(false);

        if (QHeaderView* header = table->horizontalHeader())
        {
            header->setVisible(true);
            header->setHighlightSections(false);
            header->setSectionsClickable(true);
            header->setStretchLastSection(true);
            header->setSectionResizeMode(QHeaderView::Interactive);
            header->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
            header->setSectionResizeMode(1, QHeaderView::ResizeMode::Interactive);
            header->setSectionResizeMode(2, QHeaderView::ResizeMode::Interactive);
            header->setSectionResizeMode(3, QHeaderView::ResizeMode::Interactive);
        }

        table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        // Make sure the view has some header height calculated and repaints with updated header state.
        table->updateGeometry();
        table->viewport()->update();
    }

    ctrlQuestion()->setEnabled(false);
    ctrlSend()->setEnabled(false);
    ctrlTab()->setCurrentIndex(0);
}

void EdgeDevice::setupSignals(void)
{
    connect(ctrlClose()  , &QPushButton::clicked, this, [this](bool checked) {routerDisconnect(); close(); });
    connect(ctrlConnect(), &QPushButton::clicked, this, &EdgeDevice::onConnectClicked);
    connect(ctrlSend()   , &QPushButton::clicked, this, &EdgeDevice::onSendQuestion);
}

bool EdgeDevice::routerConnect(void)
{
    mAddress = ctrlAddress()->text();
    mPort = static_cast<uint16_t>(ctrlPort()->text().toUInt());
    mName = ctrlName()->text();

    ConnectionConfiguration config(NERemoteService::eRemoteServices::ServiceRouter, NERemoteService::eConnectionTypes::ConnectTcpip);
    if (config.isConfigured())
    {
        config.setConnectionAddress(mAddress.toStdString());
        config.setConnectionPort(mPort);
        if (Application::startMessageRouting(mAddress.toStdString().c_str(), mPort))
        {
            NERegistry::Model model = AgentConsumer::createModel(mName, this);
            VERIFY(ComponentLoader::addModelUnique(model));
            ASSERT(Application::isModelLoaded(NEMultiEdgeSettings::MODEL_CONSUMER.data()) == false);
            return Application::loadModel(NEMultiEdgeSettings::MODEL_CONSUMER.data());
        }
    }

    return false;
}

void EdgeDevice::routerDisconnect(void)
{
    Application::unloadModel(NEMultiEdgeSettings::MODEL_CONSUMER.data());
    Application::stopMessageRouting();
    ComponentLoader::removeComponentModel(NEMultiEdgeSettings::MODEL_CONSUMER);
}

void EdgeDevice::onConnectClicked(bool checked)
{
    if (Application::isRouterConnected() == false)
    {
        if (routerConnect())
        {
            ctrlAddress()->setEnabled(false);
            ctrlPort()->setEnabled(false);
            ctrlName()->setEnabled(false);
            ctrlConnect()->setText(tr("&Disconnect"));
            ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-offline")));
            ctrlConnect()->setShortcut(QCoreApplication::translate("EdgeDevice", "Alt+D", nullptr));
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
        ctrlName()->setEnabled(true);
        ctrlConnect()->setText(tr("&Connect"));
        ctrlConnect()->setIcon(QIcon::fromTheme(QString::fromUtf8("network-wireless")));
        ctrlConnect()->setShortcut(QCoreApplication::translate("EdgeDevice", "Alt+C", nullptr));
    }
}

void EdgeDevice::onSendQuestion(bool checked)
{
    QString question = ctrlQuestion()->toPlainText();
    if ((question.isEmpty() == false) && (mModel != nullptr))
    {
        uint32_t id = mModel->addRequest(question);
        if (AgentConsumer::processText(id, question) == false)
        {
            mModel->addFailure("Failed to send response to process question");
        }
    }

    ctrlQuestion()->setPlainText(QString());
}
