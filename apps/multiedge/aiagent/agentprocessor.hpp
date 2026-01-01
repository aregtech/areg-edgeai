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
    };

public:
    AgentProcessorEventData(void);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, const String& modelPath);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt, const SharedBuffer& video);
    AgentProcessorEventData(AgentProcessorEventData::eAction action, uint32_t sessionId, const String& prompt);
    AgentProcessorEventData(const AgentProcessorEventData& data);
    AgentProcessorEventData(AgentProcessorEventData&& data) noexcept;
    ~AgentProcessorEventData(void) = default;

public:
    AgentProcessorEventData& operator = (const AgentProcessorEventData& data);

    AgentProcessorEventData& operator = (AgentProcessorEventData&& data) noexcept;

    inline AgentProcessorEventData::eAction getAction(void) const;

    inline const String& getPrompt(void) const;

    inline const SharedBuffer& getVideo(void) const;

    inline uint32_t getSessionId(void) const;
    
    inline const String& getModelPath(void) const;

    inline void reset(void);

private:
    eAction         mAction;  
    uint32_t        mSessionId;
    String          mPrompt;
    String          mModelPath;
    SharedBuffer    mVideo;
};

DECLARE_EVENT(AgentProcessorEventData, AgentProcessorEvent, IEAgentProcessorEventConsumer);

class AgentProcessor    : public IEWorkerThreadConsumer
                        , public IEAgentProcessorEventConsumer
{
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
    
    String activateModel(const String& modelPath);

    inline AgentProcessor& self();
    
private:
    ComponentThread*        mCompThread;
    AgentProcessorEventData mCurEvent;
    String                  mModelPath;
    llama_context_params    mLLMParams;
    uint32_t                mTextLimit;
    uint32_t                mTokenLimit;
    llama_context*          mLLMHandle;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline AgentProcessorEventData::eAction AgentProcessorEventData::getAction(void) const
{
    return mAction;
}

inline const String& AgentProcessorEventData::getPrompt(void) const
{
    return mPrompt;
}

inline const SharedBuffer& AgentProcessorEventData::getVideo(void) const
{
    return mVideo;
}

inline uint32_t AgentProcessorEventData::getSessionId(void) const
{
    return mSessionId;
}

inline const String& AgentProcessorEventData::getModelPath(void) const
{
    return mModelPath;
}

inline void AgentProcessorEventData::reset(void)
{
    mAction = ActionUnknown;
    mSessionId = 0;
    mPrompt.clear();
    mVideo.invalidate();
}

#endif // MULTIEDGE_AIAGENT_AGENTPROCESSOR_HPP
