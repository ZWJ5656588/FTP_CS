#ifndef FTP_H
#define FTP_H
#include <QObject> // 根类
#include <ftplib.h>

struct FTP_DATA
{
    QString host;// FTP服务器的主机地址
    QString user;// 登录FTP服务器的用户名
    QString pass;// 登录FTP服务器的密码
};

struct FTP_FILE_INFO
{
    FTP_FILE_INFO(const QString& access, const QString& link_cnt, const QString& ower,
                     const QString& group, const QString& size, const QString& date, const QString& file_name)
           : access(access), link_cnt(link_cnt), ower(ower), group(group),
             size(size), date(date), file_name(file_name) {}
    QString access;//权限
    QString link_cnt;//链接数
    QString ower;//属主
    QString group;//属组
    QString size;//大小
    QString date;//日期(3)
    QString file_name;//文件名(last)
//    uintptr_t size;
//    bool is_dir;
};

class Ftp
{
public:
    Ftp();
    ~Ftp();
public:
    bool login(const FTP_DATA& data);
    bool logout(); // 断开连接
    QString pwd();
    std::vector<FTP_FILE_INFO> dir();
    bool cd(const QString& path);
    bool cdup();
    bool put(const QString& put_file);
    bool get(const QString& get_file,const QString& remote_file);
    bool del(const QString& file);
    QString error();
private:

private:
    ftplib* ftp; // 包含一个ftplib对象 这个ftplib是干活的
    std::string cur_path;
};

#endif // FTP_H
