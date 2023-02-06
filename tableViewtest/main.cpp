#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "tablemodel.h"
#include <QQmlContext>
#include "DBChandler.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    //qmlregistertypes

    qmlRegisterType<tablemodel>("TableModel",0, 1, "TableModel");

    //class definitions

    DBCHandler interface;

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    QQmlContext * Context1 = new QQmlContext (engine.rootContext());

    Context1->setContextProperty("interfaceObj", &interface);


    return app.exec();
}
