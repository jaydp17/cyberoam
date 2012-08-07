#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->cyb_image->setPixmap(QPixmap::fromImage(QImage(":/cyb/webclientportallogo.png")));
    this->setWindowTitle("Cyberoam AutoLogin");
    this->setWindowIcon(QIcon(":/cyb/icon.png"));
    this->setFixedSize(this->width(),this->height());

    cyberoamURL.setUrl("http://10.100.56.55:8090/httpclient.html");

    manager = new QNetworkAccessManager(this);
    req.setUrl(cyberoamURL);
    connect(ui->login_b,SIGNAL(clicked()),this,SLOT(login()));

    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(readReply(QNetworkReply*)));
    connect(this,SIGNAL(loggedin()),this,SLOT(declareLoggedIN()));
    connect(this,SIGNAL(loggedOff()),this,SLOT(declareLoggedOFF()));

    isLoggedin = false;

    QSettings setting("daiict","cyberoam_autoLogin");
    QString uname = setting.value("uname","").toString();
    if(uname != ""){
        ui->remember->setChecked(true);
        ui->user_field->setText(uname);
        ui->pass_field->setText(setting.value("pass","").toString());
    }

    tray = new QSystemTrayIcon(this);
    tray->setIcon(QIcon(":/cyb/icon.png"));
    tray->setToolTip("Cyberoam AutoLogin Client");
    traymode = wait4logout = supressMessage = false;

    createActions();
    createTrayMenu();

    timeInterval = 179;

    tm.setInterval(179*60*1000);
    connect(&tm,SIGNAL(timeout()),this,SLOT(callLogin()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::WindowStateChange){
        if(this->windowState() & Qt::WindowMinimized){
            tray->show();
            traymode = true;
            QTimer::singleShot(250, this, SLOT(hide()));
            QTimer::singleShot(1000,this,SLOT(showTrayMessage()));
        }
    }
    QMainWindow::changeEvent(ev);
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if(ev->type() == QEvent::Close){
        if(isLoggedin){
            wait4logout = true;
            login(false);
            ev->ignore();
        } else {
            QMainWindow::closeEvent(ev);
        }
    }
}

void MainWindow::readReply(QNetworkReply *rply)
{
    if(rply->error() == QNetworkReply::NoError){
        QString response = QString::fromUtf8(rply->readAll());
        qDebug() << response;
        if(response.contains("Make sure your password is correct")){
            QMessageBox::critical(this,"Login failed","The system could not log you on.\nMake sure your password is correct.");
        } else if(response.contains("Maximum",Qt::CaseInsensitive)){
            QMessageBox::critical(this,"Login failed","You have reached Maximum Login Limit.");
        } else if(response.contains("You have successfully logged in")){
            emit loggedin();
        } else if(response.contains("You have successfully logged off")){
            emit loggedOff();
            if(wait4logout)
                this->close();
        }
    } else {
        qDebug() << "error";
        QMessageBox::critical(this,"Error Occured", rply->errorString());
    }
}

void MainWindow::login(bool timer)
{
    supressMessage = timer;
    if(ui->remember->isChecked()){
        QSettings setting("daiict","cyberoam_autoLogin");
        setting.setValue("uname",ui->user_field->text());
        setting.setValue("pass",ui->pass_field->text());
        setting.sync();
    } else {
        QSettings setting("daiict","cyberoam_autoLogin");
        setting.clear();
    }
    if(traymode){
        if(ui->user_field->text() == "" || ui->pass_field->text() == ""){
            this->show();
            tray->showMessage("Cannot Login","Incomplete credentials provided");
            showDialog();
            return;
        }
    }
    if(isLoggedin && !timer){
        QUrl credentials;
        credentials.addQueryItem("mode","193");
        credentials.addQueryItem("username",ui->user_field->text());

        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        manager->post(req,credentials.encodedQuery());
    } else {
        QUrl credentials;
        credentials.addQueryItem("mode","191");
        credentials.addQueryItem("username",ui->user_field->text());
        credentials.addQueryItem("password",ui->pass_field->text());

        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        manager->post(req,credentials.encodedQuery());
        tm.stop();
        tm.start();
    }
}

void MainWindow::declareLoggedIN()
{
    if(traymode && !supressMessage){
        tray->showMessage("Notification" , ui->user_field->text() + " successfully logged in");
    }
    ui->login_b->setText("Logout");
    isLoggedin = true;
    log_out->setEnabled(true);
    log_in->setEnabled(false);
}

void MainWindow::declareLoggedOFF()
{
    if(traymode){
        tray->showMessage("Notification", ui->user_field->text() + " successfully logged off");
    }
    ui->login_b->setText("Login");
    isLoggedin = false;
    log_out->setEnabled(false);
    log_in->setEnabled(true);
}

void MainWindow::createActions()
{
    shw = new QAction("Show",this);
    connect(shw,SIGNAL(triggered()),this,SLOT(showDialog()));

    log_in = new QAction("Login",this);
    connect(log_in,SIGNAL(triggered()),this,SLOT(login()));

    log_out = new QAction("Logout",this);
    log_out->setEnabled(false);
    connect(log_out,SIGNAL(triggered()),this,SLOT(login()));

    about = new QAction("About",this);
    connect(about,SIGNAL(triggered()),&about_dialog,SLOT(show()));

    quit = new QAction("Quit",this);
    connect(quit,SIGNAL(triggered()),this,SLOT(close()));
}

void MainWindow::createTrayMenu()
{
    trayMenu = new QMenu(this);
    trayMenu->addAction(shw);
    trayMenu->addAction(log_in);
    trayMenu->addAction(log_out);
    trayMenu->addSeparator();
    trayMenu->addAction(about);
    trayMenu->addAction(quit);
    tray->setContextMenu(trayMenu);
}

void MainWindow::showDialog()
{
    this->show();
    this->activateWindow();
}

void MainWindow::showTrayMessage()
{
    tray->showMessage("Notification","I'm alive :), sitting here..");
}

void MainWindow::callLogin()
{
    login(true);
}
