#include "networkcommunication.h"

void NetworkCommunication::handleGetQuestionList(qint64 tid, QJsonObject &obj)
{
	if(obj.value("cmdId").toInt(0) != 1){
		return;
	}
	emit this->stdOut(QString("User %1 request to get question list.").arg(tid));
	emit this->requestQuestionList(tid);
}

void NetworkCommunication::handleGetQuestionContent(qint64 tid, QJsonObject &obj)
{
	if(obj.value("cmdId").toInt(0) != 2){
		return;
	}
	int questionId = obj.value("questionId").toInt(0);
	if(questionId == 0){
		return;
	}
	emit this->stdOut(QString("User %1 request to get the content of question %2.").arg(tid).arg(questionId));
	emit this->requestQuestionContent(tid, questionId);
}

void NetworkCommunication::handleJudge(qint64 tid, QJsonObject &obj)
{
	if(obj.value("cmdId").toInt(0) != 3){
		return;
	}
	Judge judge;
	judge.username = obj.value("username").toString();
	judge.token = obj.value("token").toString();
	judge.programSource = obj.value("programSource").toString();
	judge.language = (Language)obj.value("language").toInt();
	judge.questionId = obj.value("questionId").toInt();
	judge.judgeId = obj.value("judgeId").toInt();
	if(judge.username.isEmpty() ||
			judge.token.isEmpty() ||
			judge.programSource.isEmpty() ||
			judge.language == None ||
			judge.questionId == 0 ||
			judge.judgeId == 0){
		return;
	}
	emit this->stdOut(QString("User %1 request to judge the question %2, judgeId is %3.").arg(tid).arg(judge.questionId).arg(judge.judgeId));
	emit this->requestJudge(tid, judge);
}

void NetworkCommunication::handleUserInfomation(qint64 tid, QJsonObject &obj)
{
	if(obj.value("cmdId").toInt(0) != 4){
		return;
	}
	QString username = obj.value("username").toString();
	QString token = obj.value("token").toString();
	if(username.isEmpty() || token.isEmpty()){
		return;
	}
	emit this->stdOut(QString("User %1 request its information. username: %2").arg(tid).arg(username));
	emit this->requestUserInformation(tid, username, token);
}

NetworkCommunication::NetworkCommunication(Network *network, QObject *parent) : QObject(parent), network(network)
{
	this->network = network;
	connect(network, SIGNAL(received(qint64,QJsonDocument)), this, SLOT(msgReceive(qint64,QJsonDocument)));
}

void NetworkCommunication::responseQuestionList(qint64 tid, QList<QuestionBrief> questionList)
{
	QJsonObject obj;
	obj["cmdId"] = GetQuestionList;
	int i = 0;
	foreach(auto p, questionList){
		QJsonObject pQuestion;
		pQuestion["questionId"] = p.questionId;
		pQuestion["title"] = p.title;
		pQuestion["passNum"] = p.passNum;
		pQuestion["submitNum"] = p.submitNum;
		obj.insert(QString::number(i), pQuestion);
		i++;
	}

	QJsonDocument doc(obj);
	this->network->send(tid, doc);
}

void NetworkCommunication::responseQuestionContent(qint64 tid, Question question)
{
	QJsonObject obj;
	obj["cmdId"] = GetQuestionContent;
	obj["questionId"] = question.questionId;
	obj["title"] = question.title;
	obj["description"] = question.description;
	obj["input"] = question.input;
	obj["output"] = question.output;
	obj["inputSample"] = question.inputSample;
	obj["outputSample"] = question.outputSample;
	obj["hint"] = question.hint;
	obj["source"] = question.source;
	obj["timeLimit"] = question.timeLimit;
	obj["memoryLimit"] = question.memoryLimit;
	obj["passNum"] = question.passNum;
	obj["submitNum"] = question.submitNum;

	QJsonDocument doc(obj);
	this->network->send(tid, doc);
}

void NetworkCommunication::responseJudge(qint64 tid, JudgeResult result)
{
	QJsonObject obj;
	obj["cmdId"] = GetJudged;
	obj["username"] = result.username;
	obj["judgeId"] = result.judgeId;
	obj["questionId"] = result.questionId;
	obj["status"] = result.status;
	obj["judgeTime"] = result.judgeTime;
	obj["language"] = result.language;
	obj["timeUsed"] = result.timeUsed;
	obj["memoryUsed"] = result.memoryUsed;
	obj["message"] = result.message;

	QJsonDocument doc(obj);
	this->network->send(tid, doc);
}

void NetworkCommunication::responseUserInformation(qint64 tid, User user)
{
	QJsonObject obj;
	obj["cmdId"] = GetUserInformation;
	obj["userId"] = user.userId;
	obj["username"] = user.username;
	obj["userGroup"] = user.userGroup;
	obj["email"] = user.email;
	obj["token"] = user.token;

	QJsonDocument doc(obj);
	this->network->send(tid, doc);
}

void NetworkCommunication::responseMessage(qint64 tid, QString msg)
{
	QJsonObject obj;
	obj["cmdId"] = Message;
	obj["message"] = msg;

	QJsonDocument doc(obj);
	this->network->send(tid, doc);
}

void NetworkCommunication::msgReceive(qint64 tid, QJsonDocument doc)
{
	emit this->stdOut(tr("收到用户: %1 的命令").arg(tid));
	if(!doc.isObject()){
		qWarning() << "This network ID:" << tid << "receive JSON:" << doc << "is not an object, aborted.";
		return;
	}
	QJsonObject obj = doc.object();
	switch(obj.value("cmdId").toInt(0)){
		case GetQuestionList: this->handleGetQuestionList(tid, obj); break;
		case GetQuestionContent: this->handleGetQuestionContent(tid, obj); break;
		case GetJudged: this->handleJudge(tid, obj); break;
		case GetUserInformation: this->handleUserInfomation(tid, obj); break;
		default: qDebug() << "Invaild message received, tid: " << tid << "doc:" << doc;
			return;
	}
}
