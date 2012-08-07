#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->logo->setPixmap(QPixmap::fromImage(QImage(":/cyb/icon.png")));
    QString url = "https://github.com/jaydeep17/cyberoam.git";
    QString description = "<h2>Cyberoam AutoLogin Client</h2>GNU App for cyberoam login management<br><h3>Developed using Qt "+QString(qVersion())+"</h3>This app was developed by Jaydeep (ID : 201101040)<br><br>You can find the source at:<br> <a href="+url+">"+url+"</a><br>";
    ui->desc->setText(description);

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(ui->close_b,SIGNAL(clicked()),this,SLOT(hide()));
}

Dialog::~Dialog()
{
    delete ui;
}
