#ifndef MULTIEDGE_RESOURCES_NEMULTIEDGESETTINGS_HPP
#define MULTIEDGE_RESOURCES_NEMULTIEDGESETTINGS_HPP
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
 *  \file        multiedge/resources/nemultiedgesettings.hpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <string_view>

namespace NEMultiEdgeSettings
{
    constexpr std::string_view AGENT_THREAD     { "EdgeAIThread" };     //!< The name of the AI agent consumer thread.
    constexpr std::string_view MODEL_CONSUMER   { "EdgeDevice" };       //!< The name of the model.
    constexpr std::string_view MODEL_PROVIDER   { "EdgeAIAgent" };      //!< The name of the model provider.
    constexpr std::string_view SERVICE_PROVIDER { "EdgeAIProvider" };   //!< The edge AI service provider name.
    constexpr std::string_view SERVICE_CONSUMER { "EdgeAIConsumer" };   //!< The edge AI service consumer name.
    constexpr std::string_view ROUTER_ADDRESS   { "127.0.0.1" };        //!< The IP-address of the router service.
    constexpr uint16_t         ROUTER_PORT      { 8181 };               //!< The port of the router service.
}

#endif // MULTIEDGE_RESOURCES_NEMULTIEDGESETTINGS_HPP