#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <readspellingtable.h>
#include <QThread>
#include <readdict.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_5_clicked();
    void onSign_Stop_Spelling();
    void onSign_Stop_Dict();
    void getIndex(const QHash<QString,QString> & indexHash_T);

    void on_btnOpenSpellingTable_clicked();
    void on_btnOpenDict_clicked();

    void onDoingNow();
    void onFailed_Spelling(const QStringList  & strLoseList);
    void onErroForm_Spelling();
    void onDeleteFails_Spelling();

    void onFailed_Dict(const QStringList  & strLoseList);
    void onErroForm_Dict();
    void onDeleteFails_Dict();

signals:
    void start_thread_Spelling(const QString & spellingTable_T,const QHash<QString,QString> & charHash_T ,const QString & curPath_T);
    void start_thread_Dict(const QString & dictTable_T,const QHash<QString,QString> & charHash_T ,const QString & curPath_T);

private:
    Ui::MainWindow *ui;
    QString curPath; //spelling_table 所在目录
    QString spellingTable; //spelling_table 完整路径
    QHash<QString,QString> charHash; //码元映射表
    QHash<QString,QString> indexHash; //单字全码表
    bool MakeHashTable(); //生成码元映射表
    void mkHash();//初始化第一步，调用 MakeHashTable 生成码元映射表

    ReadSpellingTable * readSpelling = new ReadSpellingTable;
    readDict * readDictTable = new readDict;
    QThread * thread_Dict = new QThread(this);
    QThread * thread_Spelling = new QThread(this);


    QString dictTable; //单字码表文件
    void setSkin();
    void openDir();

};
#endif // MAINWINDOW_H
