#ifndef NATIVE_CHAT_MATRIX_SESSION_H_
#define NATIVE_CHAT_MATRIX_SESSION_H_

#include <unordered_map>
#include <chrono>
#include <experimental/optional>
#include <vector>

#include <QObject>
#include <QString>
#include <QUrlQuery>
#include <QTimer>

#include <lmdb++.h>

#include "../QStringHash.hpp"

#include "Room.hpp"
#include "Content.hpp"

class QNetworkRequest;
class QNetworkReply;
class QIODevice;

namespace matrix {

namespace proto {
struct Sync;
}

class Matrix;

class JoinRequest : public QObject {
  Q_OBJECT

public:
  explicit JoinRequest(QObject *parent) : QObject(parent) {}

signals:
  void success(const RoomID &id);
  void error(const QString &msg);
};

class ContentFetch : public QObject {
  Q_OBJECT

public:
  explicit ContentFetch(QObject *parent = nullptr) : QObject(parent) {}

signals:
  void finished(const QString &type, const QString &disposition, const QByteArray &data);
  void error(const QString &msg);
};

class ContentPost : public QObject {
  Q_OBJECT

public:
  explicit ContentPost(QObject *parent = nullptr) : QObject(parent) {}

signals:
  void success(const QString &content_uri);
  void progress(qint64 completed, qint64 total);
  void error(const QString &msg);
};

struct SessionInit;

class Session : public QObject {
  Q_OBJECT

public:
  Session(Matrix& universe, QUrl homeserver, UserID user_id, QString access_token);

  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;

  const QString &access_token() const { return access_token_; }
  const UserID &user_id() const { return user_id_; }
  const QUrl &homeserver() const { return homeserver_; }

  void log_out();

  bool synced() const { return synced_; }
  std::vector<Room *> rooms();
  Room *room_from_id(const RoomID &r) {
    auto it = rooms_.find(r);
    if(it == rooms_.end()) return nullptr;
    return &it->second.room;
  }
  const Room *room_from_id(const RoomID &r) const {
    auto it = rooms_.find(r);
    if(it == rooms_.end()) return nullptr;
    return &it->second.room;
  }

  size_t buffer_size() const { return buffer_size_; }
  void set_buffer_size(size_t size) { buffer_size_ = size; }

  QNetworkReply *get(const QString &path, QUrlQuery query = QUrlQuery());

  QNetworkReply *post(const QString &path, QJsonObject body = QJsonObject(), QUrlQuery query = QUrlQuery());

  QNetworkReply *put(const QString &path, QJsonObject body);

  ContentFetch *get(const Content &);

  ContentFetch *get_thumbnail(const Thumbnail &);

  ContentPost *upload(QIODevice &data, const QString &content_type, const QString &filename);

  TransactionID get_transaction_id();

  JoinRequest *join(const QString &id_or_alias);

  QUrl ensure_http(const QUrl &) const;
  // Converts mxc URLs to http URLs on this homeserver, otherwise passes through

  const QUrl homeserver_;
  const UserID user_id_;

signals:
  void logged_out();
  void error(QString message);
  void synced_changed();
  void joined(matrix::Room &room);
  void sync_progress(qint64 received, qint64 total);
  void sync_complete();

private:
  struct RoomInfo {
    Room room;
    std::experimental::optional<lmdb::dbi> members;
    std::vector<Member> member_changes;

    RoomInfo(Matrix &universe, Session &session, const proto::JoinedRoom &joined_room) : room{universe, session, joined_room} {}
    RoomInfo(Matrix &universe, Session &session, RoomID id, const QJsonObject &initial,
             gsl::span<const Member> members) : room{universe, session, id, initial, members} {}
  };

  Matrix &universe_;
  QString access_token_;
  lmdb::env env_;
  lmdb::dbi state_db_, room_db_;
  size_t buffer_size_;
  std::unordered_map<RoomID, RoomInfo> rooms_;
  bool synced_;
  std::experimental::optional<SyncCursor> next_batch_;
  QNetworkReply *sync_reply_;
  QTimer sync_retry_timer_;

  std::chrono::steady_clock::time_point last_sync_error_;
  // Last time a sync failed. Used to ensure we don't spin if errors happen quickly.

  Session(Matrix& universe, QUrl homeserver, UserID user_id, QString access_token, SessionInit &&init);

  QNetworkRequest request(const QString &path, QUrlQuery query = QUrlQuery(), const QString &content_type = "application/json");

  void sync();
  void sync(QUrlQuery query);
  void handle_sync_reply();
  void dispatch(const proto::Sync &sync);
  void update_cache(const proto::Sync &sync);

  template<typename ...Ts>
  RoomInfo &add_room(const RoomID &id, Ts &&...ts);
};

}

#endif
