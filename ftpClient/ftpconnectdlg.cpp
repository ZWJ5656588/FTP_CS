#include "ftpconnectdlg.h"
#include "ui_ftpconnectdlg.h"

FtpConnectDlg::FtpConnectDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FtpConnectDlg)
{
    ui->setupUi(this);
//    ui->edit_host->setText("127.0.0.1");//192.168.1.103
    ui->edit_host->setText("192.168.1.103");//192.168.1.103
    ui->edit_user->setText("");
    ui->edit_pass->setText("");
    connect(ui->btn_connect,&QPushButton::clicked,this,&FtpConnectDlg::onConnect);
}

FtpConnectDlg::~FtpConnectDlg()
{
    delete ui;
}

FTP_DATA FtpConnectDlg::ftp_data()
{
    FTP_DATA ftp_data;
    ftp_data.host = ui->edit_host->text();
    ftp_data.user = ui->edit_user->text();
    ftp_data.pass = ui->edit_pass->text();
    return ftp_data;

}

void FtpConnectDlg::onConnect()
{
    QDialog::accept();
}
