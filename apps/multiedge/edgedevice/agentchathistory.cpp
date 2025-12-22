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
 *  \file        multiedge/edgedevice/agentchathistory.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service chat history model.
 *
 ************************************************************************/

#include "multiedge/edgedevice/agentchathistory.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/NEUtilities.hpp"
#include <functional>

namespace {
    const QString _columns[static_cast<int>(AgentChatHistory::ColumnCount)]
    {
          "Source"
        , "Message"
        , "Timestamp"
        , "Status"
    };

    const QString _source[]
    { 
          "Unknown:"
        , "Me:"
        , "AI:"
    };

    const QString _status[]
    {
          "Invalid"
        , "Pending"
        , "Replied"
        , "Canceled"
        , "Error"
        , "Ignore"
    };


    constexpr uint32_t _widths[static_cast<int>(AgentChatHistory::ColumnCount)]
    {
          50u
        , 250u
        , 100u
        , 50u
    };

}

AgentChatHistory::AgentChatHistory(QObject *parent)
    : QAbstractTableModel   (parent)
    , mHistory              (INIT_LENGTH)
    , mSequence             (0u)
    , mIconHuman            (":/icons/icon-human-question")
    , mIconRobot            (":/icons/icon-robot-ai")
    , mIconError            (":/icons/icon-error")
    , mIconCancel           (":/icons/icon-cancel")
{
}

QVariant AgentChatHistory::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Orientation::Vertical) || (section < 0) || (section >= static_cast<int>(eChatColumn::ColumnCount)))
        return QVariant();

    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
        return QVariant(_columns[section]);

    case Qt::ItemDataRole::UserRole:
        return QVariant(section);

    case Qt::ItemDataRole::SizeHintRole:
        return _widths[section];

    default:
        return QVariant();
    }
}

int AgentChatHistory::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(mHistory.size());
}

int AgentChatHistory::columnCount(const QModelIndex& parent) const
{
    return static_cast<int>(eChatColumn::ColumnCount);
}

bool AgentChatHistory::insertRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool AgentChatHistory::insertColumns(int column, int count, const QModelIndex& parent)
{
    Q_UNUSED(column)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool AgentChatHistory::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool AgentChatHistory::removeColumns(int column, int count, const QModelIndex& parent)
{
    Q_UNUSED(column)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

QVariant AgentChatHistory::data(const QModelIndex& index, int role) const
{
    if ((index.isValid() == false) || mHistory.empty())
        return QVariant();
    
    int row = index.row();
    int col = index.column();
    
    int size = static_cast<int>(mHistory.size());
    if ((row < 0) || (row >= size))
        return QVariant();
    
    if ((col < 0) || (col >= static_cast<int>(eChatColumn::ColumnCount)))
        return QVariant();
    
    const sChatEntry& entry = mHistory[row];
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::DisplayRole:
        if ((col == static_cast<int>(eChatColumn::ColumnTimestamp)) && (row < static_cast<int>(mHistory.size())))
        {
            const sChatEntry& next = mHistory[row + 1];
            return QVariant(displayName(entry,  next.chatSource == eChatSource::SourceEdgeAi ? next.chatTime : 0u, col));
        }
        else
        {
            return QVariant(displayName(entry, 0u, col));
        }
        
    case Qt::DecorationRole:
    {
        switch (entry.chatStatus)
        {
        case eMessageStatus::StatusInvalid:
        case eMessageStatus::StatusError:
            return mIconError;

        case eMessageStatus::StatusCanceled:
        case eMessageStatus::StatusIgnore:
            return mIconCancel;

        default:
            return (entry.chatSource == eChatSource::SourceEdgeAi ? mIconRobot : mIconHuman);
        }
    }
        
    case Qt::UserRole:
        return QVariant(static_cast<int>(entry.chatSource));
        
    default:
        return QVariant();
    }
}

const QString & AgentChatHistory::displayName(const sChatEntry & entry, uint64_t next, int column) const
{
    static QString _time;
    std::function fn {[this](uint64_t val, uint64_t next) -> QString{
        QString result;
        DateTime tm(val);
        String res = tm.formatTime();
        result.fromStdString(res.getData());
        result += " | ";
        if (next < val) {
            result += QString::number(next - val);
        }
        
        return result;
    }};
    
    _time.clear();    
    switch (static_cast<eChatColumn>(column))
    {
    case eChatColumn::ColumnSource:
        return _source[static_cast<int>(entry.chatSource)];
    case eChatColumn::ColumnMessage:
        return entry.chatMessage;
    case eChatColumn::ColumnTimestamp:
        _time = fn(entry.chatTime, next);
        return _time;
    case eChatColumn::ColumnStatus:
        return _status[static_cast<int>(entry.chatStatus)];
    default:
        return _time;
    }
}

uint32_t AgentChatHistory::addRequest(const QString& question)
{
    beginInsertRows(QModelIndex(), mSequence, mSequence);
    sChatEntry entry{eChatSource::SourceHuman, question, DateTime::getNow(), eMessageStatus::StatusPending, mSequence};
    mHistory.push_back(entry);
    endInsertRows();
    return mSequence ++;
}

bool AgentChatHistory::addResponse(const QString& reply, uint32_t seqId)
{
    sChatEntry entry{eChatSource::SourceEdgeAi, reply, DateTime::getNow(), eMessageStatus::StatusReplied, seqId};
    int32_t idx  = static_cast<int32_t>(seqId * 2);
    int32_t size = static_cast<int32_t>(mHistory.size());
    if (idx >= size)
    {
        beginInsertRows(QModelIndex(), mSequence, mSequence);
        entry.chatStatus = eMessageStatus::StatusError;
        mHistory.push_back(entry);
        endInsertRows();
    }
    else
    {
        idx = findEntry(seqId, idx);
        if (idx >= 0)
        {
            mHistory[idx].chatStatus = eMessageStatus::StatusReplied;
            if ((idx + 1) == size)
            {
                beginInsertRows(QModelIndex(), mSequence, mSequence);
                mHistory.push_back(entry);
            }
            else
            {
                beginInsertRows(QModelIndex(), idx + 1, idx + 1);
                mHistory.insert(mHistory.begin() + idx + 1, entry);
            }
            
            endInsertRows();
        }
        else
        {
            beginInsertRows(QModelIndex(), mSequence, mSequence);
            entry.chatStatus = eMessageStatus::StatusError;
            mHistory.push_back(entry);
            endInsertRows();
        }
    }
    
    return (entry.chatStatus == eMessageStatus::StatusReplied);
}

void AgentChatHistory::addFailure(const QString& text)
{
    beginInsertRows(QModelIndex(), mSequence, mSequence);
    sChatEntry entry{eChatSource::SourceEdgeAi, text, DateTime::getNow(), eMessageStatus::StatusError, mSequence};
    mHistory.push_back(entry);
    endInsertRows();
}

int AgentChatHistory::findEntry(uint32_t seqId, int32_t startAt)
{
    startAt = startAt >= static_cast<int32_t>(mHistory.size()) ? static_cast<int32_t>(mHistory.size()) - 1 : startAt;
    for (int32_t i = startAt; i > 0; -- i)
    {
        if (mHistory[i].chatId == seqId)
            return i;
    }
    
    return -1;
}
