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
 *  \file        multiedge/edgedevice/agentconsumer.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service consumer.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "multiedge/edgedevice/agentconsumer.hpp"
#include "multiedge/resources/nemultiedgesettings.hpp"
#include "areg/component/ComponentLoader.hpp"
#include "areg/component/ComponentThread.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/logging/GELog.h"

#include "multiedge/edgedevice/edgedevice.hpp"
#include <QApplication>

DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_processText);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_processVideo);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_serviceConnected);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_onQueueSizeUpdate);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_onEdgeAgentUpdate);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_responseProcessText);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_responseProcessVideo);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_requestProcessTextFailed);
DEF_LOG_SCOPE(multiedge_edgedevice_AgentConsumer_requestProcessVideoFailed);

String AgentConsumer::mConsumerName;

bool AgentConsumer::processText(uint32_t id, const QString& text)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_processText);
    
    AgentConsumer* comp = AgentConsumer::getService();
    if ((comp != nullptr) && comp->isConnected())
    {
        LOG_DBG("Sending text to agent consumer, id: %u", id);
        comp->requestProcessText(id, comp->mConsumerId, String(text.toStdString()));
        return true;
    }

    LOG_ERR("Failed to send text to agent consumer, id: %u", id);
    return false;
}

bool AgentConsumer::processVideo(uint32_t id, const QString& cmdText, const SharedBuffer& video)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_processVideo);
    
    AgentConsumer* comp = AgentConsumer::getService();
    if ((comp != nullptr) && comp->isConnected())
    {
        LOG_DBG("Sending video to agent consumer, id: %u", id);
        comp->requestProcessVideo(id, comp->mConsumerId, String(cmdText.toStdString()), video);
        return true;
    }

    LOG_ERR("Failed to send video to agent consumer, id: %u", id);
    return false;
}

NERegistry::Model AgentConsumer::createModel(const QString& name, EdgeDevice * context)
{
    NERegistry::Model model(NEMultiEdgeSettings::MODEL_CONSUMER);
    if (name.isEmpty() == false)
    {
        AgentConsumer::mConsumerName = name.toStdString();
        NERegistry::ComponentThreadEntry & listThreads = model.addThread(NEMultiEdgeSettings::AGENT_THREAD);
        NERegistry::ComponentEntry& component = listThreads.addComponent<AgentConsumer>(AgentConsumer::mConsumerName);
        component.addDependencyService(NEMultiEdgeSettings::SERVICE_PROVIDER);
        component.setComponentData(std::make_any<EdgeDevice *>(context));
    }

    return model;
}

AgentConsumer* AgentConsumer::getService(void)
{
    return static_cast<AgentConsumer*>(AgentConsumer::mConsumerName.isEmpty() ? nullptr : Component::findComponentByName(AgentConsumer::mConsumerName));    
}

AgentConsumer::AgentConsumer(const NERegistry::ComponentEntry& entry, ComponentThread& owner)
    : Component          (entry, owner)
    , MultiEdgeClientBase(entry.mDependencyServices[0].mRoleName, owner)
    , QObject            ( )
    , mConsumerId        (static_cast<uint32_t>(NEService::COOKIE_UNKNOWN))
    , mEdgeDevice        (std::any_cast<EdgeDevice *>(entry.getComponentData()))
{
    ASSERT(mEdgeDevice != nullptr);
    QObject::connect(this, &AgentConsumer::signalServiceConnected, mEdgeDevice, &EdgeDevice::slotServiceAvailable, Qt::ConnectionType::QueuedConnection);
}

AgentConsumer::~AgentConsumer(void)
{
    ASSERT(mEdgeDevice != nullptr);
    QObject::disconnect(this, &AgentConsumer::signalServiceConnected, mEdgeDevice, &EdgeDevice::slotServiceAvailable);
}

bool AgentConsumer::serviceConnected(NEService::eServiceConnection status, ProxyBase& proxy)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_serviceConnected);

    bool result = MultiEdgeClientBase::serviceConnected(status, proxy);
    if (result)
    {
        ASSERT(getProxy() == &proxy);
        bool isConnected(status == NEService::eServiceConnection::ServiceConnected);
        LOG_DBG("AgentConsumer service connection status: %s, proxy: %s", NEService::getString(status), proxy.getProxyAddress().getServiceName().getString());

        notifyOnQueueSizeUpdate(isConnected);
        notifyOnEdgeAgentUpdate(isConnected);
        mConsumerId = isConnected ? static_cast<uint32_t>(proxy.getProxyAddress().getCookie()) : static_cast<uint32_t>(NEService::COOKIE_UNKNOWN);
        
        ASSERT(mEdgeDevice != nullptr);
        if (isConnected)
        {
            connect(this, &AgentConsumer::signalAgentQueueSize       , mEdgeDevice, &EdgeDevice::slotAgentQueueSize        , Qt::ConnectionType::QueuedConnection);
            connect(this, &AgentConsumer::signalAgentType            , mEdgeDevice, &EdgeDevice::slotAgentType             , Qt::ConnectionType::QueuedConnection);
            connect(this, &AgentConsumer::signalTextProcessed        , mEdgeDevice, &EdgeDevice::slotTextProcessed         , Qt::ConnectionType::QueuedConnection);
            connect(this, &AgentConsumer::signalVideoProcessed       , mEdgeDevice, &EdgeDevice::slotVideoProcessed        , Qt::ConnectionType::QueuedConnection);
            connect(this, &AgentConsumer::signalAgentProcessingFailed, mEdgeDevice, &EdgeDevice::slotAgentProcessingFailed , Qt::ConnectionType::QueuedConnection);
        }
        else
        {
            disconnect(this, &AgentConsumer::signalAgentQueueSize       , mEdgeDevice, &EdgeDevice::slotAgentQueueSize);
            disconnect(this, &AgentConsumer::signalAgentType            , mEdgeDevice, &EdgeDevice::slotAgentType);
            disconnect(this, &AgentConsumer::signalTextProcessed        , mEdgeDevice, &EdgeDevice::slotTextProcessed);
            disconnect(this, &AgentConsumer::signalVideoProcessed       , mEdgeDevice, &EdgeDevice::slotVideoProcessed);
            disconnect(this, &AgentConsumer::signalAgentProcessingFailed, mEdgeDevice, &EdgeDevice::slotAgentProcessingFailed);
        }
        
        emit signalServiceConnected(isConnected);
    }

    return result;
}

void AgentConsumer::onQueueSizeUpdate(unsigned int QueueSize, NEService::eDataStateType state)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_onQueueSizeUpdate);
    LOG_DBG("Agent queue size update, size: %u, state: %s", QueueSize, NEService::getString(state));

    emit signalAgentQueueSize(state == NEService::eDataStateType::DataIsOK ? QueueSize : 0u);
}

void AgentConsumer::onEdgeAgentUpdate(NEMultiEdge::eEdgeAgent EdgeAgent, NEService::eDataStateType state)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_onEdgeAgentUpdate);
    LOG_DBG("Edge agent update, type: %s, state: %s", NEMultiEdge::getString(EdgeAgent), NEService::getString(state));

    emit signalAgentType(state == NEService::eDataStateType::DataIsOK ? EdgeAgent : NEMultiEdge::eEdgeAgent::AgentUnknown);
}

void AgentConsumer::responseProcessText(unsigned int sessionId, unsigned int agentId, const String& textReplied)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_responseProcessText);
    ASSERT(agentId == mConsumerId);

    if (agentId == mConsumerId)
    {
        LOG_DBG("Received text reply, sessionId: %u, agentId: %u", sessionId, agentId);
        emit signalTextProcessed(sessionId, QString::fromStdString(textReplied.getData()) );
    }
    else
    {
        LOG_ERR("Received text reply, but agentId does not match, sessionId: %u, agentId: %u", sessionId, agentId);
        emit signalAgentProcessingFailed(NEMultiEdge::eEdgeAgent::AgentLLM, NEService::eResultType::RequestInvalid);
    }
}

void AgentConsumer::responseProcessVideo(unsigned int sessionId, unsigned int agentId, const SharedBuffer& dataVideo)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_responseProcessVideo);
    ASSERT(agentId == mConsumerId);

    if (agentId == mConsumerId)
    {
        LOG_DBG("Received video reply, sessionId: %u, agentId: %u", sessionId, agentId);
        emit signalVideoProcessed(sessionId, dataVideo);
    }
    else
    {
        LOG_ERR("Received video reply, but agentId does not match, sessionId: %u, agentId: %u", sessionId, agentId);
        emit signalAgentProcessingFailed(NEMultiEdge::eEdgeAgent::AgentVLM, NEService::eResultType::RequestInvalid);
    }
}

void AgentConsumer::requestProcessTextFailed(NEService::eResultType FailureReason)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_requestProcessTextFailed);
    LOG_ERR("Failed to process text, reason: %s", NEService::getString(FailureReason));
    emit signalAgentProcessingFailed(NEMultiEdge::eEdgeAgent::AgentLLM, FailureReason);
}

void AgentConsumer::requestProcessVideoFailed(NEService::eResultType FailureReason)
{
    LOG_SCOPE(multiedge_edgedevice_AgentConsumer_requestProcessVideoFailed);
    LOG_ERR("Failed to process video, reason: %s", NEService::getString(FailureReason));
    emit signalAgentProcessingFailed(NEMultiEdge::eEdgeAgent::AgentVLM, FailureReason);
}
