#ifndef READSPELLINGTABLE_H
#define READSPELLINGTABLE_H

#include <QObject>
#include<QTextStream>
#include<QFile>
#include<QHash>
#include<QDebug>

class ReadSpellingTable : public QObject
{
    Q_OBJECT
public:
    explicit ReadSpellingTable(QObject *parent = nullptr);    
    void getFilePath(const QString & spellingTable_T);
    void getQhash(const QHash<QString,QString> & charHash_T);
private:
    QString spellingTable; //spelling_table
    QHash<QString,QString> charHash; //码元映射表
    QHash<QString,QString> indexHash; //单字全码表
    QStringList strLostLists;
signals:
    void sign_Stop();
    void doingNow();
    void mkIndex(const QHash<QString,QString> & indexHash_T);
    void failed(const QStringList  & strLoseList);
    void erroForm();
    void deleteFails();
public slots:
    void Thread_deal(const QString & spellingTable_T,const QHash<QString,QString> & charHash_T,const QString & curPath_T);
};

#endif // READSPELLINGTABLE_H
