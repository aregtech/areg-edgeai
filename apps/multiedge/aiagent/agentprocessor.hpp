#ifndef MULTIEDGE_AIAGENT_AGENTPROCESSOR_HPP
#define MULTIEDGE_AIAGENT_AGENTPROCESSOR_HPP
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
 *  \file        multiedge/aiagent/agentprocessor.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent worker thread consumer object to process LLM.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/

#include "areg/base/GEGlobal.h"
#include "areg/component/IEWorkerThreadConsumer.hpp"
#include "areg/component/TEEvent.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "llama.h"

class AgentProvider;

class AgentProcessorEventData
{
public:
    enum eAction
    {
          ActionUnknown
        , ActionProcessText
        , ActionProcessVideo
        , ActionReplyText
        , ActionReplyVideo
        , ActionActivateModel
        , ActionModelActivated
        , ActionTemperature
    };

public:
    AgentProcessorEventData(void);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, const String& modelPath);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, float temperature, float probability);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt, const SharedBuffer& video);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt);
    AgentProcessorEventData(const AgentProcessorEventData& data);
    AgentProcessorEventData(AgentProcessorEventData&& data) noexcept;
    ~AgentProcessorEventData(void) = default;

public:
    AgentProcessorEventData& operator = (const AgentProcessorEventData& data);

    AgentProcessorEventData& operator = (AgentProcessorEventData&& data) noexcept;

    inline AgentProcessorEventData::eAction getAction(void) const;
    
    inline SharedBuffer & getData(void);

    inline const SharedBuffer& getData(void) const;

    inline void reset(void);

private:
    eAction         mAction;
    SharedBuffer    mData;
};

DECLARE_EVENT(AgentProcessorEventData, AgentProcessorEvent, IEAgentProcessorEventConsumer);

class AgentProcessor    : public IEWorkerThreadConsumer
                        , public IEAgentProcessorEventConsumer
{
private:
    static constexpr uint32_t MAX_CHARS         { 1024u };
    static constexpr uint32_t MAX_TOKENS        { 128u  };
    static constexpr uint32_t MAX_THREADS       { 16u   };
    static constexpr uint32_t MIN_THREADS       { 2u    };
    static constexpr float    DEF_TEMPERATURE   { 0.10f };
    static constexpr float    DEF_PROBABILITY   { 0.08f };

public:
    AgentProcessor(void);
    virtual ~AgentProcessor(void) = default;
protected:

/************************************************************************/
// IEWorkerThreadConsumer overrides
/************************************************************************/

    /**
     * \brief   Triggered by Worker Thread when starts running.
     *          Make initializations and add event consumers in this
     *          method to start receiving events.
     * \param   workThread      The Worker Thread object to notify startup
     * \param   masterThread    The component thread, which owns worker thread.
     **/
    virtual void registerEventConsumers( WorkerThread & workThread, ComponentThread & masterThread ) override;

    /**
     * \brief   Triggered by Worker Thread when stops running.
     *          Make cleanups and remove event consumers in this
     *          method to stop receiving events.
     * \param   workThread  The Worker Thread object to notify stop
     **/
    virtual void unregisterEventConsumers( WorkerThread & workThread ) override;

    /**
     * \brief  Override operation. Implement this function to receive events and make processing
     * \param  data    The data, which was passed as an event.
     **/
    virtual void processEvent( const AgentProcessorEventData & data ) override;
    
private:
    String processText(const String & prompt);
    
    /**
     * \brief   Activates or loads the LLM model to be used by the agent.
     *
     * This function prepares the internal LLM context for inference using the
     * model specified by \a modelPath. It may release any previously loaded
     * model and update the internal state (e.g. model path, context handle
     * and related parameters) to reflect the newly activated model.
     *
     * \param   modelPath   Filesystem path to the LLM model to activate.
     *
     * \return  A status or informational string related to the activation
     *          operation (for example, the effective model path or error
     *          description).
     **/
    String activateModel(const String& modelPath);
    
    //!< Releases the currently active LLM model and associated context.
    void freeModel();
    
    inline AgentProcessor& self();
    
private:
    ComponentThread*        mCompThread;
    uint32_t                mSessionId;
    uint32_t                mUserId;
    String                  mModelPath;
    llama_model_params      mModelParams;
    uint32_t                mTextLimit;
    uint32_t                mTokenLimit;
    uint32_t                mThreads;
    float                   mTemperature;
    float                   mProbability;
    llama_model*            mLLMModel;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline AgentProcessorEventData::eAction AgentProcessorEventData::getAction(void) const
{
    return mAction;
}

inline SharedBuffer& AgentProcessorEventData::getData(void)
{
    return mData;
}

inline const SharedBuffer& AgentProcessorEventData::getData(void) const
{
    return mData;
}

inline void AgentProcessorEventData::reset(void)
{
    mAction = ActionUnknown;
    mData.invalidate();
}

#endif // MULTIEDGE_AIAGENT_AGENTPROCESSOR_HPP
