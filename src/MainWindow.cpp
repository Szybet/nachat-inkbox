#include "MainWindow.hpp"
#include "qdir.h"
#include "ui_MainWindow.h"

#include <algorithm>
#include <unordered_set>

#include <QProgressBar>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QDebug>

#include "matrix/Room.hpp"
#include "matrix/Session.hpp"
#include "matrix/pixmaps.hpp"

#include "sort.hpp"
#include "RoomView.hpp"
#include "ChatWindow.hpp"
#include "JoinDialog.hpp"
#include "MessageBox.hpp"
#include "utils.hpp"
#include <QStandardPaths>

MainWindow::MainWindow(matrix::Session &session)
    : ui(new Ui::MainWindow), session_(session),
      progress_(new QProgressBar(this)), sync_label_(new QLabel(this)),
      thumbnail_cache_{devicePixelRatioF()}, rooms_{session, initial_icon_size(*this), devicePixelRatioF()} {
  ui->setupUi(this);

  this->setAnimated(false);
  progress_->deleteLater();

  qDebug() << "Full screen mainwindow";
  this->showFullScreen();

  ui->status_bar->addPermanentWidget(sync_label_);
  ui->status_bar->showMessage(session.user_id_.value());

  connect(ui->action_log_out, &QAction::triggered, this, &MainWindow::askLogOut);

  connect(ui->action_join, &QAction::triggered, [this]() {
      QPointer<JoinDialog> dialog(new JoinDialog);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      connect(dialog, &QDialog::accepted, [this, dialog]() {
          const QString room = dialog->room();
          auto reply = session_.join(room);
          connect(reply, &matrix::JoinRequest::error, [room, dialog](const QString &msg) {
              if(!dialog) return;
              dialog->setEnabled(true);
              MessageBox::critical(tr("Failed to join room"), tr("Couldn't join %1: %2").arg(room).arg(msg), dialog);
            });
          connect(reply, &matrix::JoinRequest::success, dialog, &QWidget::close);
        });
      dialog->open();
    });

  connect(ui->actionClean_cache, &QAction::triggered, []() {
      QString state_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
      qDebug() << "The dir path for cache is:" << state_path;
      QMessageBox msgBox;
      msgBox.setText("Cache cleaned. Launch the app again for a fresh restart");
      msgBox.exec();
      QDir *dir = new QDir(state_path);
      dir->removeRecursively();
      QApplication::quit();
  });

  connect(&session_, &matrix::Session::error, [this](QString msg) {
      qDebug() << "Session error: " << msg;
    });

  connect(&session_, &matrix::Session::synced_changed, [this]() {
      if(session_.synced()) {
        sync_label_->hide();
      } else {
        sync_label_->setText(tr("Disconnected"));
        sync_label_->show();
      }
    });

  connect(&session_, &matrix::Session::sync_complete, [this]() {
      sync_label_->hide();
  });

  ui->action_quit->setShortcuts(QKeySequence::Quit);
  connect(ui->action_quit, &QAction::triggered, this, &MainWindow::quit);

  connect(ui->room_list, &QListView::activated, [this](const QModelIndex &){
      if(windowAvailable == false) {
          return void();
      }
      std::unordered_set<ChatWindow *> windows;
      for(auto index : ui->room_list->selectionModel()->selectedIndexes()) {
        auto &room = *session_.room_from_id(matrix::RoomID{rooms_.data(index, JoinedRoomListModel::IDRole).toString()});
        auto it = windows_.find(room.id());
        ChatWindow *window = it != windows_.end() ? it->second : nullptr;
        if(!window) {
          if(last_focused_) {
            window = last_focused_; // Add to most recently used window
          } else if(!windows_.empty()) {
            // Select arbitrary window
            window = windows_.begin()->second;
          } else {
            // Create first window
            windowAvailable = false;
            window = spawn_chat_window();
          }
        }
        window->add_or_focus(room);
        windows.insert(window);

        qDebug() << "Test &QListView::activated";
      }
      for(auto window : windows) {
        //window->exec();
        window->show();
      }
    });
  connect(ui->room_list, &QAbstractItemView::iconSizeChanged, &rooms_, &JoinedRoomListModel::icon_size_changed);
  ui->room_list->setModel(&rooms_);

  connect(&thumbnail_cache_, &ThumbnailCache::needs, [this](const matrix::Thumbnail &t) {
      auto fetch = session_.get_thumbnail(t);
      QPointer<MainWindow> self(this);
      connect(fetch, &matrix::ContentFetch::finished, [=](const QString &type, const QString &disposition, const QByteArray &data) {
          (void)disposition;
          if(!self) return;
          QPixmap pixmap = matrix::decode(type, data);

          if(pixmap.width() > t.size().width() || pixmap.height() > t.size().height())
            pixmap = pixmap.scaled(t.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
          pixmap.setDevicePixelRatio(self->devicePixelRatioF());

          self->thumbnail_cache_.set(t, pixmap);
        });
    });

  sync_progress(0, -1);
}

MainWindow::~MainWindow() {
  std::unordered_set<ChatWindow *> windows;
  for(auto &window : windows_) {
    windows.insert(window.second);
  }
  for(auto window : windows) {
    delete window;
  }
  delete ui;
}

void MainWindow::highlight(const matrix::RoomID &room) {
  QWidget *window = windows_.at(room);
  if(!window) {
    window = this;
  }
  window->show();
  QApplication::alert(window);
}

void MainWindow::sync_progress(qint64 received, qint64 total) {
  sync_label_->hide();
}

RoomWindowBridge::RoomWindowBridge(matrix::Room &room, ChatWindow &parent) : QObject(&parent), room_(room), window_(parent) {
  connect(&room, &matrix::Room::sync_complete, this, &RoomWindowBridge::display_changed);
  connect(&parent, &ChatWindow::released, this, &RoomWindowBridge::check_release);
}

void RoomWindowBridge::display_changed() {
  window_.room_display_changed(room_);
}

void RoomWindowBridge::check_release(const matrix::RoomID &room) {
  if(room_.id() == room) deleteLater();
}

ChatWindow *MainWindow::spawn_chat_window() {
  // We don't create these as children to prevent Qt from hinting to WMs that they should be floating
  auto window = new ChatWindow(thumbnail_cache_);
  connect(window, &ChatWindow::destroyed, this, &MainWindow::enableWindowsAgain);

  connect(window, &ChatWindow::focused, [this, window]() {
      last_focused_ = window;
    });
  connect(window, &ChatWindow::claimed, [this, window](const matrix::RoomID &r) {
      windows_[r] = window;
      new RoomWindowBridge(*session_.room_from_id(r), *window);
    });
  connect(window, &ChatWindow::released, [this](const matrix::RoomID &rid) {
      windows_.erase(rid);
    });
  connect(window, &ChatWindow::pop_out, [this](const matrix::RoomID &r, RoomView *v) {
      auto w = spawn_chat_window();
      w->add(*session_.room_from_id(r), v);
      // w->exec();
      w->show();
    });
  return window;
}

void MainWindow::askLogOut() {
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "Confirm", "Do you want to log out?", QMessageBox::Yes|QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    qDebug() << "Yes was clicked";
    emit log_out();
  }
}

void MainWindow::enableWindowsAgain() {
  windowAvailable = true;
}
