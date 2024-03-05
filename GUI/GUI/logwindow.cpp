#include "logwindow.h"
#include "ui_logwindow.h"

LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogWindow)
{
    ui->setupUi(this);

    //初始化Log界面
    loadLogContent();
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &LogWindow::filterLogContent);
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LogWindow::filterLogContent);

}

LogWindow::~LogWindow()
{
    delete ui;
}

void LogWindow::filterTextChanged(const QString &text)
{
    filterLogContent();
}

void LogWindow::typeComboBoxChanged(int index)
{
    filterLogContent();
}

void LogWindow::loadLogContent()
{
    logModel = new QStandardItemModel(this);

    QString currentPath = QDir::currentPath();
    QString filePath = currentPath + "/log/log.txt";

    QFile logFile(logFilePath);
    if (!logFile.exists())
    {
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << " create log.txt";
            logFile.close();
        }
        else
        {
            qDebug() << "Failed to create log.txt";
            return;
        }
    }

    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&logFile);
        bool isCaptured = false;
        logFile.seek(0);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStandardItem* item = new QStandardItem(line);

            QRegularExpression timestampRegex("\\[\\d{4}-\\d{1,2}-\\d{1,2} \\d{1,2}:\\d{1,2}:\\d{1,2}\\]");
            if (timestampRegex.match(line).hasMatch()) {
                isCaptured = false;
            }

            if (line.contains("[截获]")) {
                isCaptured = true;
            }

            if (isCaptured) {
                item->setData(QColor(200, 200, 200), Qt::ForegroundRole);  // 灰色
            } else if (line.contains("[警告]")) {
                item->setData(QColor(200, 200, 0), Qt::ForegroundRole);  // 深黄色
            } else if (line.contains("[错误]")) {
                item->setData(QColor(Qt::red), Qt::ForegroundRole);  // 红色
            } else if (line.contains("[信息]")) {
                item->setData(QColor(Qt::green), Qt::ForegroundRole);  // 绿色
            }

            logModel->appendRow(item);
        }

        logFile.close();
    }
    else
    {
        qDebug() << "Failed to open log.txt";
    }

    ui->listView->setModel(logModel);
}


void LogWindow::filterLogContent()
{
    QString filterText = ui->lineEdit->text();
    QString selectedType = ui->comboBox->currentText();

    logModel->clear();

    QFile logFile(logFilePath);
    if (!logFile.exists())
    {
        qDebug() << "log.txt does not exist";
        return;
    }

    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&logFile);
        bool isCaptured = false;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.contains(filterText, Qt::CaseInsensitive) &&
                (selectedType == "Default" || line.contains(selectedType, Qt::CaseInsensitive)))
            {
                QStandardItem* item = new QStandardItem(line);

                QRegularExpression timestampRegex("\\[\\d{4}-\\d{1,2}-\\d{1,2} \\d{1,2}:\\d{1,2}:\\d{1,2}\\]");
                if (timestampRegex.match(line).hasMatch()) {
                    isCaptured = false;
                }

                if (line.contains("[截获]")) {
                    isCaptured = true;
                }

                if (isCaptured) {
                    item->setData(QColor(200, 200, 0), Qt::ForegroundRole);  // 深黄色
                } else if (line.contains("[警告]")) {
                    item->setData(QColor(Qt::red), Qt::ForegroundRole);
                }

                logModel->appendRow(item);
            }
        }
        logFile.close();
    }
    else
    {
        qDebug() << "Failed to open log.txt";
    }
}

