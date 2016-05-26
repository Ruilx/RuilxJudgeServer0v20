#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtCore>
#include "config.h"
#include "com/global.h"
#include "com/database.h"
#include "com/compile.h"
#include "com/network.h"
#include "com/networkcommunication.h"
#include "com/networkjudgecommunication.h"
#include "com/runcode.h"
#include "com/assessment.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
	QTextEdit *outputEdit;

	typedef struct{
		int port;
		int portJudge;
		QString hostJudge;
	}NetworkSettings;

	typedef struct{
		QString c;
		QString cpp;
		QString java;
		QString cCmd;
		QString cppCmd;
		QString javaCmd;
	}CompilerPath;

	typedef struct{
		int useDocker;
		QString runCodePath;
		QString dockerSocketPath;
		QString docker;
	}UseDocker;

	int timerId;
//	QElapsedTimer *duringTimer;

	Database *db;
//	Compile *compile;
	Network *net;
	NetworkCommunication *netC;
	Network *netRunCode;
	NetworkJudgeCommunication *netRunCodeC;
	Config *config;

	NetworkSettings ns;
	DatabaseOption  ds;
	CompilerPath    cp;
	UseDocker       ud;

	QMap<qint64, Judge> judgeMap;
	QMap<qint64, int> judgeIndex;

protected:
	void timerEvent(QTimerEvent *);
	void closeEvent(QCloseEvent *);
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
private slots:
	void joinThread(QThread* thread);
	void showStdOut(QString msg);

	void receiveQuestionList(qint64 tid);
	void receiveQuestionContent(qint64 tid, int questionId);
	void receiveJudge(qint64 tid, Judge judge);
	void receiveUserInformation(qint64 tid, QString user, QString token);

	void receiveRunCodeName(qint64 tid, QString name);
	void receiveRunCodeStatus(qint64 tidRunCode, RunCodeJudgeResult result);
private:
	void startSystem();
signals:
	void sendQuestionList(qint64 tid, QList<QuestionBrief> questionList);
	void sendQuestionContent(qint64 tid, Question question);
	void sendJudge(qint64 tid, JudgeResult result);
	void sendUserInformation(qint64 tid, User user);
	void sendMessage(qint64 tid, QString message);
};

#endif // MAINWINDOW_H
