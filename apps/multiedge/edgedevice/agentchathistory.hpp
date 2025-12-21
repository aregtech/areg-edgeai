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
#include <QIcon>

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
        , ColumnCount
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
        , StatusIgnore
    };

    struct sChatEntry
    {
        eChatSource     chatSource  {eChatSource::SourceUnknown};
        QString         chatMessage {};
        uint64_t        chatTime    {0u};
        eMessageStatus  chatStatus  {eMessageStatus::StatusInvalid};
        uint32_t        chatId      {0xFFFFFFFF};
    };

    static constexpr    uint32_t    INIT_LENGTH {1000u};

    using ChatHistory   = std::vector<sChatEntry>;

public:
    explicit AgentChatHistory(QObject *parent = nullptr);
    
public:
    uint32_t addRequest(const QString& question);
    
    bool addResponse(const QString& reply, uint32_t seqId);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    // Header:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // Add/Remove data:
    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    
private:
    
    const QString & displayName(const sChatEntry & entry, uint64_t next, int column) const;
    
    int findEntry(uint32_t seqId, int32_t startAt);

private:
    ChatHistory mHistory;
    uint32_t    mSequence;
    QIcon       mIconHuman;
    QIcon       mIconRobot;
};

#endif // MULTIEDGE_EDGEDEVICE_AGENTCHATHISTORY_HPP
