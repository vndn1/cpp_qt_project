#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatlogic.h"  // ChatSession과 관련 클래스 포함

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override; // shift+enter & ctrl + t

private slots:
    void on_btn_addMessage_clicked();  // 입력 버튼
    void on_btn_analyze_clicked();     // 분석 버튼

    void on_btn_clear_clicked();       // 클리어 버튼

private:
    Ui::MainWindow *ui;
    ChatSession session;  // 감정 분석용 세션 객체
};

#endif // MAINWINDOW_H
