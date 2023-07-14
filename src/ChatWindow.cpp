#include "ChatWindow.hpp"
#include "ui_ChatWindow.h"

#include <QIcon>
#include <QCloseEvent>
#include <QScreen>

#include "matrix/Room.hpp"

#include "RoomView.hpp"
#include "RoomViewList.hpp"
#include "EntryBox.hpp"

ChatWindow::ChatWindow(ThumbnailCache &cache, QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatWindow), room_list_(new RoomViewList(this)), cache_{cache} {
    ui->setupUi(this);

    screen = QGuiApplication::screens()[0]->geometry().size();
    qDebug() << "Screen:" << screen;
    this->setFixedSize(screen.width(), (screen.height() / 2) + (screen.height() / 6));
    this->move(0, 0);

    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags | Qt::Tool);

    // Very interesting - parent must be parent because it cant just go outside of this window
    keyboard = new smallkeyboard(parent);
    keyboard->show();
    keyboard->setFixedSize(screen.width(), (screen.height() / 2) - (screen.height() / 6));
    keyboard->move(0, (screen.height() / 2) + (screen.height() / 6));
    keyboard->raise();

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Window);

    qDebug() << "Not adding things to qsplitter";
    connect(ui->room_stack, &QStackedWidget::currentChanged, this, &ChatWindow::current_changed);

    connect(room_list_, &RoomViewList::activated, [this](const matrix::RoomID &room) {
        auto &view = *rooms_.at(room);
        ui->room_stack->setCurrentWidget(&view);
        view.setFocus();
        if(isActiveWindow())
            view.selected();
        focused(room);
    });
    connect(room_list_, &RoomViewList::claimed, this, &ChatWindow::claimed);
    connect(room_list_, &RoomViewList::released, [this](const matrix::RoomID &room) {
        auto it = rooms_.find(room);
        if(it != rooms_.end()) {
            auto view = it->second;
            ui->room_stack->removeWidget(view);
            // TODO: Pass ownership elsewhere if necessary
            view->setParent(nullptr);
            rooms_.erase(it);
            delete view;
        }
        switch(ui->room_stack->count()) {
        case 0: close(); break;
        case 1: room_list_->hide(); break;
        default: break;
        }
        released(room);
    });
    connect(room_list_, &RoomViewList::pop_out, [this](const matrix::RoomID &room) {
        auto it = rooms_.find(room);
        auto view = it->second;
        rooms_.erase(it);
        room_list_->release(room);
        pop_out(room, view);
    });
}

ChatWindow::~ChatWindow() { delete ui; }

void ChatWindow::add(matrix::Room &r, RoomView *v) {
    qDebug() << "add called";
    v->setParent(this);
    connect(v, &RoomView::closeParent, this, &ChatWindow::closeThisWidget);
    rooms_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(r.id()),
        std::forward_as_tuple(v));
    ui->room_stack->addWidget(v);
    room_list_->add(r);
    if(room_list_->count() == 2) {
        room_list_->show();
    }
    room_list_->activate(r.id());
    v->setFocus();
    keyboard->start(v->giveMeText());
}

void ChatWindow::add_or_focus(matrix::Room &room) {
    qDebug() << "add_or_focus called";
    RoomView *view;
    connect(view, &RoomView::closeParent, this, &ChatWindow::closeThisWidget);
    if(rooms_.find(room.id()) == rooms_.end()) {
        view = new RoomView(cache_, room, this);
        add(room, view);
    } else {
        room_list_->activate(room.id());
        view = static_cast<RoomView*>(ui->room_stack->currentWidget());
    }
    view->setFocus();
    keyboard->start(view->giveMeText());
}

void ChatWindow::room_display_changed(matrix::Room &room) {
    room_list_->update_display(room);
    update_title();
}

RoomView *ChatWindow::take(const matrix::RoomID &room) {
    auto it = rooms_.find(room);
    auto view = it->second;
    rooms_.erase(it);
    released(room);
    return view;
}

void ChatWindow::update_title() {
    if(auto w = ui->room_stack->currentWidget()) {
        setWindowTitle(static_cast<RoomView*>(w)->room().pretty_name_highlights());
    } else {
        setWindowTitle("");
    }
}

void ChatWindow::current_changed(int i) {
    (void)i;
    update_title();
}

void ChatWindow::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    if(e->type() == QEvent::ActivationChange && isActiveWindow()) {
        focused(focused_room());
    }
}

void ChatWindow::closeEvent(QCloseEvent *evt) {
    for(auto &r : rooms_) {
        released(r.first);
    }
    evt->accept();
}

const matrix::RoomID &ChatWindow::focused_room() const {
    return static_cast<RoomView*>(ui->room_stack->currentWidget())->room().id();
}

void ChatWindow::closeThisWidget() {
    qDebug() << "Chatwindow closing";
    keyboard->close();
    this->close();
}
