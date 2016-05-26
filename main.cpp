#include "mainwindow.h"
#include <QApplication>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg){
	static QMutex mutex;
	mutex.lock();
	QString text;
	switch(type){
		case QtDebugMsg: text = QString("[DEBUG]"); break;
		case QtInfoMsg: text = QString("[INFOR]"); break;
		case QtWarningMsg: text = QString("[WARNI]"); break;
		case QtCriticalMsg: text = QString("[CRTCL]"); break;
		case QtFatalMsg: text = QString("[FATAL]"); break;
		//case QtSystemMsg: text = QString("[SYSIG]"); break;
		default: text = QString("[UNKWN]"); break;
	}
	QString context_info = QString("%1(%2)").arg(context.file).arg(context.line);
	//QString current_date_time = QDateTime::currentDateTime().toString("[yyyy/MM/dd hh:mm:ss.zzz]");
	//QString context_location = QString("%1::%2:").arg(context.category).arg(context.function);
	QString context_location = QString("%2:").arg(context.function);

	QFile file("rjs.log");
	if(!file.open(QIODevice::WriteOnly | QIODevice::Append)){
		printf("[FATAL]LogSystem: Cannot open log file: rjs.log. Failed.");
		mutex.unlock();
		qApp->exit(2);
		return;
	}
	QTextStream text_stream(&file);
	text_stream << text /*<< current_date_time*/ << context_info << context_location << msg;
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
	text_stream << "\r\n";
#elif defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
	text_stream << "\n";
#endif
	file.flush();
	file.close();
	qt_message_output(type, context, msg);
	mutex.unlock();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//qInstallMessageHandler(outputMessage);
	MainWindow w;
	w.show();
//	qDebug() << "MainWindow shown. DEBUG";
//	qInfo() << "MainWindow shown. INFO";
//	qWarning() << "MainWindow shown. WARNING";
//	qCritical() << "MainWindow shown. CRITICAL";
	//qFatal("MainWindow shown. FATAL"); //这货会让程序终止

	return a.exec();
}
