#include "multiedge/edgedevice/edgedevice.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "multiedge_" + QLocale(locale).name();
        if (translator.load(":/resource/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    a.setApplicationName("Edge Device");
    EdgeDevice w;
    w.show();
    return a.exec();
}
