#ifndef NETWORKHANDLE_H
#define NETWORKHANDLE_H

#include <QObject>
#include <QtCore>
#include <QJsonDocument>
#include <QtNetwork>

/// NetworkHandle 模块
/// 这个模块一般是用来接收和发送信息的端口
/// 里面包含一个QTcpSocket接口, 使用这个接口来接收和发送信息
/// 使用模块的时候必须新建一条线程来控制, 线程开始的信号接口到start()槽
/// 线程终止的信号连接到deleteLater()槽
/// 模块的finished()函数连接network的线程垃圾处理函数
/// 注意, 本模块的parents必须缺省
/// 模块上级: Network, 模块下级: QTcpSocket

class NetworkHandle : public QObject
{
	Q_OBJECT
	//TCP socket 实体
	QTcpSocket *socket;
	//读取缓存时间间隔
	int timerId;
	//缓存中数据数量
	qint64 bytesAvailable;
public:
	//构造函数, 传入socket, 控制其发送和接收
	explicit NetworkHandle(QTcpSocket *socket, QObject *parent = 0);
signals:
	//信号: 已经收到了信息
	void msgReceived(qint64 threadId, QJsonDocument doc);
	//信号: 连接已断开, 此线程进行完成
	void finished(QThread *);
	//信号: 网络信道已创建
	void handleConnected(qint64 threadId);
	//信号: 网络信道已关闭
	void handleDisconnected(qint64 threadId);
private slots:
	//私有槽: 断开连接
	void disconnected();
	//私有槽: 读取内容
	void read();
public slots:
	//槽: 开始干活
	void start();
	//槽: 发送内容
	void msgSend(QJsonDocument doc);
protected:
	//事件: 时间事件
	void timerEvent(QTimerEvent *);
};

#endif // NETWORKHANDLE_H
