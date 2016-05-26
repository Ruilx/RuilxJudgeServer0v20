#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>
#include <QJsonDocument>
#include <QThread>
#include "networkhandle.h"

/// Network 模块
/// 这个模块一般是用来控制和创建socket的
/// 控制socket的创建和删除
/// 模块上级: NetworkCommunication, 模块下级: NetworkHandle

typedef struct{
	QThread *thread;
	NetworkHandle *handle;
}Client;

class Network : public QObject
{
	Q_OBJECT
	//监听端口
	int port;
	//Server端
	QTcpServer *server;
	//Thread的UID对应的客户端handle的映射
	QMap<qint64, Client> clientMap;
public:
	//构造函数: 网络服务端接口
	explicit Network(int port, QObject *parent = 0);
	//析构函数: 网络服务端接口
	~Network();
	//IS函数, 是否在运行
	bool isRunning();
signals:
	//信号: 想要在信息栏输出信息
	void stdOut(QString msg);
	//信号: 标示服务器已经启动
	void serverStarted();
	//信号: 标示服务器已经停止
	void serverStopped();
	//信号: 收到信息
	void received(qint64 tid, QJsonDocument doc);
public slots:
	//槽: 发送信息:
	void send(qint64 tid, QJsonDocument doc);
	//槽: 请求开启服务器
	bool startServer();
	//槽: 请求停止服务器
	bool stopServer();
private slots:
	//私有槽: 有新客户端连接
	void newConnection();
	//私有槽: 清理已经关闭连接的线程
	void join(QThread *thread);
	//私有槽: 接收到发来的信息
	void receive(qint64 tid, QJsonDocument doc);
	//私有槽: handle线程的连接处理
	void handleConnected(qint64 threadId);
	//私有槽: handle线程的断开处理
	void handleDisconnected(qint64 threadId);
};

#endif // NETWORK_H
