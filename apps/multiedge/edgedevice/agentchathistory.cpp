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

AgentChatHistory::AgentChatHistory(QObject *parent)
    : QAbstractTableModel   (parent)
{

}
