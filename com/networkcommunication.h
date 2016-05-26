#ifndef NETWORKCOMMUNICATION_H
#define NETWORKCOMMUNICATION_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork>
#include "network.h"
#include "global.h"

/// NetworkCommunication 模块
/// 这个模块一般是用来把网络通信的消息做成动作, 进行请求
/// 模块上级: MainWindow, 模块下级: Network

class NetworkCommunication : public QObject
{
	Q_OBJECT
	Network *network;
	//用来处理想要获取题目列表的Json->结构体数据的处理函数
	void handleGetQuestionList(qint64 tid, QJsonObject &obj);
	//用来处理想要获取题目内容的Json->结构体数据的处理函数
	void handleGetQuestionContent(qint64 tid, QJsonObject &obj);
	//用来处理想要判题Json->结构体数据的处理函数
	void handleJudge(qint64 tid, QJsonObject &obj);
	//用来处理想要获取用户信息的Json->结构体数据的处理函数
	void handleUserInfomation(qint64 tid, QJsonObject &obj);

public:
	//构造函数: 网络数据处理
	explicit NetworkCommunication(Network *network, QObject *parent = 0);

signals:
	//信号: 请求输出到屏幕
	void stdOut(QString msg);
	//信号: tid用户请求获得问题的列表
	void requestQuestionList(qint64 tid);
	//信号: tid用户请求得到questionId的具体内容
	void requestQuestionContent(qint64 tid, int questionId);
	//信号: tid用户请求判题
	void requestJudge(qint64 tid, Judge judge);
	//信号: tid用户请求自己的用户信息
	void requestUserInformation(qint64 tid, QString user, QString token);
public slots:
	//槽: 回复tid用户的题目列表
	void responseQuestionList(qint64 tid, QList<QuestionBrief> questionList);
	//槽: 回复tid用户题目的具体信息
	void responseQuestionContent(qint64 tid, Question question);
	//槽: 回复tid用户题目的判定结果
	void responseJudge(qint64 tid, JudgeResult result);
	//槽: 回复tid用户的用户信息
	void responseUserInformation(qint64 tid, User user);

	//槽: 只回复一句话的回复(就是出错了, 然后拒绝服务)
	void responseMessage(qint64 tid, QString msg);
private slots:
	//私有槽: 收到网络信息
	void msgReceive(qint64 tid, QJsonDocument doc);

};

#endif // NETWORKCOMMUNICATION_H
