/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton_8;
    QPushButton *pushButton_2;
    QPushButton *pushButton_7;
    QPushButton *pushButton_10;
    QPushButton *pushButton_4;
    QPushButton *pushButton;
    QPushButton *pushButton_9;
    QPushButton *pushButton_11;
    QPushButton *pushButton_3;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        pushButton_8 = new QPushButton(centralwidget);
        pushButton_8->setObjectName("pushButton_8");
        pushButton_8->setGeometry(QRect(320, 180, 111, 41));
        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(180, 20, 111, 41));
        pushButton_7 = new QPushButton(centralwidget);
        pushButton_7->setObjectName("pushButton_7");
        pushButton_7->setGeometry(QRect(180, 180, 111, 41));
        pushButton_10 = new QPushButton(centralwidget);
        pushButton_10->setObjectName("pushButton_10");
        pushButton_10->setGeometry(QRect(460, 180, 111, 41));
        pushButton_4 = new QPushButton(centralwidget);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(460, 20, 111, 41));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(40, 20, 111, 41));
        pushButton_9 = new QPushButton(centralwidget);
        pushButton_9->setObjectName("pushButton_9");
        pushButton_9->setGeometry(QRect(180, 90, 111, 41));
        pushButton_11 = new QPushButton(centralwidget);
        pushButton_11->setObjectName("pushButton_11");
        pushButton_11->setGeometry(QRect(40, 250, 111, 41));
        pushButton_3 = new QPushButton(centralwidget);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(320, 20, 111, 41));
        pushButton_5 = new QPushButton(centralwidget);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(40, 90, 111, 41));
        pushButton_6 = new QPushButton(centralwidget);
        pushButton_6->setObjectName("pushButton_6");
        pushButton_6->setGeometry(QRect(40, 180, 111, 41));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton_8->setText(QCoreApplication::translate("MainWindow", "\350\207\252\345\244\215\345\210\266", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "MessageBoxW", nullptr));
        pushButton_7->setText(QCoreApplication::translate("MainWindow", "\344\277\256\346\224\271exe\346\226\207\344\273\266", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow", "\344\277\256\346\224\271\345\220\257\345\212\250\346\263\250\345\206\214\350\241\250", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\346\223\215\344\275\234", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "MessageBoxA", nullptr));
        pushButton_9->setText(QCoreApplication::translate("MainWindow", "socket\346\223\215\344\275\234", nullptr));
        pushButton_11->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\345\205\266\344\273\226\346\226\207\344\273\266\345\244\271", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "\345\240\206\346\223\215\344\275\234", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "\346\263\250\345\206\214\350\241\250\346\223\215\344\275\234", nullptr));
        pushButton_6->setText(QCoreApplication::translate("MainWindow", "\345\240\206\345\274\202\345\270\270\346\223\215\344\275\234", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
