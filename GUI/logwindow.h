#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>
#include <QFile>
#include <QDir.h>
#include <QRegularExpression>
#include <QString>
#include <QQueue>



namespace Ui {
class LogWindow;
}

class LogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);
    ~LogWindow();

private slots:
    void filterTextChanged(const QString &text);
    void typeComboBoxChanged(int index);

private:
    Ui::LogWindow *ui;
    QStandardItemModel *logModel;
    // log.txt的相对路径
    QString logFilePath = "./log.txt";
    QList<QStandardItem*> originalLogItems;

    void loadLogContent();
    void filterLogContent();
};

#endif // LOGWINDOW_H
