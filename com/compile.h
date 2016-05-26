#ifndef COMPILE_H
#define COMPILE_H

#include <QObject>
#include <QtCore>
#include <QProcess>
//-fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret
//无色                    无Warning忽略指导参数          无展示文件内容

/// Compile 模块
/// 这个模块一般是用来编译程序的
/// 采用的是等待编译结果的方法
/// 如果要使用多线程的话必须创建线程来控制多进程编译
/// 模块上级: MainWindow, 模块下级: 无

class Compile : public QObject
{
	Q_OBJECT
	//本地的编译器位置
	QString compilePath;
	//本地编译的时候的命令行
	QString compileCmd;
	//编译器的进程控制接口
	QProcess *compiler;
	//输入的文件地址
	QString sourceFile;
	//输出的文件地址
	QString targetFile;
	//标准错误的存储
	QString stdErr;
	//编译器退出码
	int exitCode;
	//编译器是否被人为退出
	bool initiativeStopped;
	//强制结束进程
	bool killProcess();
	//编译错误数量
	int errorNum;
	//编译警告数量
	int warningNum;
	//是否正常编译
	bool compileComplete;
public:
	//构造函数, 输入源文件地址, 准备编译
	explicit Compile(QString sourceFile, QObject *parent = 0);
	//设置编译器位置
	void setCompilePath(const QString &path);
	//设置编译时的命令
	void setCompileCmd(const QString &cmd);
	//获取标准错误的内容
	QString getStdErr();
	//获取退出码
	int getExitCode();
	//IS函数 是否编译完成
	bool isCompileComplete();
	//获取编译后的文件位置
	QString getTargetFilePath();

//	void setGccPath(const QString &path);
//	void setGppPath(const QString &path);
//	void setJavaPath(const QString &path);
//	void setGccCmd(const QString &cmd);
//	void setGppCmd(const QString &cmd);
//	void setJavaCmd(const QString &cmd);

signals:
	//请求在输出窗口输出
	void stdOut(QString msg);
private slots:
	//处理编译程序退出的信息
	void finished(int exitCode ,QProcess::ExitStatus status);
	//读取标准错误输出
//	void readStdErr();

public slots:
	//槽: 开始编译
	void startCompile();
	//槽: 停止编译
	void stopCompile();
};

#endif // COMPILE_H
