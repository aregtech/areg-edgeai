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
 *  \file        multiedge/aiagent/agentprovider.cpp
 *  \ingroup     Areg Edge AI, AI Multi Edge Device Agent
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service provider.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "multiedge/aiagent/agentprovider.hpp"
#include "multiedge/resources/nemultiedgesettings.hpp"
#include "multiedge/aiagent/aiagent.hpp"
#include "areg/base/DateTime.hpp"
#include "areg/component/ComponentThread.hpp"
#include "areg/logging/GELog.h"

DEF_LOG_SCOPE(multiedge_aiagent_AgentProvider_startupServiceInterface);
DEF_LOG_SCOPE(multiedge_aiagent_AgentProvider_shutdownServiceInterface);
DEF_LOG_SCOPE(multiedge_aiagent_AgentProvider_requestProcessText);
DEF_LOG_SCOPE(multiedge_aiagent_AgentProvider_requestProcessVideo);
DEF_LOG_SCOPE(multiedge_aiagent_AgentProvider_processEvent);

AgentProvider* AgentProvider::getService(void)
{
    return static_cast<AgentProvider *>(Component::findComponentByName(NEMultiEdgeSettings::SERVICE_PROVIDER));
}

AgentProvider::AgentProvider(const NERegistry::ComponentEntry& entry, ComponentThread& owner)
    : QObject       (nullptr)
    , Component     (entry, owner)
    , MultiEdgeStub (static_cast<Component &>(self()))
    , IEAgentProcessorEventConsumer()
    , mAIAgent      (std::any_cast<AIAgent*>(entry.getComponentData()))
    , mAgentState   (eAgentState::StateReady)
    , mListSessions ()
    , mWorkerThread ()
    , mAgentProcessor()
{
    ASSERT(mAIAgent != nullptr);
}

void AgentProvider::startupServiceInterface(Component& holder)
{
    LOG_SCOPE(multiedge_aiagent_AgentProvider_startupServiceInterface);
    LOG_DBG("Starting Edge AI agent service, adding AgentProcessorEvent event listener");

    MultiEdgeStub::startupServiceInterface(holder);
    AgentProcessorEvent::addListener(static_cast<IEAgentProcessorEventConsumer&>(self()), holder.getMasterThread());
    setEdgeAgent(NEMultiEdge::AgentLLM);
    setQueueSize(0);

    connect(this, &AgentProvider::signalQueueSize       , mAIAgent, &AIAgent::slotAgentQueueSize, Qt::ConnectionType::QueuedConnection);
    connect(this, &AgentProvider::signalEdgeAgent       , mAIAgent, &AIAgent::slotAgentType     , Qt::ConnectionType::QueuedConnection);
    connect(this, &AgentProvider::signalTextRequested   , mAIAgent, &AIAgent::slotTextRequested , Qt::ConnectionType::QueuedConnection);
    connect(this, &AgentProvider::signalTextProcessed   , mAIAgent, &AIAgent::slotTextProcessed , Qt::ConnectionType::QueuedConnection);

    emit signalEdgeAgent(NEMultiEdge::AgentLLM);
    emit signalQueueSize(0);
}

void AgentProvider::shutdownServiceInterface(Component& holder)
{
    LOG_SCOPE(multiedge_aiagent_AgentProvider_shutdownServiceInterface);
    
    disconnect(this, &AgentProvider::signalQueueSize    , mAIAgent, &AIAgent::slotAgentQueueSize );
    disconnect(this, &AgentProvider::signalEdgeAgent    , mAIAgent, &AIAgent::slotAgentType      );
    disconnect(this, &AgentProvider::signalTextRequested, mAIAgent, &AIAgent::slotTextRequested  );
    disconnect(this, &AgentProvider::signalTextProcessed, mAIAgent, &AIAgent::slotTextProcessed  );

    AgentProcessorEvent::removeListener(static_cast<IEAgentProcessorEventConsumer&>(self()), holder.getMasterThread());
    MultiEdgeStub::shutdownServiceInterface(holder);
}

void AgentProvider::requestProcessText(unsigned int sessionId, unsigned int agentId, const String& textProcess)
{
    LOG_SCOPE(multiedge_aiagent_AgentProvider_requestProcessText);
    SessionID unblock = unblockCurrentRequest();
    mListSessions.push_back({ unblock, sessionId, agentId, textProcess });
    setQueueSize(static_cast<uint32_t>(mListSessions.size()));

    LOG_DBG("Requested to process text. Agent ID [ %u ], session ID [ %u ], agent state [ %s ]", agentId, sessionId, mAgentState == eAgentState::StateReady ? "Ready" : "Busy");

    emit signalQueueSize(static_cast<uint32_t>(mListSessions.size()));
    emit signalTextRequested(sessionId, agentId, QString::fromStdString(textProcess.getString()), DateTime::getNow());
    if (mAgentState == eAgentState::StateReady)
    {
        mAgentState = eAgentState::StateBusy;
        DispatcherThread& worker = DispatcherThread::getDispatcherThread(mWorkerThread);
        ASSERT(worker.isValid());
        AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::eAction::ActionProcessText, unblock, textProcess), worker);
    }
}

void AgentProvider::requestProcessVideo(unsigned int sessionId, bool agentId, const String& cmdText, const SharedBuffer& dataVideo)
{
    LOG_SCOPE(multiedge_aiagent_AgentProvider_requestProcessVideo);
}

IEWorkerThreadConsumer* AgentProvider::workerThreadConsumer(const String& consumerName, const String& workerThreadName)
{
    mWorkerThread = workerThreadName;
    return &mAgentProcessor;
}

void AgentProvider::processEvent(const AgentProcessorEventData& data)
{
    LOG_SCOPE(multiedge_aiagent_AgentProvider_processEvent);
    if (data.getAction() == AgentProcessorEventData::eAction::ActionReplyText)
    {
        LOG_DBG("Processed text....");
        if (!mListSessions.empty())
        {
            const sTextPrompt& prompt = mListSessions.front();
            emit signalTextProcessed(prompt.agentSession, prompt.agentId, QString::fromStdString(data.getPrompt().getData()), DateTime::getNow());
            if (prepareResponse(prompt.sessionId))
            {
                LOG_DBG("Prepared response, sending response to the Agent [ %u ], session [ %u ], response text length [ %u ]"
                        , prompt.agentId
                        , prompt.agentSession
                        , data.getPrompt().getLength());

                responseProcessText(prompt.agentSession, prompt.agentId, data.getPrompt());
            }
            else
            {
                LOG_WARN("No response for Agent [ %u ], session [ %u ]", prompt.agentId, prompt.agentSession);
            }

            mListSessions.erase(mListSessions.begin());
            setQueueSize(static_cast<uint32_t>(mListSessions.size()));
            emit signalQueueSize(static_cast<uint32_t>(mListSessions.size()));
            if (!mListSessions.empty())
            {
                const sTextPrompt& nextPrompt = mListSessions.front();
                LOG_DBG("Processing next text prompt in the queue, Agent [ %u ], session [ %u ], current queue size [ %u ]"
                            , nextPrompt.agentId
                            , nextPrompt.agentSession
                            , static_cast<uint32_t>(mListSessions.size()));

                DispatcherThread& worker = DispatcherThread::getDispatcherThread(mWorkerThread);
                ASSERT(worker.isValid());
                AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::eAction::ActionProcessText, nextPrompt.sessionId, nextPrompt.prompt), worker);
            }
            else
            {
                mAgentState = eAgentState::StateReady;
                LOG_INFO("No more text prompts in the queue, agent state set to Ready");
            }
        }
    }
}

inline AgentProvider& AgentProvider::self(void)
{
    return *this;
}
