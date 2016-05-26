#include "network.h"

Network::Network(int port, QObject *parent) : QObject(parent)
{
	this->server = new QTcpServer(this);
	this->port = port;
}

Network::~Network()
{
	foreach(auto p, this->clientMap){
		p.thread->quit();
		p.thread->wait();
	}
}

bool Network::isRunning()
{
	return this->server->isListening();
}

void Network::send(qint64 tid, QJsonDocument doc)
{
	Client client = clientMap.value(tid, Client());
	if(client.thread != nullptr){
		client.handle->msgSend(doc);
	}else{
		qDebug() << "Invaild tid:" << tid;
	}
}

bool Network::startServer()
{
	if(this->isRunning()){
		qDebug() << "Server is already running while start server again.";
		return false;
	}
	if(!this->server->listen(QHostAddress::Any, this->port)){
		qCritical() << "Cannot bind port on" << this->port << "Aborted.";
		emit this->stdOut(tr("无法绑定端口%1, 请检查此端口是否被占用, 或在配置文件中设置另一个端口.").arg(this->port));
		//qApp->exit(1);
		return false;
	}
	connect(this->server, SIGNAL(newConnection()), this, SLOT(newConnection()));
	qDebug() << "Server has started at here bind port:" << this->port;
	emit this->serverStarted();
	return true;
}

bool Network::stopServer()
{
	if(!this->isRunning()){
		qDebug() << "Server is already stopped while stop server again.";
		return false;
	}
	this->server->close();
	emit this->stdOut(tr("服务器成功关闭."));
	emit this->serverStopped();
	return true;
}

void Network::newConnection()
{
	QThread *handleThread = new QThread(this);
	Client client;
	client.thread = handleThread;
	QTcpSocket *socket = server->nextPendingConnection();
	connect(handleThread, SIGNAL(finished()), socket, SLOT(deleteLater()));
	NetworkHandle *handle = new NetworkHandle(socket);
	connect(handleThread, SIGNAL(started()), handle, SLOT(start()));
	connect(handleThread, SIGNAL(finished()), handle, SLOT(deleteLater()));
	connect(handle, SIGNAL(finished(QThread*)), this, SLOT(join(QThread*)));
	connect(handle, SIGNAL(handleConnected(qint64)), this, SLOT(handleConnected(qint64)));
	connect(handle, SIGNAL(handleDisconnected(qint64)), this, SLOT(handleDisconnected(qint64)));

	handle->moveToThread(handleThread);
	handleThread->start();
	client.handle = handle;
	this->clientMap.insert((qint64)handle->thread(), client);
	connect(handle, SIGNAL(msgReceived(qint64,QJsonDocument)), this, SLOT(receive(qint64,QJsonDocument)));

	qDebug() << clientMap.keys();
}

void Network::join(QThread *thread)
{
	this->clientMap.take((qint64)thread);
	qDebug() << "Thread:" << thread << "(" << thread->currentThreadId() << ")" << "deleted.";

	thread->quit();
	thread->wait();
	delete thread;
}

void Network::receive(qint64 tid, QJsonDocument doc)
{
	emit this->received(tid, doc);
}

void Network::handleConnected(qint64 threadId)
{
	emit this->stdOut(tr("用户/评测机: %1 已连接.").arg(threadId));
}

void Network::handleDisconnected(qint64 threadId)
{
	emit this->stdOut(tr("用户/评测机: %1 已离开.").arg(threadId));
}

