#include "JoinDialog.hpp"
#include "ui_JoinDialog.h"

#include <QDebug>

JoinDialog::JoinDialog(QWidget *parent) : QDialog(parent), ui(new Ui::JoinDialog) {
  ui->setupUi(this);

  qDebug() << "Full screen LoginDialog";
  this->showFullScreen();

}

JoinDialog::~JoinDialog() { delete ui; }

QString JoinDialog::room() { return ui->lineEdit->text(); }

void JoinDialog::accept() {
  setEnabled(false);
  setResult(QDialog::Accepted);
  accepted();
}
