#include "RoomMenu.hpp"

#include <QFileDialog>
#include <QPointer>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

#include "matrix/Session.hpp"
#include "MessageBox.hpp"
#include "qapplication.h"
#include "qscreen.h"
#include "koboplatformfunctions.h"

RoomMenu::RoomMenu(matrix::Room &room, RoomView *parent) : QMenu(parent), room_(room) {
  {
    auto upload = addAction(QIcon::fromTheme("document-open"), tr("Upload &file..."));
    auto file_dialog = new QFileDialog(parent);
    connect(upload, &QAction::triggered, file_dialog, &QDialog::open);
    connect(upload, &QAction::triggered, [file_dialog]() {
        file_dialog->show();
        file_dialog->move(0, 0);
        file_dialog->resize(QGuiApplication::screens()[0]->geometry().size());
        file_dialog->setWindowModality(Qt::WindowModal);
    });
    connect(file_dialog, &QFileDialog::fileSelected, this, &RoomMenu::upload_file);
  }

  addSeparator();

  {
    auto Refresh = addAction(QIcon::fromTheme("refresh"), tr("Refresh screen"));
    connect(Refresh, &QAction::triggered, [] {
        KoboPlatformFunctions::doManualRefresh(QRect(QPoint(0, 0), QGuiApplication::screens()[0]->geometry().size()));
    });
  }

  addSeparator();

  {
    auto Exit = addAction(QIcon::fromTheme("system-log-out"), tr("Exit"));
    connect(Exit, &QAction::triggered, parent, &RoomView::closeParent);
  }
}

void RoomMenu::upload_file(const QString &path) {
  auto file = std::make_shared<QFile>(path);
  QFileInfo info(*file);
  if(!file->open(QIODevice::ReadOnly)) {
    MessageBox::critical(tr("Error opening file"), tr("Couldn't open %1: %2").arg(info.fileName()).arg(file->errorString()), parentWidget());
    return;
  }

  const QString &type = QMimeDatabase().mimeTypeForFile(info).name();
  auto reply = room_.session().upload(*file, type, info.fileName());
  QPointer<matrix::Room> room(&room_);
  // This closure captures 'file' to ensure its outlives the network request
  connect(reply, &matrix::ContentPost::success, [file, room, info, type](const QString &uri) {
      if(!room) return;
      room->send_file(uri, info.fileName(), type, info.size());
    });
  QPointer<QWidget> parent(parentWidget());
  connect(reply, &matrix::ContentPost::error, [parent, info](const QString &msg) {
      MessageBox::critical(tr("Error uploading file"), tr("Couldn't upload %1: %2").arg(info.fileName()).arg(msg), parent);
    });
}
