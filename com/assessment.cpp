#include "assessment.h"

Assessment::Assessment(const QString &std, const QString &out, QObject *parent): QObject(parent)
{
	this->std = std;
	this->out = out;
}

Status Assessment::getResult()
{
	Status level = Accepted;
	this->std.remove(QChar('\r'));
	this->std.remove(QChar('\r'));
	QStringList stdLineList = std.split(QChar('\n'));
	QStringList outLineList = out.split(QChar('\n'));
	if(outLineList.length() < stdLineList.length()){
		//如果输出的行数内容不足标准行数内容, 返回WrongAnswer
		return WrongAnswer;
	}
	for(int i = 0; i < stdLineList.length(); i++){
		//对从开始(第一个(行))的内容逐一进行检验
		QString stdPattern = stdLineList.at(i);
		QString outPattern = outLineList.at(i);
		QStringList stdPatternList = stdPattern.split(QChar(' '));
		QStringList outPatternList = outPattern.split(QChar(' '));
		if(outPatternList.length() < stdPatternList.length()){
			//如果中间出现了以\t分隔的答案, 按照内容匹配程度, 分配PE或者WA.
			level = PresentationError;
			outPatternList = outPattern.split(QRegExp("[ \t]"));
		}
		for(int ii = 0; ii < stdPattern.length(); ii++){
			QString stdWord = stdPattern.at(ii);
			QString outWord = outPattern.at(ii);
			if(QString::compare(stdWord, outWord)){
				//如果有一项不匹配, 返回WA.
				return WrongAnswer;
			}
		}
		//处理之后多余的输出
		for(int ii = stdPattern.length(); ii < outPattern.length(); ii++){
			QString outWord = outPattern.at(ii);
			if(!outWord.isEmpty()){
				//如果后面的数据不为空, 即之后不全为空格, 判为WA.
				return WrongAnswer;
			}
		}
	}
	//处理行后多余的输出
	for(int i = stdLineList.length(); i < outLineList.length(); i++){
		QString outPattern = outLineList.at(i);
		if(!outPattern.isEmpty()){
			//如果行后数据不为空, 及之后有带有空格的空行判为WA.
			return WrongAnswer;
		}
	}
	return level;
}
