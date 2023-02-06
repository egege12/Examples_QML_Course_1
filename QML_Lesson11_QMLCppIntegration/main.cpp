#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


#include "someclass.h"

/* İki yöntem var
    -   QmlRegisterType  -> burası sadece obje kullanılırken QML tarafından çağırılır.
    -   QmlContext  -> bu cpp ile başlar ve burda kullanılabilir.

 Temel fark nedir ?

    - register ile olursa qmlde tanımlar kullanırsın. Obje çağırılana kadar başlatmaz. context ile cppde çağrıldığı an başlatılır.
*/
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

//QQmlContext ile  olursa
//SomeClass testClass;

//qml register ile

    qmlRegisterType<SomeClass> ("Monty", 1, 0 , "SomeClass");


    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

//QQmlContext ile
//QQmlContext * rootContext = engine.rootContext();
//rootContext->setContextProperty( "classA",&testClass);

    return app.exec();
}
