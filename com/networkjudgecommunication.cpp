#include "networkjudgecommunication.h"

void NetworkJudgeCommunication::handleRunCodeStatus(qint64 tid, QJsonObject &obj)
{
	if(obj.value("cmdId").toInt(0) != 2){
		return;
	}
	RunCodeJudgeResult result;
	result.name = obj.value("name").toString();
	result.status = obj.value("status").toInt(0);
	result.statusStr = obj.value("statusStr").toString();
	result.exitCode = obj.value("exitCode").toInt(-1);
	result.timeUsed = obj.value("timeUsed").toInt(-1);
	result.memoryUsed = obj.value("memoryUsed").toInt(-1);
	result.outputString = obj.value("outputString").toString();
	if(result.name.isEmpty() ||
			result.status == 0 ||
			//result.statusStr.isEmpty() ||
			result.timeUsed == -1 ||
			result.memoryUsed == -1 ||
			result.outputString.isEmpty()){
		qDebug() << "result is invaild.";
		return;
	}
	emit this->stdOut(tr("RunCode: %1 send it result to host: status: %2, statusStr: %3").arg(tid).arg(result.status).arg(result.statusStr));
	emit this->requestRunCodeStatus(tid, result);
}

void NetworkJudgeCommunication::handleRunCodeName(qint64 tid, QJsonObject &obj)
{
	if(obj.value("cmdId").toInt(0) != 1){
		return;
	}
	QString name = obj.value("name").toString();
	emit this->stdOut(tr("RunCode: %1 send its name to host: %2").arg(tid).arg(name));
	emit this->requestRunCodeName(tid, name);
}

NetworkJudgeCommunication::NetworkJudgeCommunication(Network *network, QObject *parent) : QObject(parent)
{
	this->network = network;
	connect(network, SIGNAL(received(qint64,QJsonDocument)), this, SLOT(msgReceive(qint64,QJsonDocument)));
}

void NetworkJudgeCommunication::msgReceive(qint64 tid, QJsonDocument doc)
{
	emit this->stdOut(tr("收到评测机: %1 的命令").arg(tid));
	if(!doc.isObject()){
		qWarning() << "This network ID:" << tid << "receive JSON:" << doc << "is not an object, aborted.";
		return;
	}
	QJsonObject obj = doc.object();
	switch(obj.value("cmdId").toInt(0)){
		case 1: this->handleRunCodeName(tid, obj); break;
		case 2: this->handleRunCodeStatus(tid, obj); break;
		default: qDebug() << "Invaild message received, tid: " << tid << "doc:" << doc;
			return;
	}
}

