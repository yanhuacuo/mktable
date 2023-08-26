#include "readspellingtable.h"
#include <QStandardPaths>
#include <QDir>
#include <QThread>

ReadSpellingTable::ReadSpellingTable(QObject *parent)
    : QObject{parent}
{

}

void ReadSpellingTable::getFilePath(const QString & spellingTable_T){
    spellingTable = spellingTable_T;
}

void ReadSpellingTable::getQhash(const QHash<QString,QString> & charHash_T){
    charHash = charHash_T;
}

void ReadSpellingTable::Thread_deal(const QString & spellingTable_T,const QHash<QString,QString> & charHash_T, const QString & curPath_T){

    getFilePath(spellingTable_T);
    getQhash(charHash_T);

    Q_UNUSED(curPath_T);

    indexHash.clear();
    emit doingNow();
    strLostLists.clear();
    qDebug() << "ReadSpellingTable 在干活了！，该子线程 ID 是: " << QThread::currentThreadId() << '\n';

    QString location = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString toDir = location +"/新生成码表目录/";
    QDir dir(toDir);

    if (!dir.exists(toDir)){
        dir.mkpath(toDir);
    }

    QString singleCharText = toDir + "/新生成_单字全码表.txt";
    QString newSpellingText = toDir + "/新生成_拆分表.txt";

    QFile my_file1cc(singleCharText);
    QFile my_file2cc(newSpellingText);

    my_file1cc.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    my_file2cc.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);


    if (!dir.exists(toDir)){
        dir.mkpath(toDir);
    }



    if(my_file2cc.exists() || my_file1cc.exists()){

        my_file1cc.remove();
        my_file2cc.remove();

        if(my_file2cc.exists() || my_file1cc.exists()){
            emit deleteFails();
            return;
        }
    }


    QFile aFile(spellingTable);
    if(!aFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<< "拆分表文件打不开！"<<'\n';
        return;
    }


    QTextStream aStream(&aFile);

    aStream.setAutoDetectUnicode(true);

    QString str;
    QString strChar;
    QStringList strList;
    QStringList charToList;


    QFile bFile(singleCharText);
    if(!bFile.open(QIODevice::WriteOnly|QIODevice::Text)){
        return;}
    QTextStream bStream(&bFile);
    bStream.setAutoDetectUnicode(true);

    QFile cFile(newSpellingText);
    if(!cFile.open(QIODevice::WriteOnly|QIODevice::Text)){
        return;}
    QTextStream cStream(&cFile);
    cStream.setAutoDetectUnicode(true);


    while(!aStream.atEnd()){
        str = aStream.readLine();

        if(str.trimmed().isEmpty()){
            continue;
        }

        if(!str.contains("※,※")){
            emit erroForm();
            return;
        }

        strList = str.split("\t[");
        str = strList.at(0);
        strList = strList.at(1).split(",");
        strChar = strList.at(0);
        QString newCode = "";
        charToList = strChar.split("※");

        for(QStringList::Iterator it = charToList.begin(); it != charToList.end(); ++it){
            QString str = *it;
            if(str.isEmpty()){
                continue;
            }
            QHash<QString, QString>::iterator it1 = charHash.find(str);
            if(it1!=charHash.end()){
                newCode = newCode + it1.value();
            }else{
                strLostLists.append(str);
                continue;
            }
        }
        strList[1] = "※"+newCode+"※";
        bStream << str + '\t' + newCode << '\n';
        indexHash.insert(str,newCode);
        QString mystrTest = str+"\t["+strList.join(",");
        cStream << mystrTest << '\n';

    }

    aFile.close();
    bFile.close();
    cFile.close();

    QFile my_file1s(singleCharText);
    QFile my_file2s(newSpellingText);


    my_file1s.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    my_file2s.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    if(strLostLists.isEmpty()){

        emit mkIndex(indexHash);
        emit sign_Stop();

        qDebug() << "读拆分表〔成功〕的信号已发送！" << '\n';

    }else{
        emit failed(strLostLists);
        qDebug() << "读拆分表【失败】的信号已发送！" << '\n';
    }

    return;
}
