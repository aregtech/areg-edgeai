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
#include "areg/logging/GELog.h"

#include <QFileInfo>
#include <algorithm>
#include <thread>

AgentProcessorEventData::AgentProcessorEventData(void)
    : mAction   (ActionUnknown)
    , mData     ()
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, const String& modelPath)
    : mAction   (action)
    , mData     ()
{
    mData << modelPath;
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, float temperature, float probability)
    : mAction   (action)
    , mData     ()
{
    mData << temperature;
    mData << probability;
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt)
    : mAction   (action)
    , mData     ()
{
    mData << sessionId;
    mData << prompt;
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt, const SharedBuffer& video)
    : mAction   (action)
    , mData     ()
{
    mData << sessionId;
    mData << prompt;
    mData << video;
}

AgentProcessorEventData::AgentProcessorEventData(const AgentProcessorEventData& data)
    : mAction   (data.mAction)
    , mData     (data.mData)
{
}

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData&& data) noexcept
    : mAction   (data.mAction)
    , mData     (std::move(data.mData))
{
}

AgentProcessorEventData& AgentProcessorEventData::operator = (const AgentProcessorEventData& data)
{
    if (this != &data)
    {
        mAction = data.mAction;
        mData   = data.mData;
    }

    return (*this);
}

AgentProcessorEventData& AgentProcessorEventData::operator = (AgentProcessorEventData&& data) noexcept
{
    if (this != &data)
    {
        mAction = data.mAction;
        mData   = std::move(data.mData);
    }

    return (*this);
}

AgentProcessor::AgentProcessor(void)
    : IEWorkerThreadConsumer(NEMultiEdgeSettings::CONSUMER_NAME)
    , IEAgentProcessorEventConsumer( )
    , mCompThread           (nullptr)
    , mSessionId            (0xFFFFFFFF)
    , mModelParams          (llama_model_default_params())
    , mTextLimit            (MAX_CHARS)
    , mTokenLimit           (MAX_TOKENS)
    , mThreads              (1u)
    , mTemperature          (DEF_TEMPERATURE)
    , mProbability          (DEF_PROBABILITY)
    , mLLMModel             (nullptr)
{
    uint32_t cores = std::thread::hardware_concurrency();
    mThreads = std::clamp(cores, MIN_THREADS, MAX_THREADS);
    mModelParams.n_gpu_layers = 99;
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
    freeModel();
}

void AgentProcessor::processEvent(const AgentProcessorEventData& data)
{
    if (mCompThread == nullptr)
        return;

    switch (data.getAction())
    {
    case AgentProcessorEventData::ActionProcessText:
    {
        const SharedBuffer& evData = data.getData();
        String prompt;
        evData >> mSessionId;
        evData >> prompt;
        String response = processText(prompt);
        AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::ActionReplyText, mSessionId, response), static_cast<DispatcherThread&>(*mCompThread));
    }
    break;

    case AgentProcessorEventData::ActionActivateModel:
    {
        const SharedBuffer& evData = data.getData();
        String modelPath;
        evData >> modelPath;
        mModelPath = activateModel(modelPath);
        AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::ActionModelActivated, mModelPath), static_cast<DispatcherThread&>(*mCompThread));
    }
    break;

    case AgentProcessorEventData::ActionTemperature:
    {
        const SharedBuffer& evData = data.getData();
        float temperature = 0.5f;
        float probability = 0.05f;
        evData >> temperature;
        evData >> probability;
        mTemperature = std::clamp(temperature, 0.09f, 1.2f);
        mProbability = std::clamp(probability, 0.01f, 0.2f);
    }
    break;

    default:
        break;
    }
}

DEF_LOG_SCOPE(multiedge_aiagent_AgentProcessor_processText);
String AgentProcessor::processText(const String& prompt)
{
    LOG_SCOPE(multiedge_aiagent_AgentProcessor_processText);
    if (prompt.isEmpty() || mLLMModel == nullptr)
    {
        LOG_ERR("The prompt is empty (len = %d) or LLM model is not activated (%s null), cannot process text...", prompt.getLength(), mLLMModel != nullptr ? "IS NOT" : "IS");
        return String();
    }

    const llama_vocab* vocab = llama_model_get_vocab(mLLMModel);

    // Create a fresh context per request
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx    = mTextLimit;
    ctx_params.n_batch  = mTokenLimit;
    ctx_params.n_threads= mThreads;
    ctx_params.no_perf  = true;

    String response;
    llama_context* ctx = llama_init_from_model(mLLMModel, ctx_params);
    if (!ctx) 
    {
        LOG_ERR("Failed to create LLM context from model");
        return response;
    }

    // create sampler
    llama_sampler* smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(smpl, llama_sampler_init_min_p(mProbability, 1));
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(mTemperature));
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    // Tokenize prompt
    const bool add_bos = true;
    const int n_prompt = -llama_tokenize(vocab, prompt.getString(), prompt.getLength(), nullptr, 0, add_bos, true);
    if (n_prompt <= 0) 
    {
        LOG_ERR("Failed to tokenize prompt, returned value %d", n_prompt);
        llama_sampler_free(smpl);
        llama_free(ctx);
        return response;
    }

    std::vector<llama_token> tokens(n_prompt);
    if (llama_tokenize(vocab, prompt.getString(), prompt.getLength(), tokens.data(), tokens.size(), add_bos, true) < 0)
    {
        LOG_ERR("Failed to tokenize prompt, returned value %d", n_prompt);
        llama_sampler_free(smpl);
        llama_free(ctx);
        return response;
    }

    // Decode prompt
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (llama_decode(ctx, batch) != 0)
    {
        LOG_ERR("Failed to decode prompt");
        llama_sampler_free(smpl);
        llama_free(ctx);
        return response;
    }

    // Generation loop
    response.reserve(MAX_CHARS);
    llama_token token;
    char buf[256];
    std::string pieces;
    pieces.reserve(256);
    for (int i = 0; i < mTokenLimit; ++i)
    {
        token = llama_sampler_sample(smpl, ctx, -1);
        if (llama_vocab_is_eog(vocab, token))
        {
            LOG_INFO("Adding last piece [ %s ]", pieces.c_str());
            response += pieces;
            LOG_DBG("End of generation token reached, interrupting text processing.");
            break;
        }

        int n = llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true);
        if (n <= 0)
        {
            LOG_ERR("Failed to convert token to piece, token %d, ret value [ %d ]", token, n);
            break;
        }

        pieces.append(buf, n);
        if (pieces.size() >= 256)
        {
            LOG_INFO("Appending pieces: [ %s ]", pieces.c_str());
            response += pieces;
            pieces.clear();
            pieces.reserve(256);
            if (response.getLength() >= mTextLimit)
            {
                LOG_WARN("Maximum character limit reached, interrupting text processing.");
                break;
            }
        }

        batch = llama_batch_get_one(&token, 1);
        if (llama_decode(ctx, batch) != 0)
        {
            response += pieces;
            LOG_ERR("Failed to decode token");
            break;
        }
    }

    // Cleanup
    llama_sampler_free(smpl);
    llama_free(ctx);

    return response;
}

DEF_LOG_SCOPE(multiedge_aiagent_AgentProcessor_activateModel);
String AgentProcessor::activateModel(const String& modelPath)
{
    LOG_SCOPE(multiedge_aiagent_AgentProcessor_activateModel);
    
    if (modelPath.isEmpty())
    {
        LOG_ERR("The model path is empty, cannot activate model...");
        return String();
    }

    const QString qPath = QString::fromUtf8(modelPath.getString());
    QFileInfo fi(qPath);
    if (!fi.exists() || !fi.isFile())
    {
        LOG_ERR("The model file does not exist at path [%s], cannot active model...", modelPath.getString());
        return String();
    }

    // Release previous model
    freeModel();

    const QByteArray utf8Path = fi.absoluteFilePath().toUtf8();
    mLLMModel = llama_model_load_from_file(utf8Path.constData(), mModelParams);
    if (mLLMModel == nullptr)
    {
        LOG_ERR("Failed to load LLM model from file [%s]", utf8Path.constData());
        return String();
    }

    // Context is NOT created here on purpose
    // Contexts are per-request to avoid topic mixing

    LOG_DBG("Successfully activated LLM model from file [%s]", utf8Path.constData());
    return modelPath;
}

void AgentProcessor::freeModel()
{
    if (mLLMModel != nullptr)
    {
        llama_model_free(mLLMModel);
        mLLMModel = nullptr;
    }
}
