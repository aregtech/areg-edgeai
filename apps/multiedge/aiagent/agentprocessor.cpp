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

//////////////////////////////////////////////////////////////////////////
// AgentProcessorEventData event data class implementation
//////////////////////////////////////////////////////////////////////////

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

AgentProcessorEventData::AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t maxText, uint32_t maxTokens, uint32_t maxBatch, uint32_t maxThreads)
    : mAction   (action)
    , mData     ()
{
    mData << maxText;
    mData << maxTokens;
    mData << maxBatch;
    mData << maxThreads;
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

//////////////////////////////////////////////////////////////////////////
// AgentProcessor class implementation
//////////////////////////////////////////////////////////////////////////
DEF_LOG_SCOPE(multiedge_aiagent_AgentProcessor_processEvent);
DEF_LOG_SCOPE(multiedge_aiagent_AgentProcessor_processText);
DEF_LOG_SCOPE(multiedge_aiagent_AgentProcessor_activateModel);

uint32_t AgentProcessor::optThreadCount(void)
{
    uint32_t cores = std::thread::hardware_concurrency();
    return (cores != 0 ? cores : MIN_THREADS);
}

uint32_t AgentProcessor::defThreadCount(void)
{
    uint32_t cores = AgentProcessor::optThreadCount();
    return std::clamp(cores, MIN_THREADS, MAX_THREADS);
}

AgentProcessor::AgentProcessor(void)
    : IEWorkerThreadConsumer(NEMultiEdgeSettings::CONSUMER_NAME)
    , IEAgentProcessorEventConsumer( )
    , mCompThread           (nullptr)
    , mSessionId            (0xFFFFFFFF)
    , mModelParams          (llama_model_default_params())
    , mTextLimit            (DEF_CHARS)
    , mTokenLimit           (DEF_TOKENS)
    , mBatching             (DEF_BATCHING)
    , mThreads              (AgentProcessor::defThreadCount())
    , mTemperature          (DEF_TEMPERATURE)
    , mProbability          (DEF_PROBABILITY)
    , mLLMModel             (nullptr)
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
    freeModel();
}

void AgentProcessor::processEvent(const AgentProcessorEventData& data)
{
    LOG_SCOPE(multiedge_aiagent_AgentProcessor_processEvent);
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
        LOG_DBG("Processing prompt [ %s ]", prompt.getString());
        String response = processText(prompt);
        AgentProcessorEvent::sendEvent(AgentProcessorEventData(AgentProcessorEventData::ActionReplyText, mSessionId, response), static_cast<DispatcherThread&>(*mCompThread));
    }
    break;

    case AgentProcessorEventData::ActionActivateModel:
    {
        const SharedBuffer& evData = data.getData();
        String modelPath;
        evData >> modelPath;
        LOG_INFO("Loading model [ %s ]", modelPath.getString());
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
        mTemperature = std::clamp(temperature, MIN_TEMPERATURE, MAX_TEMPERATURE);
        mProbability = std::clamp(probability, MIN_PROBABILITY, MAX_PROBABILITY);
        LOG_INFO("Set temperature to [ %.2f ] and probability to [ %.2f ]", mTemperature, mProbability);
    }
    break;
        
    case AgentProcessorEventData::ActionSetLimits:
    {
        const SharedBuffer& evData = data.getData();
        uint32_t maxText    { DEF_CHARS };
        uint32_t maxToken   { DEF_TOKENS };
        uint32_t maxBatch   { DEF_BATCHING };
        uint32_t maxThread  { DEF_THREADS };
        evData >> maxText >> maxToken >> maxBatch >> maxThread;
        mTextLimit  = std::clamp(maxText    , MIN_CHARS     , MAX_CHARS);
        mTokenLimit = std::clamp(maxToken   , MIN_TOKENS    , MAX_TOKENS);
        mBatching   = std::clamp(maxBatch   , MIN_BATCHING  , MAX_BATCHING);
        mThreads    = std::clamp(maxThread  , MIN_THREADS   , AgentProcessor::optThreadCount());
        LOG_INFO("Set limits - Text: [ %u ], Tokens: [ %u ], Batching: [ %u ], Threads: [ %u ]", mTextLimit, mTokenLimit, mBatching, mThreads);
    }
    break;

    default:
    {
        LOG_WARN("Unknown action received: %d", data.getAction());
    }
    break;
    }
}

String AgentProcessor::processText(const String& prompt)
{
    LOG_SCOPE(multiedge_aiagent_AgentProcessor_processText);

    String response;
    if (prompt.isEmpty() || mLLMModel == nullptr)
    {
        LOG_ERR("Prompt empty or model not activated");
        return response;
    }

    const llama_vocab* vocab = llama_model_get_vocab(mLLMModel);

    // Create a fresh context per request
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx    = mTextLimit;
    ctx_params.n_batch  = mBatching;
    ctx_params.n_threads= mThreads;
    ctx_params.no_perf  = true;
    llama_context* ctx = llama_init_from_model(mLLMModel, ctx_params);
    if (ctx == nullptr)
    {
        LOG_ERR("Failed to create llama context");
        return response;
    }

    // Sampler chain (correct order)
    llama_sampler* smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    // Light repetition control (important for agents)
    llama_sampler*  penalties = llama_sampler_init_penalties( /* repeat_last_n */ 64
                                                             , /* repeat_penalty */ 1.10f
                                                             , /* freq_penalty   */ 0.0f
                                                             , /* present_penalty*/ 0.0f);
    llama_sampler_chain_add(smpl, penalties );
    if (mTemperature == 0.0f)
    {
        llama_sampler_chain_add(smpl, llama_sampler_init_greedy());
    }
    else
    {
        // Temperature (low = precise)
        llama_sampler_chain_add(smpl, llama_sampler_init_temp(mTemperature));
        // min_p filtering (FIXED: min_keep > 1)
        llama_sampler_chain_add(smpl, llama_sampler_init_min_p(mProbability, 5));
        // Final distribution
        llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    }

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
        LOG_ERR("Tokenization failed");
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
    response.reserve(mTextLimit);
    char buf[DEF_CHARS];
    String sentence;
    sentence.reserve(DEF_CHARS);
    constexpr std::string_view space{" "};

    const uint32_t tokenLimit = (mTemperature <= 0.2f) ? MIN_TOKENS : mTokenLimit;
    for (uint32_t i = 0; i < tokenLimit; ++i)
    {
        llama_token token = llama_sampler_sample(smpl, ctx, -1);

        if (llama_vocab_is_eog(vocab, token))
        {
            sentence.trimAll();
            LOG_INFO("Adding last piece [ %s ]", sentence.getString());
            response += sentence;
            LOG_DBG("End of generation token reached, interrupting text processing.");
            break;
        }

        int n = llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true);
        if (n <= 0)
        {
            LOG_ERR("Failed to convert token to piece, token %d, ret value [ %d ]", token, n);
            break;
        }

        sentence.append(buf, n);
        const char ch{ sentence.isEmpty() ? '\0' : sentence.getData().back()};
        if ((ch == '.') || (ch == '!') || (ch == '?'))
        {
            sentence.trimAll();
            LOG_INFO("Appending sentence: [ %s ]", sentence.getString());
            response += sentence;
            if (mTemperature == 0.0f)
            {
                // On greedy mode, flush per sentence
                LOG_WARN("Greedy mode - flushing per sentence.");
                break;
            }
            else if (response.getLength() >= mTextLimit)
            {
                LOG_WARN("Maximum character limit reached, interrupting text processing.");
                break;
            }
            
            response += space;
            sentence.clear();
            sentence.reserve(DEF_CHARS);
        }
        else if (sentence.getLength() >= 300)
        {
            sentence.trimAll();
            LOG_INFO("Appending sentence: [ %s ]", sentence.getString());
            response += sentence;
            response += space;
            sentence.clear();
            sentence.reserve(DEF_CHARS);
        }

        batch = llama_batch_get_one(&token, 1);
        if (llama_decode(ctx, batch) != 0)
        {
            response += sentence;
            LOG_ERR("Failed to decode token");
            break;
        }
    }

    // cleanup
    llama_sampler_free(smpl);
    llama_free(ctx);

    return response;
}

String AgentProcessor::activateModel(const String& modelPath)
{
    LOG_SCOPE(multiedge_aiagent_AgentProcessor_activateModel);

    if (modelPath.isEmpty())
        return String();

    QFileInfo fi(QString::fromUtf8(modelPath.getString()));
    if (!fi.exists() || !fi.isFile())
        return String();

    freeModel();

    mModelParams.n_gpu_layers= 99; // safe default, ignored on CPU
    mModelParams.use_mmap    = true;
    mModelParams.use_mlock   = true;

    const QByteArray path = fi.absoluteFilePath().toUtf8();
    mLLMModel = llama_model_load_from_file(path.constData(), mModelParams);
    if (mLLMModel == nullptr)
    {
        LOG_ERR("Model load failed");
        return String();
    }

    // Context is NOT created here on purpose
    // Contexts are per-request to avoid topic mixing
    LOG_DBG("Model activated: %s", path.constData());
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
