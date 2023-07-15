#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <memory>

#include <QDialog>

#include "matrix/Matrix.hpp"
#include "smallkeyboard.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog {
  Q_OBJECT

public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  void accept() override;
  void reject() override;

  QString username() const;
  QString password() const;
  QString homeserver() const;
  QSize screen;
  smallkeyboard* keyboard;
  void clickedUserName();
  void clickedPassword();
  void clickedServer();
  QTimer* timer;
  void clearCursor();

public slots:
  void checkFocus();
private:
  Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
