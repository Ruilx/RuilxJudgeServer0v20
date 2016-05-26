#include "database.h"

Database::Database(DatabaseOption database, QObject *parent) : QObject(parent)
{
	this->database = QSqlDatabase::addDatabase(DatabaseDriver);
	this->database.setHostName(database.hostName);
	this->database.setDatabaseName(database.databaseName);
	this->database.setUserName(database.userName);
	this->database.setPassword(database.password);
	this->database.setPort(database.port);
}

Database::~Database()
{
	if(this->database.isOpen()){
		this->database.close();
	}
	qInfo() << "Database close successfully."; // << this->database.userName << "@" << this->database.hostName << ":" << this->database.port;
	emit this->stdOut(tr("数据库连接关闭."));
	emit this->disconnected();
}

bool Database::open(){
	if(!this->database.open()){
		QString errorMsg = this->database.lastError().text();
		qCritical() << "Database open failed:" << database.userName() << "@" << database.hostName() <<":"<< database.port() << " " << errorMsg;
		emit this->stdOut(tr("数据库开启失败 %1@%2:%3. 数据库信息:%4").arg(database.userName()).arg(database.hostName()).arg(database.port()).arg(errorMsg));
		return false;
	}
	qInfo() << "Database open successfully:" << database.userName() << "@" << database.hostName() <<":"<< database.port();
	emit this->stdOut(tr("数据库连接成功 %1@%2:%3.").arg(database.userName()).arg(database.hostName()).arg(database.port()));
	emit this->connected();
	return true;
}

bool Database::close()
{
	if(this->database.isOpen()){
		this->database.close();
		if(this->database.isOpen()){
			qCritical() << "Database close failed.";
			return false;
		}
		return true;
	}
	qWarning() << "Database is already closed.";
	return true;
}

bool Database::isRunning()
{
	return this->database.isOpen();
}

QList<QuestionBrief> Database::getQuestionList()
{
	QString sql = "SELECT `id`, `title`, `pass_num`, `submit_num` FROM `question` WHERE `deleted` = '0'";
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database query failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库查询失败, 数据库信息: %1").arg(q.lastError().text()));
		return QList<QuestionBrief>();
	}
	bool ok = true;
	QList<QuestionBrief> qlist;
	while(q.next()){
		QuestionBrief qb;
		qb.questionId = q.value("id").toInt(&ok);
		qb.title = q.value("title").toString();
		qb.passNum = q.value("pass_num").toInt(&ok);
		qb.submitNum = q.value("submit_num").toInt(&ok);
		if(!ok){
			qDebug() << "Attach database reading toInt failed. Maybe this value is not a int?";
		}
		qlist.append(qb);
	}
	return qlist;
}

Question Database::getQuestionContent(int questionId)
{
	QString sql = "SELECT `id`, `title`, `description`, `input`, `output`, `input_sample`, `output_sample`, "
				  "`hint`, `source`, `time_limit`, `memory_limit`, `pass_num`, `submit_num`, `image` FROM `question` WHERE `id` = '%1'";
	sql = sql.arg(questionId);
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database query failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库查询失败, 数据库信息: %1").arg(q.lastError().text()));
		return Question();
	}
	bool ok = true;
	bool ready = false;
	Question question;
	question.questionId = -1; //如果数据库中没有这个题目的话, 用Id = -1来表示没有这个题目, 而0表示查询失败.
	while(q.next()){
		if(ready == true){
			qWarning() << "An id has >2 records? abandond second record!";
			return Question();
		}
		question.questionId = q.value("id").toInt(&ok);
		question.title = q.value("title").toString();
		question.description = q.value("description").toString();
		question.input = q.value("input").toString();
		question.output = q.value("output").toString();
		question.inputSample = q.value("input_sample").toString();
		question.outputSample = q.value("output_sample").toString();
		question.hint = q.value("hint").toString();
		question.source = q.value("source").toString();
		question.timeLimit = q.value("time_limit").toInt(&ok);
		question.memoryLimit = q.value("memory_limit").toInt(&ok);
		question.passNum = q.value("pass_num").toInt(&ok);
		question.submitNum = q.value("submit_num").toInt(&ok);
		question.image = q.value("image").toString();
		ready = true;
		if(!ok){
			qDebug() << "Attach database reading toInt failed. Maybe this value is not a int?";
		}
	}
	return question;
}

User Database::getUserInformation(QString username, QString token)
{
	QString sql = "SELECT `id`, `name`, `group_id`, `email`, `token` FROM `user` WHERE `name` = %1";
	sql = sql.arg(username);
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database query failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库查询失败, 数据库信息: %1").arg(q.lastError().text()));
		return User();
	}
	bool ok = true;
	bool ready = false;
	User user;
	while(q.next()){
		if(ready == true){
			qWarning() << "Userneme get again, abandoned second record!";
			break;
		}
		user.userId = q.value("id").toInt(&ok);
		user.userGroup = (UserGroup)q.value("group_id").toInt(&ok);
		user.email = q.value("email").toString();
		user.token = q.value("token").toString();
		user.username = q.value("name").toString();
		ready = true;
		if(!ok){
			qDebug() << "Attach database reading toInt failed. Maybe this value is not a int?";
		}
	}
	if(user.token != token){
		qWarning() << "User token issued. token:" << user.token << "receiveToken:" << token;
	}
	return user;
}

User Database::getUserInformation(QString username)
{
	QString sql = "SELECT `id`, `name`, `group_id`, `email`, `token` FROM `user` WHERE `name` = '%1'";
	sql = sql.arg(username);
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database query failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库查询失败, 数据库信息: %1").arg(q.lastError().text()));
		return User();
	}
	bool ok = true;
	bool ready = false;
	User user;
	user.userId = -1; //如果数据库没有的话, 用userId = -1来标记没有这个人.
	while(q.next()){
		if(ready == true){
			qWarning() << "Userneme get again, abandoned second record!";
			break;
		}
		user.userId = q.value("id").toInt(&ok);
		user.userGroup = (UserGroup)q.value("group_id").toInt(&ok);
		user.email = q.value("email").toString();
		user.token = q.value("token").toString();
		user.username = q.value("name").toString();
		ready = true;
		if(!ok){
			qDebug() << "Attach database reading toInt failed. Maybe this value is not a int?";
		}
	}
	return user;
}

QList<JudgeStream> Database::getQuestionJudgeData(int questionId)
{
	QString sql = "SELECT `input`, `output` FROM `stream` WHERE `question_id` = '%1' AND `deleted` = 0";
	sql = sql.arg(questionId);
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database query failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库查询失败, 数据库信息: %1").arg(q.lastError().text()));
		return QList<JudgeStream> ();
	}
	QList<JudgeStream> judgeStreamList;
	while(q.next()){
		JudgeStream js;
		js.questionId = questionId;
		js.input = q.value("input").toString();
		js.output = q.value("output").toString();
		judgeStreamList << js;
	}
	return judgeStreamList;
}

QString Database::getJudgeStatusString(Status status)
{
	QString sql = "SELECT `name` FROM `status` WHERE `id` = %1";
	sql = sql.arg((int)status);
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database query failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库查询失败, 数据库信息: %1").arg(q.lastError().text()));
		return QString();
	}
	while(q.next()){
		return q.value("name").toString();
	}
	return QString();
}

void Database::setJudgeResult(JudgeResult result, QString source)
{
	//QString iSource = source.replace("\"", "\\\"");
	QString sql = "INSERT INTO `judge` (`question_id`, `user_id`, `status_id`, `language_id`, `source`, `time_used`, `memory_used`, `message`, `create_time`) "
				  "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9')";
	//QString messageEscape = QString(result.message.toUtf8().toPercentEncoding());
	QString messageEscape = QString(result.message.toUtf8().toBase64());
	QString sourceEscape = QString(source.toUtf8().toBase64());
	sql = sql.arg(result.questionId)
			.arg(getUserInformation(result.username).userId)
			.arg(result.status)
			.arg(result.language)
			.arg(sourceEscape)
			.arg(result.timeUsed)
			.arg(result.memoryUsed)
			.arg(messageEscape)
			.arg(QDateTime::fromTime_t(result.judgeTime).toString(Qt::ISODate));
	//emit this->stdOut(tr("准备执行的SQL: %1").arg(sql));
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database insert failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库写入失败, 数据库信息: %1").arg(q.lastError().text()));
		return;
	}
}

void Database::setQuestionPass(int questionId, bool isPassed)
{
	QString sql;
	if(isPassed){
		sql = "UPDATE `question` SET `submit_num` = `submit_num` + 1, `pass_num` = `pass_num` + 1 WHERE `id` = %1";
	}else{
		sql = "UPDATE `question` SET `submit_num` = `submit_num` + 1 WHERE `id` = %1";
	}
	sql = sql.arg(questionId);
	QSqlQuery q(sql);
	if(!q.exec()){
		qWarning() << "Database update failed:" << sql << "message:" << q.lastError().text();
		emit this->stdOut(tr("一次数据库写入失败, 数据库信息: %1").arg(q.lastError().text()));
		return;
	}
}

