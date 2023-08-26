#include "readdict.h"
#include <QStandardPaths>
#include <QDir>
#include <QThread>

readDict::readDict(QObject *parent)
    : QObject{parent}
{

}
void readDict::getDictTable(const QString & dictTable_T){
    dictTable = dictTable_T;
}

void readDict::getIndexHash(const QHash<QString,QString> & indexHash_T){
    indexHash = indexHash_T;
}

void readDict::Thread_deal(const QString & dictTable_T,const QHash<QString,QString> & indexHash_T, const QString & curPath_T){

    Q_UNUSED(curPath_T);


    getDictTable(dictTable_T);
    getIndexHash(indexHash_T);

    QString location = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString toDir = location +"/新生成码表目录/";
    QDir dir(toDir);

    if (!dir.exists(toDir)){
        dir.mkpath(toDir);
    }

    QString newDictText = toDir + "/新生成_单字词库.txt";
    QFile my_file1cc(newDictText);
    my_file1cc.setPermissions(QFile::ReadOther | QFile::WriteOther);


    if(my_file1cc.exists()){
        my_file1cc.remove();

        if(my_file1cc.exists()){
            emit deleteFails();
            return;
        }
    }


    QFile aFile(dictTable);

    if(!aFile.exists()){
        return;
    }

    if(!aFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        return;
    }


    QTextStream aStream(&aFile);

    aStream.setAutoDetectUnicode(true);

    QString str;
    QString strChar;
    QStringList strList;



    QFile bFile(newDictText);
    if(!bFile.open(QIODevice::WriteOnly|QIODevice::Text)){
        return;}
    QTextStream bStream(&bFile);
    bStream.setAutoDetectUnicode(true);

    emit doingNow();
    strLostLists.clear();
    qDebug() << "readDict 在干活了！，该子线程 ID 是: " << QThread::currentThreadId() << '\n';


    while(!aStream.atEnd()){
        str = aStream.readLine();
        if(!str.contains("\t")){
            emit erroForm();
            return;
        }
        strList = str.split("\t");
        str = strList.at(0).trimmed();//汉字在此
        strChar = strList.at(1).trimmed();//编码在此
        QString newCode = "";
        int num = strChar.length(); //查出编码宽度

        if(num==0){
            continue;
        }

        QHash<QString, QString>::iterator it1 = indexHash.find(str);
        if(it1!=indexHash.end()){
            newCode = newCode + it1.value();
            strChar = newCode.first(num);
        }else{
            strLostLists.append(str);
            continue;
        }
        bStream << str + '\t' + strChar << '\n';
    }

    aFile.close();
    bFile.close();

    QFile my_file1s(newDictText);
    my_file1s.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    if(strLostLists.isEmpty()){
        emit sign_Stop();
    }else{
        emit failed(strLostLists);
    }

    return;
}
