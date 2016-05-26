#include "runcode.h"

bool RunCode::createDocker(const QString &name, const QString &image, QByteArray *id)
{
	if(name.isEmpty()){
		return false;
	}
	QProcess p(this);
	p.setProgram(rcs.dockerPath);
	QStringList arg;
	arg << "create" << "--name" << name << "-t" << "--memory-swap=-1" << image;
	p.setArguments(arg);
	p.start();
	if(!p.waitForStarted()){
		return false;
	}
	p.closeWriteChannel();
	if(!p.waitForFinished()){
		return false;
	}
	QByteArray *result = new QByteArray(p.readAll());
	if(id != nullptr){
		id = result;
	}
	int exitCode = p.exitCode();
	if(exitCode == 0){
		return true;
	}else{
		return false;
	}
}

bool RunCode::cpRunFile(const QString &name, const QString &file)
{
	if(name.isEmpty()){
		return false;
	}
	QProcess p(this);
	p.setProgram(rcs.dockerPath);
	QStringList arg;
	arg << "cp" << file << QString("%1:/home/").arg(name);
	p.setArguments(arg);
	p.start();
	if(!p.waitForStarted()){
		return false;
	}
	p.closeWriteChannel();
	if(!p.waitForFinished()){
		return false;
	}
	int exitCode = p.exitCode();
	QFileInfo inf(file);
	this->runFile = "/home/" + inf.fileName();
	if(exitCode == 0){
		return true;
	}else{
		return false;
	}
}

bool RunCode::cpInputFile(const QString &name, const QString &file)
{
	if(name.isEmpty()){
		return false;
	}
	QProcess p(this);
	p.setProgram(rcs.dockerPath);
	QStringList arg;
	arg << "cp" << file << QString("%1:/home/").arg(name);
	p.setArguments(arg);
	p.start();
	if(!p.waitForStarted()){
		return false;
	}
	p.closeWriteChannel();
	if(!p.waitForFinished()){
		return false;
	}
	int exitCode = p.exitCode();
	QFileInfo inf(file);
	this->inputFile = "/home/" + inf.fileName();
	if(exitCode == 0){
		return true;
	}else{
		return false;
	}
}

bool RunCode::startDocker(const QString &name)
{
	if(name.isEmpty()){
		return false;
	}
	QProcess p(this);
	p.setProgram(rcs.dockerPath);
	QStringList arg;
	arg << "start" << name;
	p.setArguments(arg);
	p.start();
	if(!p.waitForStarted()){
		return false;
	}
	p.closeWriteChannel();
	if(!p.waitForFinished()){
		return false;
	}
	QByteArray result = p.readAll();
	if(QString::compare(QString(result), name)){
		qDebug() << "startDocker result stdout:" << result << "name:" << name << "is not same!";
	}
	int exitCode = p.exitCode();
	if(exitCode == 0){
		return true;
	}else{
		return false;
	}
}

bool RunCode::rmDocker(const QString &name)
{
	if(name.isEmpty()){
		return false;
	}
	QProcess p(this);
	p.setProgram(rcs.dockerPath);
	QStringList arg;
	arg << "rm" << name;
	p.setArguments(arg);
	p.start();
	if(!p.waitForStarted()){
		return false;
	}
	p.closeWriteChannel();
	if(!p.waitForFinished()){
		return false;
	}
	QByteArray result = p.readAll();
	if(QString::compare(QString(result), name)){
		qDebug() << "rmDocker result stdout:" << result << "name:" << name << "is not same!";
	}
	int exitCode = p.exitCode();
	if(exitCode == 0){
		return true;
	}else{
		return false;
	}
}



RunCode::RunCode(RunCodeSettings rcs, QObject *parent) : QObject(parent)
{
	this->rcs = rcs;
	this->runcode = new QProcess(this);
	this->isReady = false;
	this->isComplete = false;
	if(!QFile::exists(rcs.runCodeFilePath)){
		emit this->stdOut(tr("未找见RunCode的执行路径"));
		this->isReady = false;
		return;
	}
	if(!rcs.useDocker){
		this->runcode->setProgram(rcs.runCodeFilePath);
		QStringList arg;
		arg << "-r" << rcs.execfile;
		arg << "-n" << QString("%1:%2").arg(rcs.host).arg(rcs.port);
		arg << "-N" << QString::number(rcs.tid);
		arg << "-t" << QString::number(rcs.timeLimit);
		arg << "-m" << QString::number(rcs.memoryLimit);
		if(this->rcs.outputLimit > 0){
			arg << "-o" << QString::number(rcs.outputLimit);
		}
		if(!rcs.inputfile.isEmpty()){
			arg << "-f" << rcs.inputfile;
		}
		qDebug() << "Runcode cmd:" << rcs.runCodeFilePath << arg;
		this->isReady = true;
	}else{
		QString tidName = QString::number(rcs.tid);
		if(!createDocker(tidName, rcs.image)){
			qCritical() << "Docker continer create failed. tid:" << rcs.tid << "image" << rcs.image;
			return;
		}
		if(!cpRunFile(tidName, rcs.execfile)){
			qCritical() << "Docker cp runfile failed.tid:" << rcs.tid << "execfile" << rcs.execfile;
			return;
		}
		if(!rcs.inputfile.isEmpty()){
			if(!cpInputFile(tidName, rcs.inputfile)){
				qCritical() << "Docker cp inputFile failed.tid:" << rcs.tid << "inputfile" << rcs.inputfile;
				return;
			}
		}
		if(!startDocker(tidName)){
			qCritical() << "Docker start failed. tid:" << rcs.tid;
			return;
		}
		this->isReady = true;
	}

}

bool RunCode::isReadyNow(){
	return this->isReady;
}

bool RunCode::isCompleteNow(){
	return this->isComplete;
}

void RunCode::startRunCode()
{
	if(rcs.useDocker == 1){
		if(rcs.dockerPath.isEmpty()){
			qCritical() << "docker path is empty";
			return;
		}
		if(rcs.tid < 0){
			qCritical() << "rcs.tid is invaild.";
			return;
		}
		if(rcs.runCodeFilePath.isEmpty()){
			qCritical() << "rcs.runCodeFilePath is empty";
			return;
		}
		QString tidName = QString::number(rcs.tid);
		QProcess p(this);
		p.setProgram(rcs.dockerPath);
		QStringList arg;
		arg << "exec" << tidName << rcs.runCodeFilePath;
		arg << "-r" << this->runFile << "-N" << tidName << "-n" << QString("%1:%2").arg(rcs.host).arg(rcs.port);
		arg << "-t" << QString::number(rcs.timeLimit) << "-m" << QString::number(rcs.memoryLimit);
		if(this->rcs.outputLimit > 0){
			arg << "-o" << QString::number(rcs.outputLimit);
		}
		if(!this->inputFile.isEmpty()){
			arg << "-f" << this->inputFile;
		}
		p.setArguments(arg);
		p.start();
		if(!p.waitForStarted()){
			qCritical() << "docker failed to start.";
			return;
		}
		if(!p.waitForFinished()){
			qCritical() << "Docker failed to stop.";
			return;
		}
		int exitCode = p.exitCode();
		if(p.exitStatus() == QProcess::CrashExit){
			qCritical() << "Docker crashed.";
			return;
		}
		this->isComplete = true;
	}else{
		if(rcs.tid < 0){
			qCritical() << "docker path is empty";
			return;
		}
		if(rcs.runCodeFilePath.isEmpty()){
			qCritical() << "rcs.runCodeFilePath is empty";
			return;
		}
		QString tidName = QString::number(rcs.tid);
		qDebug() << "tidName" << tidName;
		QProcess p(this);
		p.setProgram(rcs.runCodeFilePath);
		QStringList arg;
		arg << "-r" << /*this->runFile*/rcs.execfile << "-N" << tidName << "-n" << QString("%1:%2").arg(rcs.host).arg(rcs.port);
		arg << "-t" << QString::number(rcs.timeLimit) << "-m" << QString::number(rcs.memoryLimit);
		if(this->rcs.outputLimit > 0){
			arg << "-o" << QString::number(rcs.outputLimit);
		}
		if(!rcs.inputfile.isEmpty()){
			arg << "-f" << this->inputFile;
		}
		qDebug() << "args" << arg;
		p.setArguments(arg);
		p.start();
		if(!p.waitForStarted()){
			qCritical() << "RunCode failed to start. info:" << p.errorString();
			return;
		}
		qDebug() << "RunCode started.";
		if(!p.waitForFinished()){
			qCritical() << "RunCode failed to stop. info:" << p.errorString();
			return;
		}
		qDebug() << "RunCode Finished.";
		QByteArray result = p.readAll();
		emit this->stdOut(QString(result));
		int exitCode = p.exitCode();
		if(p.exitStatus() == QProcess::CrashExit){
			qCritical() << "RunCode crashed. info:" << p.errorString();
			return;
		}
		this->isComplete = true;
	}
	emit this->finished(QThread::currentThread());
}

