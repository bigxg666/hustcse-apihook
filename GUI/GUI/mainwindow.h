#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QProcess>
#include <QThread>
#include <windows.h>
#include <detours.h>
#include <tlhelp32.h>
#include "logwindow.h"
#include "SharedMemoryReader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    struct ProcessInfo {
        QString name;
        DWORD id;
        DWORD priority;
    };

    ProcessInfo injectDllAndRun(const QString &exePath, const QString &dllPath);

signals:
    void startReadingSharedMemory();
    // void textChanged(const QString &text);

private slots:
    // void on_plainTextEdit_textChanged();

    void on_selectFile_Button_clicked();

    // void on_textEdit_textChanged(const QString &text);

    void on_run_Button_clicked();
    // void on_startButton_clicked();

    void on_log_button_clicked();

    void updateInfoText(QString message);

    void updateProcessInfo(const ProcessInfo& info);

    void insertLogInfo(LogInfo* logInfo);

    void on_Error_Clear_clicked();

    void on_Warning_Clear_clicked();


private:
    Ui::MainWindow *ui;
    QLineEdit *fileLineEdit;
    QString selectedFileName;
    LogWindow *logWindow;
    QThread readerThread;
    SharedMemoryReader *sharedMemoryReader;
    PROCESS_INFORMATION processInfo;
    // hookdll的绝对路径
    QString dllPath = "C:/Users/zhuor/Desktop/secure_design/hustcse-apihook/hookDll/hookDLL.dll";
    // QListView *listView;
};
#endif // MAINWINDOW_H
