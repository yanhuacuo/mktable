#ifndef READDICT_H
#define READDICT_H

#include <QObject>
#include<QTextStream>
#include<QFile>
#include<QHash>
#include<QDebug>

class readDict : public QObject
{
    Q_OBJECT
public:
    explicit readDict(QObject *parent = nullptr);
    void getDictTable(const QString & dictTable_T);
    void getIndexHash(const QHash<QString,QString> & indexHash_T);
private:
    QString dictTable; //dictTable
    QHash<QString,QString> indexHash; //单字全码表
    QStringList strLostLists;
signals:
    void sign_Stop();
    void doingNow();
    void failed(const QStringList  & strLoseList);
    void erroForm();
    void deleteFails();
public slots:
    void Thread_deal(const QString & spellingTable_T,const QHash<QString,QString> & charHash_T,const QString & curPath_T);
};

#endif // READDICT_H
