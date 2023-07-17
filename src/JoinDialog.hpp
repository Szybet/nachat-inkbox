#ifndef NATIVE_CHAT_JOIN_DIALOG_HPP_
#define NATIVE_CHAT_JOIN_DIALOG_HPP_

#include <QDialog>
#include "smallkeyboard.h"

namespace Ui {
class JoinDialog;
}

class JoinDialog : public QDialog {
  Q_OBJECT

public:
  JoinDialog(QWidget *parent = nullptr);
  ~JoinDialog();

  QString room();

  void accept() override;
  QSize screen;
  smallkeyboard* keyboard;
private:
  Ui::JoinDialog *ui;
};

#endif
