#include "config.h"

Config::Config(QString filename, QObject *parent) : QObject(parent)
{
	this->ini = new QSettings(filename, QSettings::IniFormat);
}

int Config::getNetworkPort(){
	bool ok = false;
	int port = ini->value("network/port", 0).toInt(&ok);
	if((!ok) || port == 0){
		return 0;
	}
	return port;
}

int Config::getNetworkPortJudge()
{
	bool ok = false;
	int port = ini->value("network/portJudge", 0).toInt(&ok);
	if((!ok) || port == 0){
		return 0;
	}
	return port;
}

QString Config::getNetworkHostJudge()
{
	return ini->value("network/hostJudge", QString()).toString();
}

QString Config::getDatabaseHostName(){
	return ini->value("database/hostName", QString()).toString();
}

QString Config::getDatabaseDatabaseName(){
	return ini->value("database/databaseName", QString()).toString();
}

QString Config::getDatabaseUserName(){
	return ini->value("database/userName", QString()).toString();
}

QString Config::getDatabasePassword(){
	return ini->value("database/password", QString()).toString();
}

int Config::getDatabasePort(){
	bool ok = false;
	int port = ini->value("database/port", 0).toInt(&ok);
	if((!ok) || port == 0){
		return 0;
	}
	return port;
}

QString Config::getCompileC(){
	return ini->value("compile/c", QString()).toString();
}

QString Config::getCompileCpp(){
	return ini->value("compile/cpp", QString()).toString();
}

QString Config::getCompileJava(){
	return ini->value("compile/java", QString()).toString();
}

QString Config::getCompileCCmd(){
	return ini->value("compile/cCmd", QString()).toString();
}

QString Config::getCompileCppCmd(){
	return ini->value("compile/cppCmd", QString()).toString();
}

QString Config::getCompileJavaCmd(){
	return ini->value("compile/javaCmd", QString()).toString();
}

int Config::getGlobalUseDocker()
{
	return QString::compare(ini->value("global/useDocker", QString()).toString(), "true", Qt::CaseInsensitive)?
			(QString::compare(ini->value("global/useDocker", QString()).toString(), "false", Qt::CaseInsensitive)?
				 -1 : 0) : 1;
}

QString Config::getGlobalRunCodePath()
{
	return ini->value("global/runCodePath", QString()).toString();
}

QString Config::getGlobalDockerSocketPath()
{
	return ini->value("global/dockerSocketPath", QString()).toString();
}

QString Config::getGlobalDocker()
{
	return ini->value("global/docker", QString()).toString();
}



