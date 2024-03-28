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
    // connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LogWindow::filterLogContent);
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

        // Use a queue to store the last 1000 lines
        QQueue<QString> lastLines;

        while (!in.atEnd())
        {
            QString line = in.readLine();
            lastLines.enqueue(line);

            // If the queue's size is over 1000, remove the first element
            if (lastLines.size() > 1000) {
                lastLines.dequeue();
            }
        }

        // Process the last 1000 lines
        foreach (const QString &line, lastLines) {
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
                logModel->appendRow(item);
            } else if (line.contains("[警告]")) {
                item->setData(QColor(200, 200, 0), Qt::ForegroundRole);  // 深黄色
                logModel->appendRow(item);
            } else if (line.contains("[错误]")) {
                item->setData(QColor(Qt::red), Qt::ForegroundRole);  // 红色
                logModel->appendRow(item);
            } else if (line.contains("[信息]")) {
                item->setData(QColor(Qt::green), Qt::ForegroundRole);  // 绿色
                logModel->appendRow(item);
            }
            QStandardItem* originalItem = new QStandardItem(line);  // Create a new QStandardItem
            originalItem->setData(item->data(Qt::ForegroundRole), Qt::ForegroundRole);  // Copy the color
            originalLogItems.append(originalItem);  // Add the new QStandardItem to originalLogItems
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
    // Create a new model for filtered content
    QStandardItemModel *filteredModel = new QStandardItemModel(this);

    // Filter the lines
    QString filterText = ui->lineEdit->text();
    if(filterText == "Search...")
        filterText = "";
    QString filterType = ui->comboBox->currentText();
    bool isMatched = false;
    for (QStandardItem *item : originalLogItems) {
        QString line = item->text();

        // Reset isMatched if the line has a timestamp
        QRegularExpression timestampRegex("\\[\\d{4}-\\d{1,2}-\\d{1,2} \\d{1,2}:\\d{1,2}:\\d{1,2}\\]");
        if (timestampRegex.match(line).hasMatch()) {
            isMatched = false;
        }
        // Check if the line contains the filter text
        if (line.contains(filterText, Qt::CaseInsensitive) || isMatched) {
            isMatched = true;

            // Check if the line matches the filter type
            QColor itemColor = item->data(Qt::ForegroundRole).value<QColor>();
            if ((filterType == "Type" && (itemColor == QColor(200, 200, 0) || itemColor == QColor(Qt::red) ||
                                          itemColor == QColor(Qt::green) || itemColor == QColor(200, 200, 200))) ||
                (filterType == "Warning" && itemColor == QColor(200, 200, 0)) ||
                (filterType == "Error" && itemColor == QColor(Qt::red)) ||
                (filterType == "Info" && itemColor == QColor(Qt::green)) ||
                (filterType == "Custom" && itemColor == QColor(200, 200, 200))) {
                QStandardItem *newItem = new QStandardItem(line);
                newItem->setData(itemColor, Qt::ForegroundRole);
                filteredModel->appendRow(newItem);
            }
        }
    }

    // Set the filtered model as the new model
    ui->listView->setModel(filteredModel);
}

