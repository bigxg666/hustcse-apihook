/********************************************************************************
** Form generated from reading UI file 'logwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGWINDOW_H
#define UI_LOGWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LogWindow
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QListView *listView;
    QComboBox *comboBox;
    QLineEdit *lineEdit;

    void setupUi(QDialog *LogWindow)
    {
        if (LogWindow->objectName().isEmpty())
            LogWindow->setObjectName("LogWindow");
        LogWindow->resize(471, 359);
        gridLayoutWidget = new QWidget(LogWindow);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(10, 10, 451, 341));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        listView = new QListView(gridLayoutWidget);
        listView->setObjectName("listView");
        listView->setFocusPolicy(Qt::NoFocus);
        listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        gridLayout->addWidget(listView, 2, 0, 1, 2);

        comboBox = new QComboBox(gridLayoutWidget);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName("comboBox");

        gridLayout->addWidget(comboBox, 0, 0, 1, 1);

        lineEdit = new QLineEdit(gridLayoutWidget);
        lineEdit->setObjectName("lineEdit");

        gridLayout->addWidget(lineEdit, 0, 1, 1, 1);


        retranslateUi(LogWindow);

        QMetaObject::connectSlotsByName(LogWindow);
    } // setupUi

    void retranslateUi(QDialog *LogWindow)
    {
        LogWindow->setWindowTitle(QCoreApplication::translate("LogWindow", "Dialog", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("LogWindow", "Type", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("LogWindow", "Warning", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("LogWindow", "Error", nullptr));
        comboBox->setItemText(3, QCoreApplication::translate("LogWindow", "Info", nullptr));
        comboBox->setItemText(4, QCoreApplication::translate("LogWindow", "Custom", nullptr));
        comboBox->setItemText(5, QString());

        lineEdit->setText(QCoreApplication::translate("LogWindow", "Search...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogWindow: public Ui_LogWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGWINDOW_H
