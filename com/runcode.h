#ifndef RUNCODE_H
#define RUNCODE_H

#include <QObject>
#include <QtCore>
#include "global.h"

class RunCode : public QObject
{
	Q_OBJECT
	QProcess *runcode;
	RunCodeSettings rcs;
	QString runFile;
	QString inputFile;

	bool isReady;
	bool isComplete;

	bool createDocker(const QString &name, const QString &image, QByteArray *id = nullptr);
	bool cpRunFile(const QString &name, const QString &file);
	bool cpInputFile(const QString &name, const QString &file);
	bool startDocker(const QString &name);

	bool rmDocker(const QString &name);

public:
	explicit RunCode(RunCodeSettings rcs, QObject *parent = 0);
	bool isReadyNow();
	bool isCompleteNow();

signals:
	void stdOut(QString msg);
	void finished(QThread*);
public slots:
	void startRunCode();
};

#endif // RUNCODE_H
