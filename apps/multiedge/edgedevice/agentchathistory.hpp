#ifndef MULTIEDGE_EDGEDEVICE_AGENTCHATHISTORY_HPP
#define MULTIEDGE_EDGEDEVICE_AGENTCHATHISTORY_HPP
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
 *  \file        multiedge/edgedevice/agentchathistory.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service chat history model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QAbstractTableModel>
#include <vector>

class AgentChatHistory : public QAbstractTableModel
{
public:

    enum eChatColumn
    {
          ColumnInvalid     = -1
        , ColumnSource      = 0
        , ColumnMessage
        , ColumnTimestamp
        , ColumnStatus
    };

    enum eChatSource
    {
          SourceUnknown
        , SourceHuman
        , SourceEdgeAi
    };

    enum eMessageStatus
    {
          StatusInvalid
        , StatusPending
        , StatusReplied
        , StatusCanceled
        , StatusError
    };

    struct sChatEntry
    {
        eChatSource     chatSource  {eChatSource::SourceUnknown};
        QString         chatMessage {};
        uint64_t        chatTime    {0u};
        eMessageStatus  chatStatus  {eMessageStatus::StatusInvalid};
    };

    static constexpr    uint32_t    INIT_LENGTH {1000u};

    using ChatHistory   = std::vector<sChatEntry>;

public:
    explicit AgentChatHistory(QObject *parent = nullptr);

private:
    ChatHistory mRequests;
    ChatHistory mResponses;
    uint32_t    mSequence;
};

#endif // MULTIEDGE_EDGEDEVICE_AGENTCHATHISTORY_HPP
