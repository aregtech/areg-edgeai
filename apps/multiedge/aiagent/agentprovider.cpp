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
 *  \file        multiedge/aiagent/agentprovider.cpp
 *  \ingroup     Areg Edge AI, AI Multi Edge Device Agent
 *  \author      Artak Avetyan
 *  \brief       Edge AI Agent service provider.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "multiedge/aiagent/agentprovider.hpp"

AgentProvider::AgentProvider(const NERegistry::ComponentEntry& entry, ComponentThread& owner)
    : Component     (entry, owner)
    , MultiEdgeStub (static_cast<Component &>(self()))
{
}

void AgentProvider::requestProcessText(unsigned int sessionId, unsigned int agentId, const String& textProcess)
{
}

void AgentProvider::requestProcessVideo(unsigned int sessionId, bool agentId, const String& cmdText, const SharedBuffer& dataVideo)
{
}

inline AgentProvider& AgentProvider::self(void)
{
    return *this;
}