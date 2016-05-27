#include "compile.h"

//void Compile::setGccPath(const QString &path){
//	this->gccPath = path;
//}

//void Compile::setGppPath(const QString &path){
//	this->gppPath = path;
//}

//void Compile::setJavaPath(const QString &path){
//	this->javaPath = path;
//}

//void Compile::setGccCmd(const QString &cmd){
//	this->gccCmd = cmd;
//}

//void Compile::setGppCmd(const QString &cmd){
//	this->gppCmd = cmd;
//}

//void Compile::setJavaCmd(const QString &cmd){
//	this->javaCmd = cmd;
//}

bool Compile::killProcess(){
	if(this->compiler->state() != QProcess::NotRunning){
		this->compiler->terminate();
		this->initiativeStopped = true;
		bool reply = this->compiler->waitForFinished(1000);
		if(reply == false){
			if(this->compiler->state() != QProcess::NotRunning){
				this->compiler->kill();
			}
		}
	}
	if(this->compiler->state() != QProcess::NotRunning){
		initiativeStopped = false;
		return false;
	}else{
		return true;
	}
}

Compile::Compile(QString sourceFile, QObject *parent): QObject(parent){
	//		this->gccPath = "gcc";
	//		this->gppPath = "g++";
	//		this->javaPath = "javac";
	//		this->gccCmd = "-O2 -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std=c99 -fno-asm {SOURCE}";
	//		this->gppCmd = "-O2 -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std-c98 -fno-asm {SOURCE}";
	//		this->javaCmd = "{SOURCE}";

	this->compiler = new QProcess(this);
	this->sourceFile = sourceFile;

	QFileInfo info(sourceFile);
	QString folder = info.path();
	QString suffix = info.suffix();
	QString name = info.completeBaseName();

	this->targetFile = folder + "/" + name + QString(".run");
	qDebug() << "Target file name:" << this->targetFile;
	if(!QString::compare(suffix, "c", Qt::CaseInsensitive)){
		// this file may a c file;
		this->compilePath = "gcc";
		this->compileCmd = "-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std=c99 -fno-asm {SOURCE}";
	}else if(!QString::compare(suffix, "cpp", Qt::CaseInsensitive)){
		// this file may a c++ file;
		this->compilePath = "g++";
		this->compileCmd = "-O2 -fno-diagnostics-color -fno-diagnostics-show-option -fno-diagnostics-show-caret -o {TARGET} -DONLINE_JUDGE -Wall -lm --static --std-c98 -fno-asm {SOURCE}";
	}else if(!QString::compare(suffix, "java", Qt::CaseInsensitive)){
		// this file may a java file;
		this->compilePath = "java";
		this->compileCmd = "{SOURCE}";
	}else{
		qWarning() << "PrevCompiler Attach an error, source suffix is not c, c++ or java.";
		return;
	}



	this->warningNum = 0;
	this->errorNum = 0;
	this->exitCode = -1;

	this->compileComplete = false;

	connect(this->compiler, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
//	connect(this->compiler, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));

}

void Compile::setCompilePath(const QString &path){
	this->compilePath = path;
}

void Compile::setCompileCmd(const QString &cmd){
	this->compileCmd = cmd;
}

QString Compile::getStdErr()
{
	return this->stdErr;
}

int Compile::getExitCode(){
	return this->exitCode;
}

bool Compile::isCompileComplete()
{
	return this->compileComplete;
}

QString Compile::getTargetFilePath(){
	return this->targetFile;
}

void Compile::finished(int exitCode, QProcess::ExitStatus status)
{
	this->exitCode = exitCode;
	if(status == QProcess::CrashExit && this->initiativeStopped){
		qCritical() << "Complier crashed by user initiative stop, compile failed";
		emit this->stdOut(tr("编译器异常停止, 或被管理器结束, 本次编译失败, 发送系统错误."));
	}else if(status == QProcess::CrashExit){
		qCritical() << "Complier crashed by itself, compile failed";
		emit this->stdOut(tr("编译器自身异常停止, 本次编译失败, 发送系统错误."));
	}else if(status == QProcess::NormalExit){
//		qInfo() << "Compiler exitted normally code:" << exitCode << "total Errors:" << this->errorNum << "Warnings:" << this->warningNum;
//		emit this->stdOut(tr("编译成功, 共有%1错误, %2警告").arg(this->errorNum).arg(this->warningNum));
	}
}

//void Compile::readStdErr()
//{

//}

void Compile::startCompile()
{
	if(!QFile::exists(this->compilePath)){
		QString compilerAbsPath = QStandardPaths::findExecutable(this->compilePath);
		if(compilerAbsPath.isEmpty()){
			qCritical() << "Compiler not found: path" << this->compilePath;
			emit this->stdOut(tr("指定编译器未找到, 位置:%1, 无法进行编译").arg(this->compilePath));
			return;
		}else{
			//this->compilePath = compilerAbsPath;
		}
	}
	this->compileCmd.replace("{TARGET}", this->targetFile);
	this->compileCmd.replace("{SOURCE}", this->sourceFile);
	QString cmd = /*this->compilePath + " " + */this->compileCmd;
	QStringList arg = cmd.split(' ', QString::SkipEmptyParts);
	qDebug() << "Compile Command:" << this->compilePath << cmd;
//	this->compiler->setProgram(this->compilePath);
//	this->compiler->setArguments(arg);
//	qDebug() << "Arguments" << compiler->arguments();

	this->compiler->start(this->compilePath, arg);
	if(!this->compiler->waitForStarted(10000)){
		qCritical() << "Compiler starting failed:" << this->compilePath << "msg:" << this->compiler->errorString();
		qCritical() << "Compiler command:" << this->compilePath << cmd;
		emit this->stdOut(tr("编译器无法启动, 编译器:%1 信息:%2").arg(this->compilePath).arg(this->compiler->errorString()));
		emit this->stdOut(tr("编译命令: %1 %2").arg(this->compilePath).arg(cmd));
		this->compileComplete = false;
		return;
	}
	this->compiler->closeWriteChannel();
	if(!this->compiler->waitForFinished(30000)){
		qCritical() << "Compiler ran out of time (30s), Compiler compile too slow! msg:" << this->compiler->errorString();
		emit this->stdOut(tr("编译器编译时间超出阀值30s, 运行过程中被阻塞, 强制退出. 信息:%1").arg(this->compiler->errorString()));
		this->compileComplete = false;
		return;
	}
	this->compiler->setReadChannel(QProcess::StandardError);
	QByteArray rawErr = this->compiler->readAllStandardError();
	if(!rawErr.isEmpty()){
		rawErr.replace(QByteArray("\r"), QByteArray());
		QList<QByteArray> rawErrList = rawErr.split('\n');
		foreach (auto p, rawErrList) {
			if(p.isEmpty()){
				continue;
			}
			QList<QByteArray> pList = p.split(':');
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
			if(pList.length() >= 6){
#else
			if(pList.length() >= 5){
#endif
				//pattern:
				// hello.c : 18 : 6 : warning : return type of 'main' is not 'int'
				// FILENAME LINE COL  LEVEL     MESSAGE
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
				// E : /MyQt/RuilxJudgeServer0v20/temp/374669656.1000.c : 2 : 2 : warning: implicit declaration of function 'puts'
				// FILENAME                                             LINE COL  LEVEL    MESSAGE
				if(pList.at(0).length() == 1 && !pList.at(0).startsWith('/') && !pList.at(0).startsWith('.')){
					pList.takeFirst();
				}
#endif
				if(pList.at(0).startsWith(' ')){
					continue;
				}
				//QString filename = QString(pList.at(0)); // hello.c
				QString line = QString(pList.at(1)); //18
				QString col = QString(pList.at(2)); //6
				QString level = QString(pList.at(3)); //warning
				if(level.contains("warning") || level.contains("警告")){
					this->warningNum++;
				}else if(level.contains("error") || level.contains("错误")){
					this->errorNum++;
				}
				QString message;
				for(int i=4; i<pList.length(); i++){
					message += pList.at(i); //return type of 'main' is not 'int'
				}
				QString msg = QString("%1: %2: %3: %4\n").arg(level).arg(line).arg(col).arg(message);
				this->stdErr.append(msg);
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
			}else if(pList.length() >= 5){
#else
			}else if(pList.length() >= 4){
#endif
				//pattern:
				//Main.java : 5 : error: ';' expected
				//FILENAME   LINE LEVEL  MESSAGE
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
				if(pList.at(0).length() == 1 && !pList.at(0).startsWith('/') && !pList.at(0).startsWith('.')){
					pList.takeFirst();
				}
#endif
				if(pList.at(0).startsWith(' ')){
					continue;
				}
				//QString filename = QString(pList.at(0)); //Main.java
				QString line = QString(pList.at(1)); //5
				QString level = QString(pList.at(2)); //error
				if(level.contains("warning") || level.contains("警告")){
					this->warningNum++;
				}else if(level.contains("error") || level.contains("错误")){
					this->errorNum++;
				}
				QString message;
				for(int i=3; i<pList.length(); i++){
					message += pList.at(i);
				}
				QString msg = QString("%1: %2: %3\n").arg(level).arg(line).arg(message);
				this->stdErr.append(msg);
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
			}else if(pList.length() >= 4){
#else
			}else if(pList.length() >= 3){
#endif
				//pattern:
				//hello.c : In function 'main':
				//FILENAME  MESSAGE            EMPTY
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
				if(pList.at(0).length() == 1 && !pList.at(0).startsWith('/') && !pList.at(0).startsWith('.')){
					pList.takeFirst();
				}
#endif
				if(pList.at(0).startsWith(' ')){
					continue;
				}
				//QString filename = QString(pList.at(0)); //hello.c
				QString message;
				for(int i=1; i<pList.length(); i++){
					message += pList.at(i);
				}
				QString msg = QString("%1\n").arg(message);
				this->stdErr.append(msg);
			}else{
				continue;
			}
		}
	}
	this->compileComplete = true;
	qInfo() << "Compiler exitted normally code:" << exitCode << "total Errors:" << this->errorNum << "Warnings:" << this->warningNum;
	emit this->stdOut(tr("编译成功, 共有%1错误, %2警告").arg(this->errorNum).arg(this->warningNum));
}

void Compile::stopCompile()
{
	killProcess();
}
