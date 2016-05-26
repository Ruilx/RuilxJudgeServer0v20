#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>

//控制端发送的请求类型码
enum CmdId{
	NoneCmdId = 0,
	GetQuestionList = 1,
	GetQuestionContent = 2,
	GetJudged = 3,
	GetUserInformation = 4,

	Message = -1,
};

//程序判定返回码
enum Status{
	NoneStatus = 0,
	Accepted = 1,
	PresentationError = 2,
	WrongAnswer = 3,
	OutputLimitExcedded = 4,
	ValidatorError = 5,
	MemoryLimitExcedded = 6,
	TimeLimitExcedded = 7,
	RuntimeError = 8,
	CompileError = 9,
	SystemError = 10,
	RestrictedFunction = 11,
	Running = 12,
	Hidden = 13,
};

//用户类型ID
enum UserGroup{
	UnknownGroup = 0,
	Judger = 1,
	Manager = 2,
	Admin = 3,
};

//程序源码语言ID
enum Language{
	None = 0,
	C = 1,
	Cpp = 2,
	Java = 3,
	Python = 4,
	Php = 5,
	Lua = 6,
};

//问题列表Node
typedef struct{
	int questionId;
	QString title;
	int passNum;
	int submitNum;
}QuestionBrief;

//问题
typedef struct{
	int questionId;
	QString title;
	QString description;
	QString input;
	QString output;
	QString inputSample;
	QString outputSample;
	QString hint;
	QString source;
	int timeLimit;
	int memoryLimit;
	int passNum;
	int submitNum;
	QString image;
}Question;

//判题
typedef struct{
	QString username;
	QString token;
	QString programSource;
	Language language;
	int questionId;
	int judgeId;
}Judge;

//判题结果
typedef struct{
	QString username;
	int judgeId;
	int questionId;
	Status status;
	int judgeTime;
	Language language;
	int timeUsed;
	int memoryUsed;
	QString message;
}JudgeResult;

//RunCode子系统发来的运行结果
typedef struct{
	QString name;
	int status;
	QString statusStr;
	int exitCode;
	int timeUsed;
	int memoryUsed;
	QString outputString;
}RunCodeJudgeResult;

//RunCode配置
typedef struct{
	qint64 tid;
	bool useDocker;
	QString execfile;
	QString inputfile;
	QString host;
	int port;
	int timeLimit;
	int memoryLimit;
	int outputLimit;
	QString runCodeFilePath;
	QString dockerPath;
	QString image;
}RunCodeSettings;

//用户信息
typedef struct{
	int userId;
	QString username;
	UserGroup userGroup;
	QString email;
	QString token;
}User;

//数据库连接
const QString DatabaseDriver = "QMYSQL";
typedef struct{
	QString hostName;
	QString databaseName;
	QString userName;
	QString password;
	int port;
}DatabaseOption;

//判题的输入输出数据
typedef struct{
	int questionId;
	QString input;
	QString output;
}JudgeStream;



#endif // GLOBAL_H

