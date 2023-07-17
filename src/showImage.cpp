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
    QByteArray bytes = reply->readAll();
    image.loadFromData(bytes);
    image = image.scaledToWidth(QGuiApplication::screens()[0]->geometry().size().width());
    if(image.isNull() == false) {
        qDebug() << "image:" << image;
        int limit = QGuiApplication::screens()[0]->geometry().size().height() - ui->pushButton->height() - 70;
        if(image.height() >= limit) {
            image = image.scaledToHeight(limit);
        }
        ui->label->setPixmap(image);
        ui->textBrowser->hide();
    } else {
        qDebug() << "Trying text";
        ui->textBrowser->setPlainText(QString::fromStdString(bytes.toStdString()));
        ui->label->hide();
    }

    reply->deleteLater();
    qDebug() << "Showing file";
}

void showImage::on_pushButton_clicked()
{
    this->deleteLater();
    this->close();
}

