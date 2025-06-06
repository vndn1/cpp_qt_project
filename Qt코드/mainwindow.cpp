#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatlogic.h"

#include <QString>
#include <QMessageBox>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 초기화 시 회피 표현 불러오기
    loadAvoidancePhrases("avoid.txt");

    // 초기 메시지 비움
    ui->lineEdit_message->clear();
    ui->textEdit_result->clear();

    // 🔹 이벤트 필터 연결
    ui->lineEdit_message->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_addMessage_clicked()
{
    QString sender = ui->comboBox_sender->currentText();
    QString message = ui->lineEdit_message->toPlainText();
    //QString message = ui->lineEdit_message->text();

    if (message.trimmed().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "메시지를 입력해주세요.");
        return;
    }

    session.addMessage(sender.toStdString(), message.toStdString());
    ui->lineEdit_message->clear();
    ui->textEdit_result->append("[" + sender + "] " + message);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lineEdit_message && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        // 🔸 Ctrl + T → 화자 전환 기능 추가
        if ((keyEvent->key() == Qt::Key_T) && (keyEvent->modifiers() & Qt::ControlModifier)) {
            // 현재 화자 확인
            QString current = ui->comboBox_sender->currentText();
            if (current == "A")
                ui->comboBox_sender->setCurrentText("B");
            else
                ui->comboBox_sender->setCurrentText("A");

            return true; // 기본 동작 차단
        }

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                // 🔸 Shift+Enter → 줄바꿈
                ui->lineEdit_message->insertPlainText("\n");
            } else {
                // 🔸 Enter만 → 메시지 전송
                on_btn_addMessage_clicked();
            }
            return true;  // 기본 동작 차단
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_btn_analyze_clicked()
{
    // 분석 실행 → 두 문자열로 반환됨
    QString summary = QString::fromStdString(session.analyzeSession());
    QString perUser = QString::fromStdString(session.analyzeSessionPerUser());

    ui->textEdit_result->append("");  // 줄바꿈
    ui->textEdit_result->append(summary);
    ui->textEdit_result->append(perUser);
}

void MainWindow::on_btn_clear_clicked()
{
    ui->textEdit_result->clear();       // 분석 결과 텍스트창 초기화
    session.clear();                    // ChatSession 내부 데이터 전부 초기화
    ui->lineEdit_message->clear();      // 입력창도 초기화 (선택)
}
