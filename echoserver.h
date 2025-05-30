//echoserver.h
#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

#include <QHostAddress>
#include <QDebug>

class EchoServer : public QObject
{
    Q_OBJECT

public:
    explicit EchoServer(quint16 port, bool debug = false, QObject *parent = nullptr);
    ~EchoServer();

private slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void serverClosed();

private:
    QWebSocketServer *m_pWebSocketServer;
    QList <QWebSocket*> m_clients;
    bool m_debug;
};

#endif // ECHOSERVER_H
