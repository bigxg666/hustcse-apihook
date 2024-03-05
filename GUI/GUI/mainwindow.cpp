#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logWindow(new LogWindow(this))
{
    ui->setupUi(this);

    sharedMemoryReader = new SharedMemoryReader();
    sharedMemoryReader->moveToThread(&readerThread);

    connect(&readerThread, &QThread::finished, sharedMemoryReader, &QObject::deleteLater);
    connect(this, &MainWindow::startReadingSharedMemory, sharedMemoryReader, &SharedMemoryReader::readSharedMemory);
    connect(sharedMemoryReader, &SharedMemoryReader::messageReceived, this, &MainWindow::updateInfoText);

    connect(sharedMemoryReader, &SharedMemoryReader::API_Received, this, &MainWindow::insertLogInfo);


    readerThread.start();

}

MainWindow::~MainWindow()
{
    readerThread.terminate();
    readerThread.wait();
    TerminateProcess(processInfo.hProcess, 0);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    delete ui;
}


void MainWindow::on_selectFile_Button_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*)");
    if (!fileName.isEmpty()){
        // fileLineEdit->setText(fileName);
        selectedFileName = fileName;
        ui->Filename_text->setPlainText(fileName);
        // emit textChanged(fileName);
    }
}



MainWindow::ProcessInfo MainWindow::injectDllAndRun(const QString &exePath, const QString &dllPath)
{
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    BOOL success = DetourCreateProcessWithDllEx(NULL, (LPWSTR)exePath.utf16(), NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi, dllPath.toStdString().c_str(), NULL);
    if (!success) {
        qDebug() << "DetourCreateProcessWithDllEx failed: " << GetLastError();
        return ProcessInfo();
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        qDebug() << "CreateToolhelp32Snapshot failed: " << GetLastError();
        return ProcessInfo();
    }
    processInfo = pi;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == pi.dwProcessId) {
                CloseHandle(hSnapshot);
                return ProcessInfo { QString::fromWCharArray(pe.szExeFile), pi.dwProcessId, GetPriorityClass(pi.hProcess) };
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return ProcessInfo();
}


void MainWindow::on_run_Button_clicked()
{
    if (!selectedFileName.isEmpty())
    {
        QFileInfo fileInfo(selectedFileName);
        if (fileInfo.isExecutable())
        {
              // Replace with your DLL path
            ProcessInfo info = injectDllAndRun(selectedFileName, dllPath);
            emit startReadingSharedMemory();
            if (!info.name.isEmpty()) {
#ifdef OUTPUT_TO_FILE
                updateProcessInfo(info);
#else
                qDebug() << "Process Name: " << info.name;
                qDebug() << "Process ID: " << info.id;
                qDebug() << "Process Priority: " << info.priority;
#endif
            } else {
                qDebug() << "Failed to inject DLL and run the process.";
            }
        }
        else
        {
            qDebug() << "Selected file is not an executable.";
        }
    }
    // emit startReadingSharedMemory();
}

void MainWindow::updateProcessInfo(const ProcessInfo& info) {
        ui->id->setText(QString::number(info.id));  // 设置进程ID
        ui->name->setText(info.name);  // 设置进程名称
        ui->priority->setText(QString::number(info.priority));  // 设置进程优先级
}


void MainWindow::on_log_button_clicked()
{
    delete logWindow;
    logWindow = new LogWindow(this);
    logWindow->show();
}

void MainWindow::updateInfoText(QString message){
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->Info_text);
    item->setText(0, message);  // 将消息设置到树的节点上

    // 根据消息内容设置文本颜色
    if (message.contains("错误")) {
        item->setForeground(0, QBrush(Qt::red));  // 将文本设为红色
    } else if (message.contains("警告")) {
        item->setForeground(0, QBrush(QColor(200, 200, 0)));  // 将文本设为黄色
    }

    ui->Info_text->addTopLevelItem(item);  // 添加节点到树上
    ui->Info_text->resizeColumnToContents(0);  // 调整列宽以适应内容
}

void MainWindow::insertLogInfo(LogInfo* logInfo)
{
    // 将系统时间转换为字符串
    QString time = QString("%1-%2-%3 %4:%5:%6")
                       .arg(logInfo->st.wYear)
                       .arg(logInfo->st.wMonth)
                       .arg(logInfo->st.wDay)
                       .arg(logInfo->st.wHour)
                       .arg(logInfo->st.wMinute)
                       .arg(logInfo->st.wSecond);

    // 创建一个新的树形项目来存储时间和函数名
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->API_text);
    item->setText(0, time);
    item->setText(1, logInfo->funcName);

    // 遍历所有参数
    for(int i = 0; i < logInfo->argNum; i++) {
        // 创建一个新的树形项目来存储参数名称和数据
        QTreeWidgetItem* argItem = new QTreeWidgetItem(item);
        argItem->setText(0, logInfo->argName[i]);
        argItem->setText(1, logInfo->argData[i]);
    }

    // 将新的树形项目添加到树形小部件中
    ui->API_text->addTopLevelItem(item);
}



