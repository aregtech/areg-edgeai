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

#include "areg/component/NERegistry.hpp"
#include <QObject>

class AIAgent;

class AgentProvider : public QObject
                    , public Component
                    , public MultiEdgeStub
{
    Q_OBJECT

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

private:

    inline AgentProvider& self(void);
    
private:
    AIAgent*    mAIAgent;
};

#endif // MULTIEDGE_AIAGENT_AGENTPROVIDER_HPP
