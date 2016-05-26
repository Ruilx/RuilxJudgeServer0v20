#include "networkhandle.h"

NetworkHandle::NetworkHandle(QTcpSocket *socket, QObject *parent) : QObject(parent)
{
	this->socket = socket;
}

void NetworkHandle::disconnected()
{
	qInfo() << "ClientId:" << QThread::currentThreadId() << "(" << QThread::currentThread() << ")" << "Disconnected.";
	emit this->handleDisconnected((qint64)QThread::currentThread());
	emit this->finished(QThread::currentThread());
}

void NetworkHandle::read()
{
	if(!this->timerId){
		this->timerId = this->startTimer(10);
	}
}

void NetworkHandle::start()
{
	this->bytesAvailable = 0;
	this->timerId = this->startTimer(10);
	connect(this->socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(this->socket, SIGNAL(readyRead()), this, SLOT(read()));
	emit this->handleConnected((qint64)QThread::currentThread());
	qInfo() << "ClientId:" << QThread::currentThreadId() << "(" << QThread::currentThread() << ")" << "Connected.";
}

void NetworkHandle::msgSend(QJsonDocument doc)
{
	if(doc.isNull() || doc.isEmpty()){
		qDebug() << "Will send an empty or null message document. Abandoned.";
		return;
	}
	this->socket->write(doc.toBinaryData());
}

void NetworkHandle::timerEvent(QTimerEvent *)
{
	qint64 bytes = this->bytesAvailable;
	this->bytesAvailable = this->socket->bytesAvailable();
	if(this->bytesAvailable != bytes){
		return;
	}
	killTimer(this->timerId);
	this->timerId = 0;
	if(this->bytesAvailable == 0){
		return;
	}
	this->bytesAvailable = 0;

	QByteArray data = this->socket->readAll();
	QJsonDocument doc = QJsonDocument::fromBinaryData(data);
	if(doc.isNull() || doc.isEmpty()){
		qDebug() << "Received an empty or null message document. Abandoned.";
		return;
	}
	qDebug() << "Receive: tid:" << (qint64)QThread::currentThread() << "doc:" << doc;
	emit this->msgReceived((qint64)QThread::currentThread(), doc);
}
