#include "LoginDialog.hpp"
#include "qscreen.h"
#include "ui_LoginDialog.h"

#include <QSettings>

#include "matrix/Session.hpp"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog) {
  ui->setupUi(this);

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
  keyboard->setDisabled(true);
  keyboard->dontExit();
  QObject::connect(this, &QWidget::destroyed, keyboard, &QWidget::close);

  ui->buttonBox->addButton(tr("Quit"), QDialogButtonBox::RejectRole);
  ui->buttonBox->addButton(tr("Sign In"), QDialogButtonBox::AcceptRole);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &LoginDialog::checkFocus);
  timer->setInterval(500);
  timer->start();
  QObject::connect(this, &QWidget::destroyed, timer, &QTimer::stop);
  QObject::connect(this, &QWidget::destroyed, timer, &QTimer::deleteLater);

  QSettings settings;

  auto username = settings.value("login/username");
  if(!username.isNull()) {
    ui->username->setText(settings.value("login/username").toString());
    ui->password->setFocus(Qt::OtherFocusReason);
  }

  auto homeserver = settings.value("login/homeserver");
  if(!homeserver.isNull()) {
    ui->homeserver->setText(homeserver.toString());
  }  
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::accept() {
  setDisabled(true);
  setResult(QDialog::Accepted);
  clearCursor();
  accepted();
}

void LoginDialog::reject() {
  qDebug() << "Adios";
  QCoreApplication::quit();
}

QString LoginDialog::username() const { return ui->username->text(); }
QString LoginDialog::password() const { return ui->password->text(); }
QString LoginDialog::homeserver() const { return ui->homeserver->text(); }

void LoginDialog::checkFocus() {
  // qDebug() << "Check focus";
  if(ui->username->hasFocus()) {
    ui->username->clearFocus();
    clearCursor();
    clickedUserName();
  } else if(ui->password->hasFocus()) {
    ui->password->clearFocus();
    clearCursor();
    clickedPassword();
  } else if(ui->homeserver->hasFocus()) {
    ui->homeserver->clearFocus();
    clearCursor();
    clickedServer();
  }
}

void LoginDialog::clickedUserName() {
  qDebug() << "clickedUserName";
  keyboard->setDisabled(false);
  keyboard->start(ui->username);
}
void LoginDialog::clickedPassword() {
  qDebug() << "clickedPassword";
  keyboard->setDisabled(false);
  keyboard->start(ui->password);
}
void LoginDialog::clickedServer() {
  qDebug() << "clickedServer";
  keyboard->setDisabled(false);
  keyboard->start(ui->homeserver);
}

void LoginDialog::clearCursor() {
  ui->password->setText(QString(ui->password->text()).replace(QString(keyboard->cursorCharacter), ""));
  ui->homeserver->setText(QString(ui->homeserver->text()).replace(QString(keyboard->cursorCharacter), ""));
  ui->username->setText(QString(ui->username->text()).replace(QString(keyboard->cursorCharacter), ""));
}
