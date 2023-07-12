#include "showImage.h"
#include "ui_showImage.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QScreen>

showImage::showImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::showImage)
{
    ui->setupUi(this);
}

showImage::~showImage()
{
    delete ui;
}

void showImage::start(QUrl url) {
    qDebug() << "url image requested" << url;
    this->showFullScreen();

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(url));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    qDebug() << "Downloading image";
    loop.exec();
    qDebug() << "Image downloaded image";

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error downloading image:" << reply->errorString();
        reply->deleteLater();
        return void();
    }

    QPixmap image;
    image.loadFromData(reply->readAll());
    image = image.scaledToWidth(QGuiApplication::screens()[0]->geometry().size().width());
    int limit = QGuiApplication::screens()[0]->geometry().size().height() - ui->pushButton->height() - 70;
    if(image.height() >= limit) {
        image = image.scaledToHeight(limit);
    }
    qDebug() << "image:" << image;
    ui->label->setPixmap(image);

    reply->deleteLater();
    qDebug() << "Showing image";
}

void showImage::on_pushButton_clicked()
{
    this->deleteLater();
    this->close();
}

