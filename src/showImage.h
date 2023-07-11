#ifndef SHOWIMAGE_H
#define SHOWIMAGE_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class showImage;
}

class showImage : public QDialog
{
    Q_OBJECT

public:
    explicit showImage(QWidget *parent = nullptr);
    ~showImage();
    void start(QUrl url);

private slots:
    void on_pushButton_clicked();

private:
    Ui::showImage *ui;
};

#endif // SHOWIMAGE_H
