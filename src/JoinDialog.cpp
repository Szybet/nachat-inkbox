#include "JoinDialog.hpp"
#include "qscreen.h"
#include "ui_JoinDialog.h"

#include <QDebug>

JoinDialog::JoinDialog(QWidget *parent) : QDialog(parent), ui(new Ui::JoinDialog) {
  ui->setupUi(this);

  qDebug() << "Full screen LoginDialog";
  screen = QGuiApplication::screens()[0]->geometry().size();
  qDebug() << "Screen:" << screen;
  this->setFixedSize(screen.width(), (screen.height() / 2) + (screen.height() / 6));
  this->move(0, 0);

  // Very interesting - parent must be parent because it cant just go outside of this window
  keyboard = new smallkeyboard(parent);
  keyboard->show();
  keyboard->setFixedSize(screen.width(), (screen.height() / 2) - (screen.height() / 6));
  keyboard->move(0, (screen.height() / 2) + (screen.height() / 6));
  keyboard->raise();
  keyboard->dontExit();
  QObject::connect(this, &QWidget::destroyed, keyboard, &QWidget::close);
  keyboard->start(ui->lineEdit);

}

JoinDialog::~JoinDialog() { delete ui; }

QString JoinDialog::room() { return ui->lineEdit->text(); }

void JoinDialog::accept() {
  setEnabled(false);
  setResult(QDialog::Accepted);
  accepted();
}
