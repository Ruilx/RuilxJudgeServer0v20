#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QtCore>

class Config : public QObject
{
	Q_OBJECT
	QSettings *ini;
public:
	explicit Config(QString filename, QObject *parent = 0);
	//获取文件中规定的网络端口
	int getNetworkPort();
	//获取文件中规定评测机所接入的网络端口
	int getNetworkPortJudge();
	//获取文件中规定的评测机接入的主机IP地址
	QString getNetworkHostJudge();
	//获取文件中规定的数据库服务器地址
	QString getDatabaseHostName();
	//获取文件中规定的数据库数据库名字
	QString getDatabaseDatabaseName();
	//获取文件中数据库登录名
	QString getDatabaseUserName();
	//获取文件中数据库登录密码
	QString getDatabasePassword();
	//获取文件中数据库端口
	int getDatabasePort();
	//获取C文件编译器地址
	QString getCompileC();
	//获取C++文件编译器地址
	QString getCompileCpp();
	//获取Java文件编译器地址
	QString getCompileJava();
	//获取C文件编译的命令行
	QString getCompileCCmd();
	//获取C++文件编译的命令行
	QString getCompileCppCmd();
	//获取Java文件编译的命令行
	QString getCompileJavaCmd();
	//获取程序是否使用docker环境进行代码运行
	int getGlobalUseDocker();
	//获取RunCode评测子系统位置(如果UseDocker==true, 则填入RunPath在容器中的位置)
	QString getGlobalRunCodePath();
	//获取dockerSocket的文件位置(如果UseDocker==false, 则无效)
	QString getGlobalDockerSocketPath();
	//获取Docker打粉文件位置(如果UseDocker==false, 则无效)
	QString getGlobalDocker();

signals:

public slots:
};

#endif // CONFIG_H
