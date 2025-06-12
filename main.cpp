//main.cpp server_1_1
#include "echoserver.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    EchoServer server(1234, true);
    MainWindow w;

    QObject::connect(&server, &EchoServer::jsonReceived, &w, &MainWindow::processJsonMessage);
    QObject::connect(&server, &EchoServer::nojsonReceived, &w, &MainWindow::processNoJsonMessage);

    w.show();
    return a.exec();
}
