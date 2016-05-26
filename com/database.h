#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>
#include <QtCore>
#include "global.h"

/// Database 模块
/// 这个模块一般是用来数据库的连接
/// 很多常用的操作都在这里了
/// 模块上级: MainWindow, 模块下级: 无

class Database : public QObject
{
	Q_OBJECT
	//数据库连接端口
	QSqlDatabase database;
public:
	//构造函数: 数据库连接
	explicit Database(DatabaseOption database, QObject *parent = 0);
	//析构函数: 数据库断开连接
	~Database();
	//打开数据库
	bool open();
	//关闭数据库
	bool close();
	//数据库是否在打开
	bool isRunning();
	//获得数据库里问题的列表
	QList<QuestionBrief> getQuestionList();
	//获得数据库里问题的内容
	Question getQuestionContent(int questionId);
	//获得数据库里用户的信息
	User getUserInformation(QString username, QString token);
	//使用用户名字获得用户信息
	User getUserInformation(QString username);
	//使用题目信息取得判题需要的输入输出数据
	QList<JudgeStream> getQuestionJudgeData(int questionId);
	//获得答案结果字符串
	QString getJudgeStatusString(Status status);

	//写入判题信息
	void setJudgeResult(JudgeResult result, QString source);
	//写入题目通过次数
	void setQuestionPass(int questionId, bool isPassed);
signals:
	//信号: 请求输出到用户信息列表中
	void stdOut(QString msg);
	//信号: 数据库已连接
	void connected();
	//信号: 数据库已经断开连接
	void disconnected();
public slots:
};

#endif // DATABASE_H
