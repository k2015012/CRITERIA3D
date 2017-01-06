#include "mainwindow.h"
#include <QApplication>
#include <QtNetwork/QNetworkProxy>
#include <QDebug>
#include "raster.h"


bool setProxy(QString hostName, int port)
{
    QNetworkProxy myProxy;

    myProxy.setType(QNetworkProxy::HttpProxy);
    myProxy.setHostName(hostName);
    myProxy.setPort(port);

    qDebug("Init Proxy...");
    try {
       QNetworkProxy::setApplicationProxy(myProxy);
    }
    catch (...) {
       qDebug("Error in proxy configuration!");
       return false;
    }

    return true;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //setProxy("172.16.1.21", 8080);

    initializeDTM();

    MainWindow w;
    w.show();

    return a.exec();
}
