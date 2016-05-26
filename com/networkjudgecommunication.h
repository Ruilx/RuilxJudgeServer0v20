#ifndef NETWORKJUDGECOMMUNICATION_H
#define NETWORKJUDGECOMMUNICATION_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork>
#include "network.h"
#include "global.h"

/// NetworkJudgeCommunication 模块
/// 这个模块是用来接收RunCode判定子进程发来的评测消息
/// 模块上级MainWindow, 模块下级: Network

class NetworkJudgeCommunication : public QObject
{
	Q_OBJECT
	Network *network;

	//用来处理RunCode发来的评测结果
	void handleRunCodeStatus(qint64 tid, QJsonObject &obj);
	//用来处理RunCode发来的名字
	void handleRunCodeName(qint64 tid, QJsonObject &obj);
public:
	//构造函数: 网络数据处理
	explicit NetworkJudgeCommunication(Network *network, QObject *parent = 0);

signals:
	//信号, 请求输出到屏幕
	void stdOut(QString msg);
	//信号, RunCode发来了评测结果
	void requestRunCodeStatus(qint64 tid, RunCodeJudgeResult result);
	//信号, RunCode发来了评测机名字
	void requestRunCodeName(qint64 tid, QString name);
public slots:
private slots:
	void msgReceive(qint64 tid, QJsonDocument doc);
};

#endif // NETWORKJUDGECOMMUNICATION_H
