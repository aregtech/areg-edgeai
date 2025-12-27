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
 *  \file        multiedge/aiagent/agentprocessor.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent worker thread consumer object to process LLM.
 *
 ************************************************************************/
#include "multiedge/aiagent/agentprocessor.hpp"
#include "areg/component/WorkerThread.hpp"
#include "multiedge/resources/nemultiedgesettings.hpp"
#include "areg/component/ComponentThread.hpp"

AgentProcessorEventData::AgentProcessorEventData(void)
    : mAction   (ActionUnknown)
    , mSessionId(0)
    , mPrompt   ( )
    , mVideo    ( )
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt)
    : mAction   (action)
    , mSessionId(sessionId)
    , mPrompt   (prompt)
    , mVideo    ( )
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt, const SharedBuffer& video)
    : mAction   (action)
    , mSessionId(sessionId)
    , mPrompt   (prompt)
    , mVideo    (video)
{
}

AgentProcessorEventData::AgentProcessorEventData(const AgentProcessorEventData& data)
    : mAction   (data.mAction)
    , mSessionId(data.mSessionId)
    , mPrompt   (data.mPrompt)
    , mVideo    (data.mVideo)
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData&& data) noexcept
    : mAction   (data.mAction)
    , mSessionId(data.mSessionId)
    , mPrompt   (std::move(data.mPrompt))
    , mVideo    (std::move(data.mVideo))
{
}

AgentProcessorEventData& AgentProcessorEventData::operator = (const AgentProcessorEventData& data)
{
    if (this != &data)
    {
        mAction     = data.mAction;
        mSessionId  = data.mSessionId;
        mPrompt     = data.mPrompt;
        mVideo      = data.mVideo;
    }

    return (*this);
}

AgentProcessorEventData& AgentProcessorEventData::operator = (AgentProcessorEventData&& data) noexcept
{
    if (this != &data)
    {
        mAction     = data.mAction;
        mSessionId  = data.mSessionId;
        mPrompt     = std::move(data.mPrompt);
        mVideo      = std::move(data.mVideo);
    }

    return (*this);
}

AgentProcessor::AgentProcessor(void)
    : IEWorkerThreadConsumer(NEMultiEdgeSettings::CONSUMER_NAME)
    , IEAgentProcessorEventConsumer( )
    , mCompThread           (nullptr)
    , mCurEvent             ( )
{
}

void AgentProcessor::registerEventConsumers(WorkerThread& workThread, ComponentThread& masterThread)
{
    mCompThread = &masterThread;
    AgentProcessorEvent::addListener(static_cast<IEAgentProcessorEventConsumer&>(*this), static_cast<DispatcherThread &>(workThread));
}

void AgentProcessor::unregisterEventConsumers(WorkerThread& workThread)
{
    mCompThread = nullptr;
    AgentProcessorEvent::removeListener(static_cast<IEAgentProcessorEventConsumer&>(*this), static_cast<DispatcherThread&>(workThread));
}

void AgentProcessor::processEvent(const AgentProcessorEventData& data)
{
    if (mCompThread == nullptr)
        return;

    if (data.getAction() == AgentProcessorEventData::ActionProcessText)
    {
        mCurEvent = data;
        String response = processText(data.getPrompt());
        AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::ActionReplyText, data.getSessionId(), response), static_cast<DispatcherThread &>(*mCompThread));
    }
}

String AgentProcessor::processText(const String& prompt)
{
    return String();
}
