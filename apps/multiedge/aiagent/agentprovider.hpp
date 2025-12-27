#ifndef MULTIEDGE_AIAGENT_AGENTPROVIDER_HPP
#define MULTIEDGE_AIAGENT_AGENTPROVIDER_HPP
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
 *  \file        multiedge/aiagent/agentprovider.hpp
 *  \ingroup     Areg Edge AI, AI Multi Edge Device Agent
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service provider.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "areg/base/GEGlobal.h"
#include "areg/component/Component.hpp"
#include "multiedge/resources/MultiEdgeStub.hpp"
#include <QObject>
#include "multiedge/aiagent/agentprocessor.hpp"

class AIAgent;

class AgentProvider : public QObject
                    , public Component
                    , public MultiEdgeStub
                    , protected IEAgentProcessorEventConsumer
{
    Q_OBJECT

private:
    struct sTextPrompt
    {
        SessionID   sessionId{ 0 };
        uint32_t    agentSession{0};
        uint32_t    agentId{0};
        String      prompt{};
    };

    using ListSession = std::vector<sTextPrompt>;

    enum eAgentState
    {
          StateReady
        , StateBusy
    };
//////////////////////////////////////////////////////////////////////////
// Internal types, constants and static methods
//////////////////////////////////////////////////////////////////////////
public:
    static AgentProvider* getService(void);
    
public:
    AgentProvider(const NERegistry::ComponentEntry& entry, ComponentThread& owner);
    virtual ~AgentProvider(void) = default;

//////////////////////////////////////////////////////////////////////////
// MultiEdge Interface Requests
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Request call.
     *          The request sent by edge device to process the text.
     * \param   sessionId   A unique ID of the session to distinguish the requests. The ID is sent back by the response.
     * \param   agentId     The ID of edge device. It is sent back to the edge device to confirm target device that the request is processed.
     * \param   textProcess The text to process.
     * \see     responseProcessText
     **/
    virtual void requestProcessText(unsigned int sessionId, unsigned int agentId, const String& textProcess) override;

    /**
     * \brief   Request call.
     *          Process a video data
     * \param   sessionId   An ID set by edge device to process video. The ID is unique on edge device side.
     * \param   agentId     The ID of edge device. It is sent back to the edge device to confirm target device that the request is processed.
     * \param   cmdText     A command as a string to request to process video data.
     * \param   dataVideo   A binary buffer of video data to process.
     * \see     responseProcessVideo
     **/
    virtual void requestProcessVideo(unsigned int sessionId, bool agentId, const String& cmdText, const SharedBuffer& dataVideo) override;

protected:

    /**
     * \brief   Returns pointer to Worker Thread Consumer object identified
     *          by consumer name and if needed, by worker thread name.
     *          This function is triggered, when component is initialized and
     *          worker threads should be created.
     * \param   consumerName        The name of worker thread consumer object to identify
     * \param   workerThreadName    The name of worker thread, which consumer should return
     * \return  Return valid pointer if worker thread has assigned consumer.
     **/
    virtual IEWorkerThreadConsumer* workerThreadConsumer(const String& consumerName, const String& workerThreadName) override;


    /**
     * \brief  Override operation. Implement this function to receive events and make processing
     * \param  data    The data, which was passed as an event.
     **/
    virtual void processEvent( const AgentProcessorEventData & data ) override;

protected:
/************************************************************************/
// StubBase overrides. Triggered by Component on startup.
/************************************************************************/

    /**
     * \brief   This function is triggered by Component when starts up.
     *          Overwrite this method and set appropriate request and
     *          attribute update notification event listeners here
     * \param   holder  The holder component of service interface of Stub,
     *                  which started up.
     **/
    virtual void startupServiceInterface( Component & holder ) override;

    /**
     * \brief   This function is triggered by Component when shuts down.
     *          Overwrite this method to remove listeners and stub cleanup
     * \param   holder  The holder component of service interface of Stub,
     *                  which shuts down.
     **/
    virtual void shutdownServiceInterface ( Component & holder ) override;

private:

    inline AgentProvider& self(void);
    
private:
    AIAgent*        mAIAgent;
    eAgentState     mAgentState;
    ListSession     mListSessions;
    String          mWorkerThread;
    AgentProcessor  mAgentProcessor;
};

#endif // MULTIEDGE_AIAGENT_AGENTPROVIDER_HPP
