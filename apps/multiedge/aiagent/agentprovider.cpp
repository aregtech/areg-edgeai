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
#include "areg/component/ComponentThread.hpp"

AgentProvider* AgentProvider::getService(void)
{
    return static_cast<AgentProvider *>(Component::findComponentByName(NEMultiEdgeSettings::SERVICE_PROVIDER));
}

AgentProvider::AgentProvider(const NERegistry::ComponentEntry& entry, ComponentThread& owner)
    : QObject       (nullptr)
    , Component     (entry, owner)
    , MultiEdgeStub (static_cast<Component &>(self()))
    , IEAgetProcessorEventConsumer()
    , mAIAgent      (std::any_cast<AIAgent*>(entry.getComponentData()))
    , mAgentState   (eAgentState::StateReady)
    , mListSessions ()
    , mWorkerThread ()
    , mAgentProcessor()
{
    ASSERT(mAIAgent != nullptr);
}

void AgentProvider::requestProcessText(unsigned int sessionId, unsigned int agentId, const String& textProcess)
{
    SessionID unblock = unblockCurrentRequest();
    mListSessions.push_back({ unblock, sessionId, agentId, textProcess });
    setQueueSize(static_cast<uint32_t>(mListSessions.size()));
    if (mAgentState == eAgentState::StateReady)
    {
        DispatcherThread& worker = DispatcherThread::getDispatcherThread(mWorkerThread);
        ASSERT(worker.isValid());
        AgetProcessorEvent::sendEvent(AgetProcessorEventData(AgetProcessorEventData::eAction::ActionProcessText, unblock, textProcess), worker);
    }
}

void AgentProvider::requestProcessVideo(unsigned int sessionId, bool agentId, const String& cmdText, const SharedBuffer& dataVideo)
{
}

IEWorkerThreadConsumer* AgentProvider::workerThreadConsumer(const String& consumerName, const String& workerThreadName)
{
    mWorkerThread = workerThreadName;
    return &mAgentProcessor;
}

void AgentProvider::processEvent(const AgetProcessorEventData& data)
{
    if (data.getAction() == AgetProcessorEventData::eAction::ActionReplyText)
    {
        if (!mListSessions.empty())
        {
            const sTextPrompt& prompt = mListSessions.front();
            if (prepareResponse(prompt.sessionId))
            {
                responseProcessText(prompt.agentSession, prompt.agentId, data.getPrompt());
            }

            mListSessions.erase(mListSessions.begin());
            setQueueSize(static_cast<uint32_t>(mListSessions.size()));
            if (!mListSessions.empty())
            {
                const sTextPrompt& nextPrompt = mListSessions.front();
                DispatcherThread& worker = DispatcherThread::getDispatcherThread(mWorkerThread);
                ASSERT(worker.isValid());
                AgetProcessorEvent::sendEvent(AgetProcessorEventData(AgetProcessorEventData::eAction::ActionProcessText, nextPrompt.sessionId, nextPrompt.prompt), worker);
            }
            else
            {
                mAgentState = eAgentState::StateReady;
            }
        }
    }
}

void AgentProvider::startupServiceInterface(Component& holder)
{
    MultiEdgeStub::startupServiceInterface(holder);
    AgetProcessorEvent::addListener(static_cast<IEAgetProcessorEventConsumer&>(self()), holder.getMasterThread());
    setEdgeAgent(NEMultiEdge::AgentLLM);
}

void AgentProvider::shutdownServiceIntrface(Component& holder)
{
    AgetProcessorEvent::removeListener(static_cast<IEAgetProcessorEventConsumer&>(self()), holder.getMasterThread());
    MultiEdgeStub::shutdownServiceIntrface(holder);
}

inline AgentProvider& AgentProvider::self(void)
{
    return *this;
}
