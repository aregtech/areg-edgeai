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
 *  \file        multiedge/edgedevice/main.cpp
 *  \ingroup     Areg Edge AI, Edge Device
 *  \author      Artak Avetyan
 *  \brief       Edge Device Dialog.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "multiedge/aiagent/aiagent.hpp"
#include "areg/appbase/Application.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "llama.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "multiedge_" + QLocale(locale).name();
        if (translator.load(":/resource/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }

    Application::initApplication(true, true, false);
    // Load backends once per process.    
    ggml_backend_load_all();

    a.setApplicationName("Edge AI Agent");
    AIAgent w;
    w.show();
    int result = a.exec();
    w.disconnectAgent();
    return result;
}
