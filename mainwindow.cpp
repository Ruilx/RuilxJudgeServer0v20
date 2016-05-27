#include "mainwindow.h"

void MainWindow::timerEvent(QTimerEvent *)
{
	this->killTimer(this->timerId);
	this->startSystem();
}

void MainWindow::closeEvent(QCloseEvent *)
{
	if(this->net != nullptr && this->net->isRunning()){
		this->net->stopServer();
	}
	if(this->db != nullptr && this->db->isRunning()){
		this->db->close();
	}
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	this->resize(800, 600);
	this->outputEdit = new QTextEdit(this);
	this->outputEdit->setReadOnly(true);
	this->outputEdit->setWordWrapMode(QTextOption::NoWrap);
//	this->duringTimer = new QElapsedTimer;

//	QGridLayout *lay = new QGridLayout;
//	this->setLayout(lay);
//	lay->addWidget(this->outputEdit);

	this->setCentralWidget(this->outputEdit);
//	this->duringTimer->start();

	if(!QFile::exists("config.ini")){
		qCritical() << "Config file not found.";
		showStdOut(tr("配置文件错误: config.ini未找到."));
		return;
	}

	this->config = new Config("config.ini", this);

	this->showStdOut(tr("系统将在3秒后进行启动..."));
	this->timerId = this->startTimer(3000);

	this->net = nullptr;
	this->db = nullptr;
	this->netC = nullptr;
	this->netRunCode = nullptr;
	this->netRunCodeC = nullptr;
}

MainWindow::~MainWindow()
{
}

void MainWindow::joinThread(QThread *thread)
{
	thread->quit();
	thread->wait();
	delete thread;
}

void MainWindow::showStdOut(QString msg)
{
	QStringList msgList = msg.split("\n");
	foreach(QString mmsg, msgList){
		this->outputEdit->append("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "]" + mmsg);
	}
}

void MainWindow::receiveQuestionList(qint64 tid)
{
	QList<QuestionBrief> qb = this->db->getQuestionList();
	if(qb.isEmpty()){
		emit this->sendMessage(tid, tr("题库中没有题目."));
	}else{
		emit this->sendQuestionList(tid, qb);
	}
}

void MainWindow::receiveQuestionContent(qint64 tid, int questionId)
{
	Question q = this->db->getQuestionContent(questionId);
	if(q.questionId == -1){
		emit this->sendMessage(tid, tr("题库中没有题号为%1的题目信息.").arg(questionId));
	}else{
		emit this->sendQuestionContent(tid, q);
	}
}

void MainWindow::receiveJudge(qint64 tid, Judge judge)
{
	enum JudgeError{
		TempDirCannotBuild = 0,
		FileCannotOpen,
		LanguageNotSupport,
		CompilerNotSupport,
		CompileFailed,
		InputFileCannotOpen,
		OutputFileCannotOpen,
	};

	this->showStdOut(tr("用户%1(%2)上传源码进行评测, 评测题目:%3, 评测ID:%4, 使用语言:%5").arg(tid).arg(judge.username).arg(judge.questionId).arg(judge.judgeId).arg(judge.language));
	QString source = judge.programSource;
	JudgeResult result;
	result.judgeTime = (int)QDateTime::currentDateTime().toTime_t();
	result.language = judge.language;
	result.username = judge.username;
	result.questionId = judge.questionId;
	result.judgeId = judge.judgeId;
	try{
		QDir dir = QDir::current();
		if(!dir.cd("temp")){
			if(!dir.mkdir("temp")){
				throw TempDirCannotBuild;
			}
			if(!dir.cd("temp")){
				throw TempDirCannotBuild;
			}
		}
		QString filename = QString::number(tid).append(".") + QString::number(judge.questionId);
		QString filenameBase = filename;
		switch(judge.language){
			case C:
				filename.append(".c"); break;
			case Cpp:
				filename.append(".cpp"); break;
			case Java:
				filename.append(".java"); break;
			case Python:
				filename.append(".py"); break;
			case Php:
				filename.append(".php"); break;
			case Lua:
				filename.append(".lua"); break;
			default:
				throw LanguageNotSupport;
		}

		QString filepath = dir.absoluteFilePath(filename);
		QFile file(filepath);
		if(!file.open(QIODevice::WriteOnly)){
			throw FileCannotOpen;
		}
		QTextStream sfile(&file);
		sfile.setCodec("utf-8");
		sfile << source;
		file.close();
		Compile compile(filepath, this);
		connect(&compile, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));
		switch(judge.language){
			case C:
				compile.setCompilePath(cp.c);
				compile.setCompileCmd(cp.cCmd);
				break;
			case Cpp:
				compile.setCompilePath(cp.cpp);
				compile.setCompileCmd(cp.cppCmd);
				break;
			case Java:
				compile.setCompilePath(cp.java);
				compile.setCompileCmd(cp.javaCmd);
				break;
			default:
				throw CompilerNotSupport;
		}

		compile.startCompile();
		if(!compile.isCompileComplete()){
			throw CompileFailed;
		}
		if(compile.getExitCode() != 0){
			//Compiled has error, cannot continue.
			result.status = CompileError;
			result.timeUsed = 0;
			result.memoryUsed = 0;
			result.message = compile.getStdErr();
			result.language = judge.language;
			qInfo() << "User: " << tid << "has compile error.";
			this->showStdOut(tr("用户%1(%2)的代码编译出错: %3").arg(tid).arg(judge.username).arg(compile.getStdErr()));
			this->db->setJudgeResult(result, judge.programSource);
			emit this->sendJudge(tid, result);
			showStdOut(tr("评测机%1为用户%2(%3)评测的结果是: 题目%4, 结果%5").arg("未定义").arg(tid).arg(result.username).arg(result.questionId).arg(this->db->getJudgeStatusString(result.status)));
			return;
		}else{
			//Compiled has warning or succeed.
			if(!compile.getStdErr().isEmpty()){
				qInfo() << "User: " << tid << "has compile warning.";
				result.message = compile.getStdErr();
				emit this->sendMessage(tid, tr("您的代码编译后有警告, 将在之后的回执中为您显示."));
				this->showStdOut(tr("用户%1(%2)上传的代码编译有警告: %3").arg(tid).arg(judge.username).arg(compile.getStdErr()));
			}
		}

		this->judgeMessage.insert(tid, compile.getStdErr());

		QList<JudgeStream> judgeStreamList = this->db->getQuestionJudgeData(judge.questionId);
		QList<JudgeStream> judgeStreamFilePathList;
		int i = 0;
		foreach(auto judgeStream, judgeStreamList){
			JudgeStream stream;
			QString inputStreamFilename = judgeStream.input.isEmpty()?QString():filenameBase.append(".").append(QString::number(i)).append(".sin");
			QString outputStreamFilename = filenameBase.append(".").append(QString::number(i)).append(".sout");
			stream.questionId = judge.questionId;
			stream.input = inputStreamFilename.isEmpty()?QString():dir.absoluteFilePath(inputStreamFilename);
			stream.output = dir.absoluteFilePath(outputStreamFilename);
			judgeStreamFilePathList << stream;

			if(!judgeStream.input.isEmpty()){
				QFile inputFile(stream.input);
				if(!inputFile.open(QIODevice::WriteOnly)){
					throw InputFileCannotOpen;
				}
				QTextStream sin(&inputFile);
				sin << judgeStream.input;
				inputFile.close();
			}

			QFile outputFile(stream.output);
			if(!outputFile.open(QIODevice::WriteOnly)){
				throw OutputFileCannotOpen;
			}
			QTextStream sout(&outputFile);
			sout << judgeStream.output;
			outputFile.close();
			this->judgeIndex.insert(tid, i);
		}

		Question ques = this->db->getQuestionContent(judge.questionId);
		if(ques.questionId <= 0){
			return;
		}

//		for(int i=0; i<judgeStreamFilePathList.length(); i++){
			RunCodeSettings settings;
			settings.dockerPath = ud.docker;
			settings.execfile = compile.getTargetFilePath();
			settings.host = ns.hostJudge;
			settings.image = ques.image;
			settings.inputfile = judgeStreamFilePathList.at(/*i*/0).input;
			settings.memoryLimit = ques.memoryLimit;
			settings.port = ns.portJudge;
			settings.runCodeFilePath = ud.runCodePath;
			settings.tid = tid;
			settings.timeLimit = ques.timeLimit;
			settings.useDocker = ud.useDocker == 1?true:false;
			settings.outputLimit = judgeStreamList.at(/*i*/0).output.length() * ProgramOutputLimitTimes;

			QThread *runCodeThread = new QThread(this);
			RunCode *run = new RunCode(settings);
			connect(runCodeThread, SIGNAL(finished()), run, SLOT(deleteLater()));
			connect(run, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));
			connect(runCodeThread, SIGNAL(started()), run, SLOT(startRunCode()));
			connect(run, SIGNAL(finished(QThread*)), this, SLOT(joinThread(QThread*)));
			run->moveToThread(runCodeThread);
			runCodeThread->start();
			this->judgeMap.insert(tid, judge);
//		}

	}catch(JudgeError error){
		switch(error){
			case TempDirCannotBuild:
				showStdOut(tr("评测出现错误: temp文件夹无法找到或打开, 或权限不足."));
				qCritical() << "Judge Error: temp directory cannot enter, maybe permission denied?";
				break;
			case FileCannotOpen:
				showStdOut(tr("评测出现错误: 程序源码文件无法存储, 写入失败."));
				qCritical() << "Judge Error: program source cannot write, maybe permission denied?";
				break;
			case LanguageNotSupport:
				showStdOut(tr("评测出现错误: 源码语言不支持, 操作失败."));
				qCritical() << "Judge Error: language not support";
				break;
			case CompilerNotSupport:
				showStdOut(tr("评测出现错误: 编译器不支持这种语言, 操作失败."));
				qCritical() << "Judge Error: compiler do not support this language";
				break;
			case CompileFailed:
				showStdOut(tr("评测出现错误: 编译器启动失败."));
				qCritical() << "Judge Error: compiler failed to start.";
				break;
			case InputFileCannotOpen:
				showStdOut(tr("评测出现错误: 无法写入输入文件."));
				qCritical() << "Judge Error: cannot write inputfile, maybe permission denied?";
				break;
			case OutputFileCannotOpen:
				showStdOut(tr("评测出现错误: 无法写入输出文件."));
				qCritical() << "Judge Error: cannot write outputfile, maybe permission denied?";
				break;
		}
		result.status = SystemError;
		result.timeUsed = 0;
		result.memoryUsed = 0;
		result.language = judge.language;
		result.message = tr("评测机出现系统错误, 请与评测机管理员联系获取更多支持信息. 错误码:%1").arg(error);
		this->db->setJudgeResult(result, judge.programSource);
		emit this->sendJudge(tid, result);
		return;
	}


//	source.remove('\r');
//	QStringList sourceList = source.split("\n");
//	foreach(auto p, sourceList){
//		this->showStdOut(p);
//	}
}

void MainWindow::receiveUserInformation(qint64 tid, QString user, QString token)
{
	Q_UNUSED(token);
	User userInfo = this->db->getUserInformation(user);
	if(userInfo.userId == -1){
		emit this->sendMessage(tid, tr("用户%1不存在.").arg(user));
	}else{
		emit this->sendUserInformation(tid, userInfo);
	}
}

void MainWindow::receiveRunCodeName(qint64 tid, QString name)
{
	showStdOut(tr("收到评测机 %1 发来的评测机名称: %2").arg(tid).arg(name));
}

void MainWindow::receiveRunCodeStatus(qint64 tidRunCode, RunCodeJudgeResult result)
{
	showStdOut(tr("收到评测机 %1 发来的评测结果.").arg(tidRunCode));
	bool toInt = false;
	qint64 tid = result.name.toLongLong(&toInt);
	if(!toInt){
		qWarning() << "Runcode send a invaild tid for name: name:" << result.name;
		return;
	}
	Judge j = judgeMap.value(tid, Judge());
	if(j.judgeId == 0){
		qCritical() << "this tid cannot found in a map. tid:" << tid;
		return;
	}
	judgeMap.take(tid);

	int index = judgeIndex.value(tid, -1);
	if(index == -1){
		qCritical() << "this tid cannot found any in index map:" << tid;
		qCritical() << "Danger! Default to match index 0.";
		index = 0;
	}
	this->judgeIndex.take(tid);
	//答案对比
	Status res;
	QString message = this->judgeMessage.take(tid);
	showStdOut(tr("评测机%1正在为用户%2(%3)进行答案评析...").arg(tidRunCode).arg(tid).arg(j.username));
	if(result.status == 0){
		//如果运行成功
		QFile fout(QString::number(tid).append(".") + QString::number(j.judgeId).append(".") + QString::number(index).append(".sout"));
		QByteArray stdByteArray = fout.readAll();
		Assessment a(QString(stdByteArray), result.outputString, this);
		res = a.getResult();
	}else{
		//如果运行失败
		res = (Status)result.status;
	}
	this->db->setQuestionPass(j.questionId, res == Accepted?true:false);
	showStdOut(tr("评测机%1为用户%2(%3)评测的结果是: 题目%4, 结果%5").arg(tidRunCode).arg(tid).arg(j.username).arg(j.questionId).arg(this->db->getJudgeStatusString(res)));

	JudgeResult jResult;
	jResult.judgeId = j.judgeId;
	jResult.judgeTime = (int)QDateTime::currentDateTime().toTime_t();
	jResult.language = j.language;
	jResult.memoryUsed = result.memoryUsed;
	jResult.questionId = j.questionId;
	jResult.status = (Status)res;
	jResult.timeUsed = result.timeUsed;
	jResult.username = j.username;
	jResult.message = message;
	this->db->setJudgeResult(jResult, j.programSource);
	emit this->sendJudge(tid, jResult);

}

void MainWindow::startSystem()
{
	showStdOut(tr("正在检查配置文件..."));
	ns.port = config->getNetworkPort();
	if(ns.port == 0){
		qCritical() << "Config file error: network port invaild or not defined.";
		showStdOut(tr("配置文件错误: 未找到网络端口设置或端口设置错误, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: network port:" << ns.port;
		showStdOut(tr("配置: 读取网络端口: %1.").arg(ns.port));
	}

	ns.portJudge = config->getNetworkPortJudge();
	if(ns.portJudge == 0){
		qCritical() << "Config file error: network portJudge invaild or not defined.";
		showStdOut(tr("配置文件错误: 未找到评测网络端口设置或端口设置错误, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: network portJudge:" << ns.portJudge;
		showStdOut(tr("配置: 读取网络评测端口: %1.").arg(ns.portJudge));
	}

	if(ns.port == ns.portJudge){
		qCritical() << "Comfig file error: network and judge port is same!";
		showStdOut(tr("配置文件错误: 客户端网络端口不能和评测机网络端口相同, 请正确配置config.ini文件."));
		return;
	}

	ns.hostJudge = config->getNetworkHostJudge();
	if(ns.hostJudge.isEmpty()){
		qCritical() << "Comfig file error: Judge host not set ot invaild.";
		showStdOut(tr("配置文件错误: 主机相较于评测机的地址未设置或无效, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: Judge host:" << ns.hostJudge;
		showStdOut(tr("配置: 主机相较于评测机地址: %1").arg(ns.hostJudge));
	}

	ds.databaseName = config->getDatabaseDatabaseName();
	if(ds.databaseName.isEmpty()){
		qCritical() << "Config file error: database name inavild or not defined.";
		showStdOut(tr("配置文件错误: 未找到数据库名设置, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: database name:" << ds.databaseName;
		showStdOut(tr("配置: 读取数据库名称: %1").arg(ds.databaseName));
	}

	ds.hostName = config->getDatabaseHostName();
	if(ds.hostName.isEmpty()){
		qCritical() << "Config file error: database hostname inavild or not defined.";
		showStdOut(tr("配置文件错误: 未找到数据库地址设置, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: database host:" << ds.hostName;
		showStdOut(tr("配置: 读取数据库位置: %1").arg(ds.hostName));
	}

	ds.password = config->getDatabasePassword();

	ds.port = config->getDatabasePort();
	if(ds.port <= 0){
		qCritical() << "Config file error: database port inavild or not defined.";
		showStdOut(tr("配置文件错误: 未找到数据库端口设置, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: database port:" << ds.port;
		showStdOut(tr("配置: 读取数据库端口: %1").arg(ds.port));
	}

	ds.userName = config->getDatabaseUserName();
	if(ds.userName.isEmpty()){
		qCritical() << "Config file error: database username inavild or not defined.";
		showStdOut(tr("配置文件错误: 未找到数据库用户设置, 请正确配置config.ini文件."));
		return;
	}else{
		qInfo() << "Config: database user:" << ds.userName;
		showStdOut(tr("配置: 读取数据库用户名: %1").arg(ds.userName));
	}

	cp.c = config->getCompileC();
	if(cp.c.isEmpty()){
		cp.c = "gcc";
		qWarning() << "Config file warning: compiler for c not set, use gcc instead.";
		showStdOut(tr("配置文件警告: 未找到编译器C语言处理程序, 默认使用gcc."));
	}
	if(!QFile::exists(cp.c)){
		QString gccPath = QStandardPaths::findExecutable(cp.c);
		if(gccPath.isEmpty()){
			qCritical() << "Compile path: gcc compile not found:" << cp.c;
			showStdOut(tr("指定传统gcc编译器失败, 请查阅系统环境PATH变量, 或在配置文件中指定编译器执行文件地址."));
			return;
		}else{
			qInfo() << "Config: gcc path:" << gccPath;
			showStdOut(tr("配置: 找到gcc文件位置: %1").arg(gccPath));
		}
	}else{
		qInfo() << "Config: gcc path:" << cp.c;
		showStdOut(tr("配置: 读取gcc文件位置: %1").arg(cp.c));
	}

	cp.cCmd = config->getCompileCCmd();
	if(cp.cCmd.isEmpty()){
		qWarning() << "User never config the C compiler execute command.";
		qWarning() << "Use \"-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std=c99 -fno-asm {SOURCE}\" instead.";
		showStdOut(tr("配置文件警告: 未设置C语言编译器执行命令参数."));
		showStdOut(tr("使用\"-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std=c99 -fno-asm {SOURCE}\"代替"));
		cp.cCmd = QString("-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std=c99 -fno-asm {SOURCE}");
	}else{
		qInfo() << "User configed the C complier execute command:" << cp.cCmd;
		showStdOut(tr("配置: 找到C语言编译器执行命令参数: %1").arg(cp.cCmd));
	}

	cp.cpp = config->getCompileCpp();
	if(cp.cpp.isEmpty()){
		cp.cpp = "g++";
		qWarning() << "Config file warning: compiler for c++ not set, use g++ instead.";
		showStdOut(tr("配置文件警告: 未找到编译器C++语言处理程序, 默认使用g++."));
	}
	if(!QFile::exists(cp.cpp)){
		QString gppPath = QStandardPaths::findExecutable(cp.cpp);
		if(gppPath.isEmpty()){
			qCritical() << "Compile path: g++ compile not found:" << cp.c;
			showStdOut(tr("指定传统g++编译器失败, 请查阅系统环境PATH变量, 或在配置文件中指定编译器执行文件地址."));
			return;
		}else{
			qInfo() << "Config: g++ path:" << gppPath;
			showStdOut(tr("配置: 找到g++文件位置: %1").arg(gppPath));
		}
	}else{
		qInfo() << "Config: g++ path:" << cp.cpp;
		showStdOut(tr("配置: 读取g++文件位置: %1").arg(cp.cpp));
	}

	cp.cppCmd = config->getCompileCppCmd();
	if(cp.cppCmd.isEmpty()){
		qWarning() << "User never config the C++ compiler execute command.";
		qWarning() << "Use \"-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std-c98 -fno-asm {SOURCE}\" instead.";
		showStdOut(tr("配置文件警告: 未设置C++语言编辑器执行命令参数."));
		showStdOut(tr("使用\"-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std-c98 -fno-asm {SOURCE}\"代替"));
		cp.cppCmd = QString("-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std-c98 -fno-asm {SOURCE}");
	}else{
		qInfo() << "User configed the C++ complier execute command:" << cp.cppCmd;
		showStdOut(tr("配置: 找到C++语言编译器执行命令参数: %1").arg(cp.cppCmd));
	}

	cp.java = config->getCompileJava();
	if(cp.java.isEmpty()){
		cp.java = "javac";
		qWarning() << "Config file warning: compiler for java not set, use javac instead.";
		showStdOut(tr("配置文件警告: 未找到编译器Java语言处理程序, 默认使用javac."));
	}
	if(!QFile::exists(cp.java)){
		QString javaPath = QStandardPaths::findExecutable(cp.java);
		if(javaPath.isEmpty()){
			qCritical() << "Compile path: java compile not found:" << cp.c;
			showStdOut(tr("指定传统javac编译器失败, 请查阅系统环境PATH变量, 或在配置文件中指定编译器执行文件地址."));
			return;
		}else{
			qInfo() << "Config: javac path:" << javaPath;
			showStdOut(tr("配置: 找到javac文件位置: %1").arg(javaPath));
		}
	}else{
		qInfo() << "Config: javac path:" << cp.java;
		showStdOut(tr("配置: 读取javac文件位置: %1").arg(cp.java));
	}

	cp.javaCmd = config->getCompileJavaCmd();
	if(cp.javaCmd.isEmpty()){
		qWarning() << "User never config the Java compiler execute command.";
		qWarning() << "Use \"{SOURCE}\" instead.";
		showStdOut(tr("配置文件警告: 未设置C++语言编辑器执行命令参数."));
		showStdOut(tr("使用\"{SOURCE}\"代替"));
		cp.javaCmd = QString("{SOURCE}");
	}else{
		qInfo() << "User configed the Java complier execute command:" << cp.javaCmd;
		showStdOut(tr("配置: 找到Java语言编译器执行命令参数: %1").arg(cp.javaCmd));
	}

	ud.useDocker = config->getGlobalUseDocker();
	if(ud.useDocker < 0){
		qCritical() << "Config file error: docker function inavild or not defined.";
		showStdOut(tr("配置文件错误: 未找到docker配置, 请正确配置config.ini文件."));
		return;
	}else if(ud.useDocker == 0){
		qInfo() << "Server judge without Docker. DANGER!";
		showStdOut(tr("配置文件信息: 服务器开启为非安全模式, 将在本机直接运行评测, 危险的操作."));
	}else if(ud.useDocker == 1){
		ud.docker = this->config->getGlobalDocker();
		qInfo() << "Server judge using Docker.";
		showStdOut(tr("配置文件信息: 服务器开启使用Docker, 正在检测Docker配置."));
		if(!QFile::exists(ud.docker)){
			QString dockerPath = QStandardPaths::findExecutable(ud.docker);
			if(dockerPath.isEmpty()){
				qCritical() << "Undefined Docker path. Settings invaild.";
				showStdOut(tr("配置文件错误: Docker执行地址设置错误, 请正确配置config.ini设置."));
				return;
			}else{
				qInfo() << "Get docker path:" << dockerPath;
				showStdOut(tr("检测到Docker地址:%1").arg(dockerPath));
			}
		}else{
			qInfo() << "Get docker path:" << ud.docker;
			showStdOut(tr("检测到Docker地址:%1").arg(ud.docker));
		}
	}

	qInfo() << "Attecting Docker daemon if is running.";
	showStdOut(tr("判断Docker的运行状态..."));
	ud.useDocker = config->getGlobalUseDocker();
	if(ud.useDocker < 0){
		qCritical() << "Undefined Docker status settings. Settings invaild.";
		showStdOut(tr("配置文件错误: 未设置Docker的运行状态, 请正确配置config.ini设置."));
		return;
	}else if(ud.useDocker == 1){
		ud.dockerSocketPath = config->getGlobalDockerSocketPath();
		if(ud.dockerSocketPath.isEmpty()){
			qCritical() << "Invaild Docker socket path.";
			showStdOut(tr("配置文件错误: 未设置Docker Socket的位置, 请正确配置config.ini设置."));
			return;
		}
		if(QFile::exists(ud.dockerSocketPath)){
			qInfo() << "Docker daemon is running.";
			showStdOut(tr("Docker daemon在运行中, 程序使用Docker作为容器进行评测."));
		}else{
			qCritical() << "Docker daemon is not running!";
			showStdOut(tr("Docker daemon未在运行, 请使用 \"sudo docker daemon\" 启动Docker主机, 然后再试一次."));
			return;
		}
	}

	if(ud.useDocker == 0){
		qInfo() << "Load Runcode program path.";
		showStdOut(tr("载入评测机位置..."));
		ud.runCodePath = config->getGlobalRunCodePath();
		if(ud.runCodePath.isEmpty()){
			qCritical() << "Invaild runCode program path.";
			showStdOut(tr("配置文件错误: 为正确设置评测软件位置, 请正确配置config.ini设置."));
			return;
		}else{
			if(ud.useDocker == false){
				if(!QFile::exists(ud.runCodePath)){
					QString runCodePath = QStandardPaths::findExecutable(ud.runCodePath);
					if(runCodePath.isEmpty()){
						qCritical() << "RunCode file not found:" << ud.runCodePath;
						showStdOut(tr("指定评测机RunCode失败, 请查阅系统环境PATH变量, 或在配置文件中指定评测机执行文件地址."));
						return;
					}else{
						qInfo() << "Config: javac path:" << runCodePath;
						showStdOut(tr("配置: 找到RunCode文件位置: %1").arg(runCodePath));
					}
				}else{
					qInfo() << "Config: RunCode path:" << ud.runCodePath;
					showStdOut(tr("配置: 读取RunCode文件位置: %1").arg(ud.runCodePath));
				}
			}
		}
	}


	qInfo() << "Request to open database...";
	showStdOut(tr("请求连接数据库..."));
	this->db = new Database(ds, this);
	connect(this->db, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));
	if(!this->db->open()){
		qCritical() << "Database error, database connect failed.";
		showStdOut(tr("数据库错误: 连接数据库失败."));
		return;
	}

	qInfo() << "Request to open network...";
	showStdOut(tr("请求打开网络端口..."));
	this->net = new Network(ns.port, this);
	connect(this->net, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));
	if(!this->net->startServer()){
		qCritical() << "Network error, network start failed.";
		showStdOut(tr("网络错误: 网络端口无法开启."));
		return;
	}

	qInfo() << "Starting network communication server.";
	showStdOut(tr("启动网络处理系统..."));
	this->netC = new NetworkCommunication(this->net, this);
	connect(this->netC, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));
	//connect

	qInfo() << "Request to open judgeNetwork...";
	showStdOut(tr("请求打开评测端口..."));
	this->netRunCode = new Network(ns.portJudge, this);
	connect(this->netRunCode, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));
	if(!this->netRunCode->startServer()){
		qCritical() << "Network Judge error, network start failed.";
		showStdOut(tr("网络错误: 网络评测端口无法开启."));
		return;
	}

	qInfo() << "Starting network judge communication server.";
	showStdOut(tr("启动网络评测处理系统..."));
	this->netRunCodeC = new NetworkJudgeCommunication(this->netRunCode, this);
	connect(this->netRunCodeC, SIGNAL(stdOut(QString)), this, SLOT(showStdOut(QString)));

	qInfo() << "Maybe all ready server had been checking successfully!";
	showStdOut(tr("所有子系统应该都成功启动了吧..."));

	showStdOut(tr("打开系统网络请求监听器..."));
	connect(this->netC, SIGNAL(requestQuestionList(qint64)), this, SLOT(receiveQuestionList(qint64)));
	connect(this->netC, SIGNAL(requestQuestionContent(qint64,int)), this, SLOT(receiveQuestionContent(qint64,int)));
	connect(this->netC, SIGNAL(requestJudge(qint64,Judge)), this, SLOT(receiveJudge(qint64,Judge)));
	connect(this->netC, SIGNAL(requestUserInformation(qint64,QString,QString)), this, SLOT(receiveUserInformation(qint64,QString,QString)));
	connect(this, SIGNAL(sendQuestionList(qint64,QList<QuestionBrief>)), this->netC, SLOT(responseQuestionList(qint64,QList<QuestionBrief>)));
	connect(this, SIGNAL(sendQuestionContent(qint64,Question)), this->netC, SLOT(responseQuestionContent(qint64,Question)));
	connect(this, SIGNAL(sendJudge(qint64,JudgeResult)), this->netC, SLOT(responseJudge(qint64,JudgeResult)));
	connect(this, SIGNAL(sendUserInformation(qint64,User)), this->netC, SLOT(responseUserInformation(qint64,User)));

	connect(this->netRunCodeC, SIGNAL(requestRunCodeName(qint64,QString)), this, SLOT(receiveRunCodeName(qint64,QString)));
	connect(this->netRunCodeC, SIGNAL(requestRunCodeStatus(qint64,RunCodeJudgeResult)), this, SLOT(receiveRunCodeStatus(qint64,RunCodeJudgeResult)));

	connect(this, SIGNAL(sendMessage(qint64,QString)), this->netC, SLOT(responseMessage(qint64,QString)));
	showStdOut(tr("网络请求监听器已开启."));

	showStdOut(tr("等待网络接口用户接入..."));
}
