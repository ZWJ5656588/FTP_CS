#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ftp.h"

#include <QLabel>  // 包含 QLabel 类头文件，用于在状态栏显示文本信息
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT // 启用信号槽

private:
    void init_menu();// 初始化右键菜单
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    void timerEvent(QTimerEvent *e);
    void closeEvent(QCloseEvent* e);
public slots:
    // 这些槽函数用于响应用户操作和事件，例如连接到 FTP 服务器、在文件列表中双击文件或目录、执行文件操作（上传、下载、删除等）。
    void onConnect();
    void disConnect();
    void cellDoubleClicked(int row);
    void cdupAction();
    void refAction();
    void putAction();
    void getAction();
    void delAction();

public:
    static QString cur_time();
private:
    QString get_file_name(int row);
    void set_pwd();
    void clear_ui_list();
    void insert_list(const std::vector<FTP_FILE_INFO>& list);
    void insert_row(int row,const FTP_FILE_INFO& info);
    void insert_item(int row,int idx,QString item);
private:
    Ui::MainWindow *ui;// 指向由 uic 生成的 UI 类的实例
    QLabel ui_connect_status;// 显示连接状态的标签
    QLabel ui_pwd;// 显示当前路径的标签
    QLabel ui_cur_time;// 显示当前时间的标签
    int timer1;
    Ftp ftp;// FTP操作的实例
};
#endif // MAINWINDOW_H
