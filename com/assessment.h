#ifndef ASSESSMENT_H
#define ASSESSMENT_H

#include <QObject>
#include <QtCore>
#include "global.h"

class Assessment : public QObject
{
	Q_OBJECT
	QString std;
	QString out;
public:
	explicit Assessment(const QString &std, const QString &out, QObject *parent = 0);
	Status getResult();
signals:

public slots:
};

#endif // ASSESSMENT_H

/*
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//一些定义
const int ACCEPT = 1;
const int PRESENTATION_ERROR= 2;
const int WRONG_ANSWER = 3;
//fstd 标准输出 fout 选手输出 fin 标准输入
FILE *fstd,*fout,*fin;

int LastCharStd = -2,LastCharOut=-2;

//检查下一个字符
inline int Peek(FILE* f){
	if(f==fstd){
		if(LastCharStd == -2)
			LastCharStd=fgetc(f);
		return LastCharStd;
	}else{
		if(LastCharOut == -2)
			LastCharOut=fgetc(f);
		return LastCharOut;
	}
}

//取出下一个字符
inline void Pop(FILE* f){
	if(f==fstd){
		if(LastCharStd == -2)
			fgetc(f);
		else
			LastCharStd = -2;
	}else{
		if(LastCharOut == -2)
			fgetc(f);
		else
			LastCharOut = -2;
	}
}

//判断字符是否为空白
inline bool IsSpace(int ch){
	return ch>=0 && (ch<=32 || ch>=127);
}

//执行比较操作。
bool DoCompare(){
	int stdPosition=0,outPosition=0;
	bool stdInSpace=true,outInSpace=true;
	while(true){
		int stdC=Peek(fstd),outC=Peek(fout);
		if(stdC==EOF && outC==EOF){
			return true;
		}else if(stdC==EOF && IsSpace(outC)){
			outPosition++;
			Pop(fout);
		}else if(outC==EOF && IsSpace(stdC)){
			stdPosition++;
			Pop(fstd);
		}else if(IsSpace(stdC) && IsSpace(outC)){
			stdPosition++;
			outPosition++;
			stdInSpace=true;
			outInSpace=true;
			Pop(fstd);
			Pop(fout);
		}else if(IsSpace(stdC) && outInSpace){
			stdPosition++;
			Pop(fstd);
		}else if(IsSpace(outC) && stdInSpace){
			outPosition++;
			Pop(fout);
		}else if(stdC==outC){
			stdPosition++;
			outPosition++;
			stdInSpace=false;
			outInSpace=false;
			Pop(fstd);
			Pop(fout);
		}else{
			printf("答案文件的第%d字节",stdPosition+1);
			if(stdC==EOF){
				printf("<EOF>");
			}else{
				printf("0x%x",stdC);
			}
			printf("不能匹配输出文件的第%d字节",outPosition+1);
			if(outC==EOF){
				printf("%lt;EOF%gt;");
			}else{
				printf("0x%x",outC);
			}
			puts("");
			return false;
		}
	}
}

int main(int argc, char* argv[])
{
	if(argc!=4){
		printf("参数不足 %d",argc);
		return -1;
	}

	//打开文件
	if(NULL==(fstd=fopen(argv[1],"r"))){
		return -1;
	}
	if(NULL==(fout=fopen(argv[2],"r"))){
		return -1;
	}
	if(NULL==(fin=fopen(argv[3],"r"))){
		return -1;
	}

	if(DoCompare()){
		return ACCEPT;
	}else{
		return WRONG_ANSWER;
	}
}
*/
