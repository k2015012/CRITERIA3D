#include "mainWindow.h"
#include <QApplication>
#include <QtNetwork/QNetworkProxy>
#include <QDebug>
#include <QMessageBox>

#include "pragaProject.h"


PragaProject myProject;


bool setProxy(QString hostName, unsigned short port)
{
    QNetworkProxy myProxy;

    myProxy.setType(QNetworkProxy::HttpProxy);
    myProxy.setHostName(hostName);
    myProxy.setPort(port);

    try {
       QNetworkProxy::setApplicationProxy(myProxy);
    }
    catch (...) {
        QMessageBox::information(NULL, "Error in proxy configuration!", "");
        return false;
    }

    return true;
}


int main(int argc, char *argv[])
{
    QApplication myApp(argc, argv);

    QString currentPath = myApp.applicationDirPath() + "/";
    if (! myProject.readGenericSettings(currentPath))
        return -1;

    if (! myProject.readPragaParameters())
        return -1;

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    QApplication::setOverrideCursor(Qt::ArrowCursor);
    MainWindow w;
    w.show();

    return myApp.exec();
}





