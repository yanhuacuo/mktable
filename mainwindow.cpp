#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QFontDatabase>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("码表修正工具");
    mkHash(); //生成码元映射表

    connect(this,SIGNAL(start_thread_Dict(const QString &,const QHash<QString,QString> &,const QString &)),readDictTable,SLOT(Thread_deal(const QString &,const QHash<QString,QString> &,const QString &)),Qt::UniqueConnection);
    connect(this,SIGNAL(start_thread_Spelling(const QString &,const QHash<QString,QString> &,const QString &)),readSpelling,SLOT(Thread_deal(const QString &,const QHash<QString,QString> &,const QString &)),Qt::UniqueConnection);

    connect(readSpelling,&ReadSpellingTable::sign_Stop,this,&MainWindow::onSign_Stop_Spelling);
    connect(readSpelling,&ReadSpellingTable::mkIndex,this,&MainWindow::getIndex);
    connect(readSpelling,&ReadSpellingTable::doingNow,this,&MainWindow::onDoingNow);

    connect(readDictTable,&readDict::sign_Stop,this,&MainWindow::onSign_Stop_Dict);    
    connect(readDictTable,&readDict::doingNow,this,&MainWindow::onDoingNow);
    //失败信号

    connect(readSpelling,&ReadSpellingTable::deleteFails,this,&MainWindow::onDeleteFails_Spelling);
    connect(readSpelling,&ReadSpellingTable::erroForm,this,&MainWindow::onErroForm_Spelling);
    connect(readSpelling,&ReadSpellingTable::failed,this,&MainWindow::onFailed_Spelling);

    connect(readDictTable,&readDict::erroForm,this,&MainWindow::onErroForm_Dict);
    connect(readDictTable,&readDict::deleteFails,this,&MainWindow::onDeleteFails_Dict);
    connect(readDictTable,&readDict::failed,this,&MainWindow::onFailed_Dict);

    setSkin();
    setWindowIcon(QIcon(":/icon/icon.ico"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::MakeHashTable(){

    curPath= QCoreApplication::applicationDirPath();

    QString charData_Path = curPath + "//索引构建目录";

    QDir dir(charData_Path);

    if (!dir.exists(charData_Path)){
        dir.mkpath(charData_Path);
    }

    QString aFileName(charData_Path + "/charData.txt");
    QFile aFile(aFileName);

    QFile my_file2(charData_Path + "/Wubi98_Single_Code.txt");
    QFile my_file3(charData_Path + "/wubi98-dz.txt");

    if(!aFile.exists() || !my_file2.exists() || my_file3.exists()){
        QFile my_file11(":/table/charData.txt");
        bool ok1 = my_file11.copy(charData_Path + "/charData.txt");
        QFile my_file22(":/table/Wubi98_Single_Code.txt");
        bool ok2 = my_file22.copy(charData_Path + "/Wubi98_Single_Code.txt");
        QFile my_file33(":/table/wubi98-dz.txt");
        bool ok3 = my_file33.copy(charData_Path + "/wubi98-dz.txt");

        QFile my_file1s(charData_Path + "/charData.txt");
        QFile my_file2s(charData_Path + "/Wubi98_Single_Code.txt");
        QFile my_file3s(charData_Path + "/wubi98-dz.txt");

        my_file1s.setPermissions(QFile::ReadOther | QFile::WriteOther);
        my_file2s.setPermissions(QFile::ReadOther | QFile::WriteOther);
        my_file3s.setPermissions(QFile::ReadOther | QFile::WriteOther);

        if(ok1 && ok2 && ok3){
            QMessageBox::information(this, "缺少码元单义表", "已经从程序中拿出备份补上！",
                                     QMessageBox::Ok,QMessageBox::NoButton);
        }

    }

    if(!aFile.exists()){return false;}
    if(!aFile.open(QIODevice::ReadOnly|QIODevice::Text)){return false;}

    QTextStream aStream(&aFile);
    aStream.setAutoDetectUnicode(true);
    charHash.clear();
    QString str;
    QStringList strList;

    while(!aStream.atEnd()){
        str = aStream.readLine().trimmed();
        strList = str.split('\t');
        charHash.insert(strList.at(0).trimmed(),strList.at(1).trimmed());
    }
    aFile.close();

    ui->label_2->setOpenExternalLinks(true);
    ui->label_2->setText("<a style='color: gray;' href=\"https://wubi98.gitee.io\">五笔小筑");

    QStringList strAboutList = {"1. 依据【码元映射表】自动【修正拆分表中的编码】。",
                            "2. 依据修正过的【拆分表】自动生成【单字全码表】",
                            "3. 接受导入【单行单义】的【单字表】，并用索引数据纠正编码",
                                };

    ui->textBrowser->setText(strAboutList.at(0)+'\n'+strAboutList.at(1)+'\n'+strAboutList.at(2)+'\n');

    return true;
}

void MainWindow::mkHash(){

    bool ok = MakeHashTable();
    if (ok){
        ui->label_State->setText("码元映射表已成功生成！");
    }

}

void MainWindow::on_pushButton_5_clicked()
{
    ui->textBrowser->clear();
    QHash<QString, QString>::const_iterator i = charHash.cbegin();
    while (i != charHash.cend()) {
        ui->textBrowser->append("【"+ i.key()+"】" +" 的编码是: "+ i.value()+'\n');
        ++i;
    }
}
void MainWindow::on_btnOpenDict_clicked()
{
    bool ok = ui->checkBox->isChecked();

    if(!ok){
        QMessageBox::information(this, " 尚未生成单字码表索引", "请先读取一个【拆分表】生成单字索引 ",
                                     QMessageBox::Ok,QMessageBox::NoButton);

        return;
    }

    curPath= QCoreApplication::applicationDirPath();
    QString dlgTitle="打开一个单字码表";
    QString filter = "单字码表(*.txt)";
    dictTable=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if(spellingTable.isEmpty()){
        return;
    }
    ui->lineEdit_2->setText(dictTable);

    readDictTable->moveToThread(thread_Dict);

    if(thread_Dict->isRunning()){
        return;
    }
    thread_Dict->start();
    emit start_thread_Dict(dictTable,indexHash,curPath);

}
void MainWindow::on_btnOpenSpellingTable_clicked()
{
    curPath= QCoreApplication::applicationDirPath();
    QString dlgTitle="打一个 spelling 文件";
    QString filter = "spelling文件(*.txt)";
    spellingTable=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if(spellingTable.isEmpty()){
        return;
    }
    ui->lineEdit->setText(spellingTable);
    readSpelling->moveToThread(thread_Spelling);


    if(thread_Spelling->isRunning()){
        return;
    }
    thread_Spelling->start();
    emit start_thread_Spelling(spellingTable,charHash,curPath);
}

void MainWindow::getIndex(const QHash<QString,QString> & indexHash_T){
    indexHash.clear();
    indexHash = indexHash_T;
    ui->checkBox->setChecked(true);
}

void MainWindow::onSign_Stop_Spelling(){

    if(thread_Spelling->isRunning()){
        thread_Spelling->quit();
    }

    ui->textBrowser->clear();
    ui->textBrowser->setText("【新生成码表目录】在桌面上，\n程序已构建全单字编码索引。");

    QMessageBox::information(this, "码表处理完毕", "【新生成码表目录】在桌面上！",
                                     QMessageBox::Ok,QMessageBox::NoButton);
    openDir();
}

void MainWindow::onSign_Stop_Dict(){

    if(thread_Dict->isRunning()){
        thread_Dict->quit();
    }

    ui->textBrowser->clear();
    ui->textBrowser->setText("【新生成码表目录】在桌面上，\n单字码词库已处理完毕。");

    QMessageBox::information(this, "码表处理完毕", "【新生成码表目录】在桌面上！",
                                     QMessageBox::Ok,QMessageBox::NoButton);
    openDir();
    readDictTable->disconnect();
}



void MainWindow::onDoingNow(){

    qDebug() << "主线程收到了子线程【信号】，主线程 ID 是: " << QThread::currentThreadId() << '\n';

}


void MainWindow::onFailed_Dict(const QStringList  & strLoseList){



    if(thread_Dict->isRunning()){
        thread_Dict->quit();
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText("程序内建的【数据索引】不全，缺失下列项，请修正后，重新生成码表！\n");
    for(auto it = strLoseList.begin(); it!= strLoseList.end(); ++it){
        QString str = *it;
        ui->textBrowser->append(" ・ 【" + str + "】");
    }

}

void MainWindow::onErroForm_Dict(){

    if(thread_Dict->isRunning()){
        thread_Dict->quit();
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText("目标文件（即单行单义格式单字表）:\n"+ ui->lineEdit_2->text() +"\n格式不正确，\n请参照范例文件修正格式。");

}


void MainWindow::onDeleteFails_Dict(){


    if(thread_Dict->isRunning()){
        thread_Dict->quit();
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText("桌面上【新生成码表目录】中的：\n【新生成_单字词库.txt】被占用，\n请手动删除后再尝试。");

}

void MainWindow::onFailed_Spelling(const QStringList  & strLoseList){


    if(thread_Spelling->isRunning()){
        thread_Spelling->quit();
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText("查到【数据索引】不全，缺失下列项，请修正，并重新生成码表！\n");
    for(auto it = strLoseList.begin(); it!= strLoseList.end(); ++it){
        QString str = *it;
        ui->textBrowser->append(" ・ 【" + str + "】");
    }

}

void MainWindow::onErroForm_Spelling(){

    if(thread_Spelling->isRunning()){
        thread_Spelling->quit();
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText("目标文件(即拆分文件):\n" + ui->lineEdit->text() + "\n格式不正确，\n请参照范例文件修正格式。");

}


void MainWindow::onDeleteFails_Spelling(){


    if(thread_Spelling->isRunning()){
        thread_Spelling->quit();
    }
    ui->textBrowser->clear();
    ui->textBrowser->setText("桌面上【新生成码表目录】中的:\n【新生成_拆分表.txt】和【新生成_单字全码表.txt】被占用，\n请手动删除后再尝试。");

}

void MainWindow::setSkin()
{
    int font_Id = QFontDatabase::addApplicationFont(":/table/98WB-1.otf");
    QStringList font_list = QFontDatabase::applicationFontFamilies(font_Id);
    qDebug()<<"自定义字体序值：" << font_Id << '\n';
    qDebug()<<"打印字体列表：" << font_list << '\n';
    qDebug() << "主线程 ID 是: " << QThread::currentThreadId();

    if(!font_list.isEmpty())
    {
        QFont myFont;
        myFont.setFamily(font_list[0]);
        qApp->setFont(myFont);
    }

};


void MainWindow::openDir(){
    QProcess process;
    QString location = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString toDir = location +"/新生成码表目录/";
    QString cmd = QString("explorer.exe /select,\"%1\"").arg(toDir);
    process.startDetached(cmd);
}
