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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QTextEdit *priority;
    QTextEdit *name;
    QTextEdit *id;
    QLabel *Process_Name_2;
    QLabel *Process_Name;
    QLabel *Process_Name_3;
    QTreeWidget *API_text;
    QWidget *verticalLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QTreeWidget *Modules_Text;
    QPushButton *Error_Clear;
    QPushButton *Warning_Clear;
    QTreeWidget *Info_text;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QTextEdit *Filename_text;
    QPushButton *selectFile_Button;
    QPushButton *run_Button;
    QPushButton *log_button;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(648, 385);
        MainWindow->setMaximumSize(QSize(648, 385));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayoutWidget = new QWidget(centralwidget);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(10, 50, 331, 51));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        priority = new QTextEdit(gridLayoutWidget);
        priority->setObjectName("priority");
        priority->setMouseTracking(false);
        priority->setFocusPolicy(Qt::NoFocus);
        priority->setReadOnly(true);

        gridLayout->addWidget(priority, 3, 2, 1, 1);

        name = new QTextEdit(gridLayoutWidget);
        name->setObjectName("name");
        name->setFocusPolicy(Qt::NoFocus);
        name->setReadOnly(true);

        gridLayout->addWidget(name, 3, 0, 1, 1);

        id = new QTextEdit(gridLayoutWidget);
        id->setObjectName("id");
        id->setFocusPolicy(Qt::NoFocus);
        id->setReadOnly(true);

        gridLayout->addWidget(id, 3, 1, 1, 1);

        Process_Name_2 = new QLabel(gridLayoutWidget);
        Process_Name_2->setObjectName("Process_Name_2");

        gridLayout->addWidget(Process_Name_2, 0, 1, 2, 1);

        Process_Name = new QLabel(gridLayoutWidget);
        Process_Name->setObjectName("Process_Name");

        gridLayout->addWidget(Process_Name, 0, 0, 2, 1);

        Process_Name_3 = new QLabel(gridLayoutWidget);
        Process_Name_3->setObjectName("Process_Name_3");

        gridLayout->addWidget(Process_Name_3, 0, 2, 2, 1);

        API_text = new QTreeWidget(centralwidget);
        API_text->setObjectName("API_text");
        API_text->setGeometry(QRect(10, 110, 331, 391));
        verticalLayoutWidget_2 = new QWidget(centralwidget);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(350, 50, 291, 311));
        gridLayout_2 = new QGridLayout(verticalLayoutWidget_2);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        Modules_Text = new QTreeWidget(verticalLayoutWidget_2);
        Modules_Text->setObjectName("Modules_Text");
        Modules_Text->setLineWidth(1);
        Modules_Text->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        Modules_Text->setAutoScroll(true);
        Modules_Text->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        Modules_Text->setSortingEnabled(false);
        Modules_Text->setAllColumnsShowFocus(true);

        gridLayout_2->addWidget(Modules_Text, 0, 0, 1, 2);

        Error_Clear = new QPushButton(verticalLayoutWidget_2);
        Error_Clear->setObjectName("Error_Clear");

        gridLayout_2->addWidget(Error_Clear, 6, 0, 1, 1);

        Warning_Clear = new QPushButton(verticalLayoutWidget_2);
        Warning_Clear->setObjectName("Warning_Clear");

        gridLayout_2->addWidget(Warning_Clear, 6, 1, 1, 1);

        Info_text = new QTreeWidget(verticalLayoutWidget_2);
        Info_text->headerItem()->setText(1, QString());
        Info_text->setObjectName("Info_text");
        Info_text->setMouseTracking(true);
        Info_text->setFocusPolicy(Qt::ClickFocus);
        Info_text->setLineWidth(1);
        Info_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        Info_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        Info_text->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
        Info_text->setDragEnabled(false);
        Info_text->setSelectionBehavior(QAbstractItemView::SelectItems);
        Info_text->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        Info_text->setItemsExpandable(false);
        Info_text->header()->setCascadingSectionResizes(false);
        Info_text->header()->setDefaultSectionSize(100);
        Info_text->header()->setHighlightSections(false);
        Info_text->header()->setProperty("showSortIndicator", QVariant(false));

        gridLayout_2->addWidget(Info_text, 2, 0, 1, 2);

        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 10, 631, 31));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(layoutWidget);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        Filename_text = new QTextEdit(layoutWidget);
        Filename_text->setObjectName("Filename_text");
        Filename_text->setFocusPolicy(Qt::ClickFocus);
        Filename_text->setUndoRedoEnabled(true);
        Filename_text->setReadOnly(false);

        horizontalLayout->addWidget(Filename_text);

        selectFile_Button = new QPushButton(layoutWidget);
        selectFile_Button->setObjectName("selectFile_Button");

        horizontalLayout->addWidget(selectFile_Button);

        run_Button = new QPushButton(layoutWidget);
        run_Button->setObjectName("run_Button");

        horizontalLayout->addWidget(run_Button);

        log_button = new QPushButton(layoutWidget);
        log_button->setObjectName("log_button");

        horizontalLayout->addWidget(log_button);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        priority->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Microsoft YaHei UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">priority</p></body></html>", nullptr));
        name->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Microsoft YaHei UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">name</p></body></html>", nullptr));
        id->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Microsoft YaHei UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">id</p></body></html>", nullptr));
        Process_Name_2->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">Process ID</p></body></html>", nullptr));
        Process_Name->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">Process Name</p></body></html>", nullptr));
        Process_Name_3->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">Priority</p></body></html>", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = API_text->headerItem();
        ___qtreewidgetitem->setText(1, QCoreApplication::translate("MainWindow", "Value", nullptr));
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("MainWindow", "Name", nullptr));
        QTreeWidgetItem *___qtreewidgetitem1 = Modules_Text->headerItem();
        ___qtreewidgetitem1->setText(0, QCoreApplication::translate("MainWindow", "Modules", nullptr));
        Error_Clear->setText(QCoreApplication::translate("MainWindow", "Clear Erro Info", nullptr));
        Warning_Clear->setText(QCoreApplication::translate("MainWindow", "Clear Warning Info", nullptr));
        QTreeWidgetItem *___qtreewidgetitem2 = Info_text->headerItem();
        ___qtreewidgetitem2->setText(0, QCoreApplication::translate("MainWindow", "Info", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Filename:", nullptr));
        selectFile_Button->setText(QCoreApplication::translate("MainWindow", "Open File", nullptr));
        run_Button->setText(QCoreApplication::translate("MainWindow", "Run", nullptr));
        log_button->setText(QCoreApplication::translate("MainWindow", "Log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
