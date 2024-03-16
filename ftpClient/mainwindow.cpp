#include <ftplib.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QTime>
#include <QTimerEvent>
#include <QMessageBox>
#include <QCloseEvent>
#include <ftpconnectdlg.h>
#include <QFileDialog>
#include <QDebug>

void MainWindow::init_menu()
{
    QMenu* menu = new QMenu(ui->list_file);
    auto cdupAction = menu->addAction(u8"返回上一级目录");
    auto refAction = menu->addAction(u8"刷新");
    auto putAction = menu->addAction(u8"上传");
    auto getAction = menu->addAction(u8"下载");
    auto delAction = menu->addAction(u8"删除");
    ui->list_file->setContextMenuPolicy(Qt::CustomContextMenu);// 设置自定义的右键菜单策略
    connect(ui->list_file,&QTableWidget::customContextMenuRequested,[=](const QPoint& pos){
        qDebug() << ui->list_file->currentRow();
        menu->exec(ui->list_file->viewport()->mapToGlobal(pos));
    });
    connect(cdupAction, &QAction::triggered, this,&MainWindow::cdupAction);
    connect(refAction, &QAction::triggered, this,&MainWindow::refAction);
    connect(putAction, &QAction::triggered, this,&MainWindow::putAction);
    connect(getAction, &QAction::triggered, this,&MainWindow::getAction);
    connect(delAction, &QAction::triggered, this,&MainWindow::delAction);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,ui_connect_status(u8"就绪")// 初始化状态栏的连接状态为"就绪"
    ,ui_pwd(u8"当前路径：/")// 初始化状态栏的当前路径
    ,ui_cur_time(cur_time())// 初始化状态栏的当前时间
{
    ui->setupUi(this); // 加载UI布局
    // 在状态栏添加上述初始化的三个部件
    ui->statusbar->addWidget(&ui_connect_status);
    ui->statusbar->addWidget(&ui_pwd);
    ui->statusbar->addPermanentWidget(&ui_cur_time);
    // 连接退出动作的触发信号到槽函数，实现退出提示
    connect(ui->action_exit,&QAction::triggered,this,[=](){
        qDebug() << "action_exit";
        if(QMessageBox::question(this,this->windowTitle(),u8"你确定要退出吗？") != QMessageBox::Yes) return;
        exit(0);
    });
    // 连接退出动作的触发信号到槽函数，实现退出提示
    // QAction是设计ui时的类 代表一个连接操作 用户通过点击按钮操作
    connect(ui->action_connect,&QAction::triggered,this,&MainWindow::onConnect);
    timer1 = startTimer(10);

    connect(ui->actiondisconnect,&QAction::triggered,this, &MainWindow::disConnect);

    // 初始化文件列表表格的列头
    ui->list_file->setColumnCount(7);
    ui->list_file->setHorizontalHeaderLabels({u8"权限",u8"链接数",u8"属主",u8"属组",u8"大小",u8"日期",u8"文件名"});

    connect(ui->list_file,&QTableWidget::cellDoubleClicked,this,&MainWindow::cellDoubleClicked); // 连接双击表单的事件

    init_menu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    if(e->timerId() != timer1)return;
    ui_cur_time.setText(cur_time());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    qDebug() << "closeEvent";
    if(QMessageBox::question(this,this->windowTitle(),u8"你确定要退出吗？") == QMessageBox::Yes)return;
    e->ignore();
}

void MainWindow::onConnect()
{
    FtpConnectDlg dlg;
    if(dlg.exec() != QDialog::Accepted) return; // 显示对话框
    if(!ftp.login(dlg.ftp_data())) return; // 给ftp_data的成员赋值 并且连接
    ui_connect_status.setText(u8"连接成功：" + dlg.ftp_data().host);

    clear_ui_list(); // 重置表单
    set_pwd();
    insert_list(ftp.dir());
}


void MainWindow::disConnect()
{
    if(ftp.logout()) // 如果已经断开连接
    {
        qDebug() << "Successfully disconnected";
        clear_ui_list(); // 清空文件列表
        ui_connect_status.setText(u8"已断开连接"); // 更新状态栏显示
        QMessageBox::information(this, tr("FTP"), tr("Disconnected from the server.")); // 通知用户
    }
    else
    {
        // 断开连接失败时的处理
        qDebug() << "Failed to disconnect.";
        QMessageBox::warning(this, tr("FTP"), tr("Failed to disconnect from the server."));
    }
}

/*处理双击 QTableWidget 控件中某一行时的事件。具体来说，它实现了在 FTP 客户端应用程序切换目录的功能*/
void MainWindow::cellDoubleClicked(int row)
{
    if(row == -1)return;
    QString file_name = ui->list_file->item(row,6)->text();
    if(ui->list_file->item(row,0)->text()[0] != 'd')file_name = file_name.split(" ")[0];
    if(!ftp.cd(file_name)){
        QMessageBox::warning(this,this->windowTitle(),QString(u8"%1切换失败，%1可能不是目录").arg(file_name));
        return;
    }
    set_pwd();
    auto list = ftp.dir();
    if(list.empty()){
        QMessageBox::warning(this,this->windowTitle(),QString(u8"%1切换失败，%1可能不是目录").arg(file_name));
        return;
    }
    clear_ui_list();
    insert_list(list);
}

void MainWindow::cdupAction()
{
    if(!ftp.cdup()) return;
    set_pwd();
    clear_ui_list();
    insert_list(ftp.dir());
}

void MainWindow::refAction()
{
    clear_ui_list();
    insert_list(ftp.dir());
}

void MainWindow::putAction()
{
    QFileDialog file(this,u8"上传文件");
    file.setFileMode(QFileDialog::FileMode::ExistingFile);
    if(file.exec() != QDialog::Accepted)return;
    auto files = file.selectedFiles();
    if(files.size() > 1) {
        QMessageBox::warning(this,u8"选择文件过多！",u8"最多选择一个文件");
        return;
    }
    qDebug() << files[0];
    if(!ftp.put(files[0])){
        QMessageBox::warning(this,this->windowTitle(),"上传失败\n错误：" + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),u8"上传成功！");
    clear_ui_list();
    insert_list(ftp.dir());
}

void MainWindow::getAction()
{
    int row = ui->list_file->currentRow();
    if(row == -1){
        QMessageBox::warning(this,this->windowTitle(),u8"没有选择下载文件");
        return;
    }
    QFileDialog file(this,u8"下载文件到");
    file.setFileMode(QFileDialog::FileMode::AnyFile);
    file.selectFile(ui->list_file->item(row,6)->text());
    if(file.exec() != QDialog::Accepted)return;
    auto files = file.selectedFiles();
    if(files.size() > 1) {
        QMessageBox::warning(this,u8"选择文件过多！",u8"最多选择一个文件");
        return;
    }
    qDebug() << files[0];
    if(!ftp.get(files[0],ui->list_file->item(row,6)->text())){
        QMessageBox::warning(this,this->windowTitle(),u8"下载失败\n错误：" + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),u8"下载成功！");
}

void MainWindow::delAction()
{
    int row = ui->list_file->currentRow();
    if(row == -1){
        QMessageBox::warning(this,this->windowTitle(),u8"没有选择删除文件");
        return;
    }
    if(!ftp.del(get_file_name(row)))
    {
        QMessageBox::warning(this,this->windowTitle(),u8"删除失败\n错误：" + ftp.error());
        return;
    }
    QMessageBox::information(this,this->windowTitle(),u8"删除成功！");
    clear_ui_list();
    insert_list(ftp.dir());
}

QString MainWindow::cur_time()
{
    return QTime::currentTime().toString();
}

QString MainWindow::get_file_name(int row)
{
    auto file_name = ui->list_file->item(row,6)->text();
    if(file_name.contains(" ") && ui->list_file->item(row,0)->text()[0] == 'l')
        file_name = file_name.split(" ")[0];
    return file_name;
}

void MainWindow::set_pwd()
{
    ui_pwd.setText(u8"当前路径：" + ftp.pwd());
}

void MainWindow::clear_ui_list()
{
    while(ui->list_file->rowCount() > 0)ui->list_file->removeRow(0);
}

void MainWindow::insert_list(const std::vector<FTP_FILE_INFO> &list)
{
    for(auto& info : list)
    {
        insert_row(ui->list_file->rowCount(),info);
    }
}

void MainWindow::insert_row(int row, const FTP_FILE_INFO &info)
{
    ui->list_file->insertRow(row);
    insert_item(row,0,info.access);
    insert_item(row,1,info.link_cnt);
    insert_item(row,2,info.ower);
    insert_item(row,3,info.group);
    insert_item(row,4,info.size);
    insert_item(row,5,info.date);
    insert_item(row,6,info.file_name);
}

void MainWindow::insert_item(int row,int idx,QString item)
{
    ui->list_file->setItem(row,idx,new QTableWidgetItem(item));
    ui->list_file->item(row,idx)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
}

