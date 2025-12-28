#ifndef MULTIEDGE_EDGEDEVICE_AGENTCONSUMER_HPP
#define MULTIEDGE_EDGEDEVICE_AGENTCONSUMER_HPP
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
 *  \file        multiedge/edgedevice/agentconsumer.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service consumer.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "areg/base/GEGlobal.h"
#include "areg/component/Component.hpp"
#include "multiedge/resources/MultiEdgeClientBase.hpp"
#include <QObject>

#include "areg/component/NERegistry.hpp"
#include <QString>
#include <string_view>

class EdgeDevice;

class AgentConsumer : public QObject
                    , public Component
                    , public MultiEdgeClientBase
{

    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:

    static bool processText(uint32_t id, const QString& text);

    static bool processVideo(uint32_t id, const QString& cmdText, const SharedBuffer& video);

    static NERegistry::Model createModel(const QString& name, EdgeDevice * context);
    
    static AgentConsumer* getService(void);

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    AgentConsumer(const NERegistry::ComponentEntry& entry, ComponentThread& owner);
    virtual ~AgentConsumer(void);

signals:

    void signalServiceConnected(bool isConnected);

    void signalAgentQueueSize(uint32_t queueSize);

    void signalAgentType(NEMultiEdge::eEdgeAgent EdgeAgent);

    void signalTextProcessed(uint32_t id, QString reply, uint64_t stamp);

    void signalVideoProcessed(uint32_t id, SharedBuffer video);

    void signalAgentProcessingFailed(NEMultiEdge::eEdgeAgent agent, NEService::eResultType reason);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Triggered when receives service provider connected / disconnected event.
     *          When the service provider is connected, the client objects can set the listeners here.
     *          When the service provider is disconnected, the client object should clean the listeners.
     *          Up from connected status, the clients can subscribe and unsubscribe on updates,
     *          responses and broadcasts, can trigger requests. Before connection, the clients cannot
     *          neither trigger requests, nor receive data update messages.
     * \param   status  The service connection status.
     * \param   proxy   The Service Interface Proxy object, which is notifying service connection.
     * \return  Return true if this service connect notification was relevant to client object.
     **/
    virtual bool serviceConnected( NEService::eServiceConnection status, ProxyBase & proxy ) override;

    /**
     * \brief   Triggered, when QueueSize attribute is updated. The function contains
     *          attribute value and validation flag. When notification is enabled,
     *          the method should be overwritten in derived class.
     *          Attributes QueueSize description:
     *          The current size of the queue with pending requests to process.
     * \param   QueueSize   The value of QueueSize attribute.
     * \param   state       The data validation flag.
     **/
    virtual void onQueueSizeUpdate( unsigned int QueueSize, NEService::eDataStateType state ) override;

    /**
     * \brief   Triggered, when EdgeAgent attribute is updated. The function contains
     *          attribute value and validation flag. When notification is enabled,
     *          the method should be overwritten in derived class.
     *          Attributes EdgeAgent description:
     *          The type of active Edge AI agent
     * \param   EdgeAgent   The value of EdgeAgent attribute.
     * \param   state       The data validation flag.
     **/
    virtual void onEdgeAgentUpdate( NEMultiEdge::eEdgeAgent EdgeAgent, NEService::eDataStateType state ) override;

/************************************************************************
 * Responses
 ************************************************************************/
    /**
     * \brief   Response callback.
     *          Response sent from Edge AI to the edge device as a result of request to process a text. Valid for the LLM
     *          Overwrite, if need to handle Response call of server object.
     *          This call will be automatically triggered, on every appropriate request call
     * \param   sessionId   A unique ID of the session set by the edge device, received from request.
     * \param   agentId     The ID of edge device received in request, it is sent back to the edge device to confirm target device that the request is processed.
     * \param   textReplied The text replied by the Edge AI.
     * \see     requestProcessText
     **/
    virtual void responseProcessText( unsigned int sessionId, unsigned int agentId, const String & textReplied );

    /**
     * \brief   Response callback.
     *          Response of processing a video data.
     *          Overwrite, if need to handle Response call of server object.
     *          This call will be automatically triggered, on every appropriate request call
     * \param   sessionId   A unique ID of the session set by the edge device, received from request.
     * \param   agentId     The ID of edge device received in request, it is sent back to the edge device to confirm target device that the request is processed.
     * \param   dataVideo   A response data to send back to the edge device.
     * \see     requestProcessVideo
     **/
    virtual void responseProcessVideo(unsigned int sessionId, unsigned int agentId, const SharedBuffer& dataVideo);

    /**
     * \brief   Overwrite to handle error of ProcessText request call.
     * \param   FailureReason   The failure reason value of request call.
     **/
    virtual void requestProcessTextFailed( NEService::eResultType FailureReason ) override;

    /**
     * \brief   Overwrite to handle error of ProcessVideo request call.
     * \param   FailureReason   The failure reason value of request call.
     **/
    virtual void requestProcessVideoFailed( NEService::eResultType FailureReason ) override;

private:
    static String   mConsumerName;  //!< The service name of the Agent Consumer
    uint32_t        mConsumerId;    //!< The unique ID of the consumer within the network.
    EdgeDevice*     mEdgeDevice;    //!< The pointer to the main dialog window.
};

#endif // MULTIEDGE_EDGEDEVICE_AGENTCONSUMER_HPP
