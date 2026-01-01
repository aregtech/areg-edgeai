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

#include <QFileInfo>
#include <cmath>
#include <thread>

AgentProcessorEventData::AgentProcessorEventData(void)
    : mAction   (ActionUnknown)
    , mSessionId(0)
    , mPrompt   ( )
    , mModelPath( )
    , mVideo    ( )
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, const String& modelPath)
    : mAction   (action)
    , mSessionId(0xFFFFFFFFu)
    , mPrompt   ( )
    , mModelPath(modelPath)
    , mVideo    ( )
{
}
    
AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt)
    : mAction   (action)
    , mSessionId(sessionId)
    , mPrompt   (prompt)
    , mModelPath( )
    , mVideo    ( )
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt, const SharedBuffer& video)
    : mAction   (action)
    , mSessionId(sessionId)
    , mPrompt   (prompt)
    , mModelPath( )
    , mVideo    (video)
{
}

AgentProcessorEventData::AgentProcessorEventData(const AgentProcessorEventData& data)
    : mAction   (data.mAction)
    , mSessionId(data.mSessionId)
    , mPrompt   (data.mPrompt)
    , mModelPath(data.mModelPath)
    , mVideo    (data.mVideo)
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData&& data) noexcept
    : mAction   (data.mAction)
    , mSessionId(data.mSessionId)
    , mPrompt   (std::move(data.mPrompt))
    , mModelPath(std::move(data.mModelPath))
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
        mModelPath  = data.mModelPath;
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
        mModelPath  = std::move(data.mModelPath);
        mVideo      = std::move(data.mVideo);
    }

    return (*this);
}

AgentProcessor::AgentProcessor(void)
    : IEWorkerThreadConsumer(NEMultiEdgeSettings::CONSUMER_NAME)
    , IEAgentProcessorEventConsumer( )
    , mCompThread           (nullptr)
    , mCurEvent             ( )
    , mLLMParams            (llama_context_default_params())
    , mTextLimit            (512)
    , mTokenLimit           (2048)
    , mLLMModel             (nullptr)
    , mLLMHandle            (nullptr)
{
    mLLMParams.n_ctx = 2048;   // safe default, tune later
    mLLMParams.n_threads = std::max(1u, std::thread::hardware_concurrency());
}

void AgentProcessor::registerEventConsumers(WorkerThread& workThread, ComponentThread& masterThread)
{
    mCompThread = &masterThread;
    AgentProcessorEvent::addListener(static_cast<IEAgentProcessorEventConsumer&>(*this), static_cast<DispatcherThread &>(workThread));
}

void AgentProcessor::unregisterEventConsumers(WorkerThread& workThread)
{
    freeModel();
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
    else if (data.getAction() == AgentProcessorEventData::ActionActivateModel)
    {
        mModelPath = activateModel(data.getModelPath());
        AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::ActionModelActivated, mModelPath), static_cast<DispatcherThread &>(*mCompThread));
    }
}

String AgentProcessor::processText(const String& prompt)
{
    // TODO: implement text processing here
    return String();
}

String AgentProcessor::activateModel(const String& modelPath)
{
    if (modelPath.isEmpty())
        return String();

    const QString qPath = QString::fromUtf8(modelPath.getString());
    QFileInfo fi(qPath);
    if (!fi.exists() || !fi.isFile())
        return String();

    // Cleanup previous model/context
    freeModel();
    
    // Load model
    const QByteArray utf8Path = fi.absoluteFilePath().toUtf8();
    const char* pathUtf8 = utf8Path.constData();
    llama_model_params modelParams = llama_model_default_params();
    mLLMModel = llama_model_load_from_file(pathUtf8, modelParams);
    mLLMHandle = mLLMModel != nullptr ? llama_init_from_model(mLLMModel, mLLMParams) : nullptr;
    if (mLLMHandle == nullptr)
    {
        freeModel();
        return String();
    }

    return modelPath;
}

void AgentProcessor::freeModel()
{
    if (mLLMHandle != nullptr)
    {
        llama_free(mLLMHandle);
        mLLMHandle = nullptr;
    }
    
    if (mLLMModel != nullptr)
    {
        llama_model_free(mLLMModel);
        mLLMModel = nullptr;
    }
}
