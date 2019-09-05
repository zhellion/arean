/********************************************************************************
** Form generated from reading UI file 'arean.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AREAN_H
#define UI_AREAN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_areanClass
{
public:
    QWidget *centralWidget;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLineEdit *lineEdit_4;
    QLineEdit *lineEdit_5;
    QLineEdit *lineEdit_6;
    QLineEdit *lineEdit_7;
    QLineEdit *lineEdit_8;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_12;
    QLabel *label_13;
    QLabel *label_14;
    QLabel *label_15;
    QLabel *label_16;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QLabel *label_17;
    QWidget *widget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *areanClass)
    {
        if (areanClass->objectName().isEmpty())
            areanClass->setObjectName(QString::fromUtf8("areanClass"));
        areanClass->resize(1369, 856);
        QPalette palette;
        QBrush brush(QColor(170, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush1(QColor(170, 0, 0, 128));
        brush1.setStyle(Qt::SolidPattern);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Active, QPalette::PlaceholderText, brush1);
#endif
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Inactive, QPalette::PlaceholderText, brush1);
#endif
        QBrush brush2(QColor(120, 120, 120, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush2);
        QBrush brush3(QColor(0, 0, 0, 128));
        brush3.setStyle(Qt::SolidPattern);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setBrush(QPalette::Disabled, QPalette::PlaceholderText, brush3);
#endif
        areanClass->setPalette(palette);
        centralWidget = new QWidget(areanClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(1080, 20, 47, 13));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(1080, 50, 47, 17));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(1080, 80, 47, 17));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(1020, 130, 291, 16));
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(1020, 110, 91, 16));
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(1080, 0, 91, 16));
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(1080, 40, 91, 10));
        label_8 = new QLabel(centralWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(1080, 60, 91, 20));
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setEnabled(true);
        lineEdit->setGeometry(QRect(1020, 180, 331, 20));
        lineEdit_2 = new QLineEdit(centralWidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setEnabled(true);
        lineEdit_2->setGeometry(QRect(1020, 230, 331, 20));
        lineEdit_3 = new QLineEdit(centralWidget);
        lineEdit_3->setObjectName(QString::fromUtf8("lineEdit_3"));
        lineEdit_3->setEnabled(true);
        lineEdit_3->setGeometry(QRect(1020, 280, 331, 20));
        lineEdit_4 = new QLineEdit(centralWidget);
        lineEdit_4->setObjectName(QString::fromUtf8("lineEdit_4"));
        lineEdit_4->setEnabled(true);
        lineEdit_4->setGeometry(QRect(1020, 330, 331, 20));
        lineEdit_5 = new QLineEdit(centralWidget);
        lineEdit_5->setObjectName(QString::fromUtf8("lineEdit_5"));
        lineEdit_5->setEnabled(true);
        lineEdit_5->setGeometry(QRect(1020, 370, 331, 21));
        lineEdit_6 = new QLineEdit(centralWidget);
        lineEdit_6->setObjectName(QString::fromUtf8("lineEdit_6"));
        lineEdit_6->setEnabled(true);
        lineEdit_6->setGeometry(QRect(1020, 420, 331, 20));
        lineEdit_7 = new QLineEdit(centralWidget);
        lineEdit_7->setObjectName(QString::fromUtf8("lineEdit_7"));
        lineEdit_7->setEnabled(true);
        lineEdit_7->setGeometry(QRect(1020, 460, 331, 20));
        lineEdit_8 = new QLineEdit(centralWidget);
        lineEdit_8->setObjectName(QString::fromUtf8("lineEdit_8"));
        lineEdit_8->setEnabled(true);
        lineEdit_8->setGeometry(QRect(1020, 500, 331, 20));
        label_9 = new QLabel(centralWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(1020, 160, 47, 17));
        label_10 = new QLabel(centralWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(1020, 210, 171, 17));
        label_11 = new QLabel(centralWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(1020, 260, 81, 17));
        label_12 = new QLabel(centralWidget);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(1020, 310, 111, 17));
        label_13 = new QLabel(centralWidget);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(1020, 350, 131, 17));
        label_14 = new QLabel(centralWidget);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(1020, 400, 101, 17));
        label_15 = new QLabel(centralWidget);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(1020, 440, 121, 17));
        label_16 = new QLabel(centralWidget);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(1020, 480, 241, 17));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(1020, 540, 141, 41));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(1020, 590, 141, 41));
        pushButton_3 = new QPushButton(centralWidget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(1170, 590, 151, 41));
        label_17 = new QLabel(centralWidget);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setGeometry(QRect(1060, 160, 291, 17));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(0, 0, 1011, 841));
        areanClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(areanClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1369, 21));
        areanClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(areanClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        areanClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(areanClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        areanClass->setStatusBar(statusBar);

        retranslateUi(areanClass);

        QMetaObject::connectSlotsByName(areanClass);
    } // setupUi

    void retranslateUi(QMainWindow *areanClass)
    {
        areanClass->setWindowTitle(QApplication::translate("areanClass", "arean", nullptr));
        label->setText(QApplication::translate("areanClass", "TextLabel", nullptr));
        label_2->setText(QApplication::translate("areanClass", "TextLabel", nullptr));
        label_3->setText(QApplication::translate("areanClass", "TextLabel", nullptr));
        label_4->setText(QApplication::translate("areanClass", "TextLabel", nullptr));
        label_5->setText(QApplication::translate("areanClass", "FrameTime", nullptr));
        label_6->setText(QApplication::translate("areanClass", "X", nullptr));
        label_7->setText(QApplication::translate("areanClass", "Y", nullptr));
        label_8->setText(QApplication::translate("areanClass", "Z", nullptr));
        label_9->setText(QApplication::translate("areanClass", "\320\222\321\200\320\265\320\274\321\217", nullptr));
        label_10->setText(QApplication::translate("areanClass", "\320\224\320\276\320\273\320\263\320\276\321\202\320\260 \320\262\320\276\321\201\321\205\320\276\320\264\321\217\321\211\320\265\320\263\320\276 \321\203\320\267\320\273\320\260", nullptr));
        label_11->setText(QApplication::translate("areanClass", "\320\235\320\260\320\272\320\273\320\276\320\275\320\265\320\275\320\270\320\265", nullptr));
        label_12->setText(QApplication::translate("areanClass", "\320\220\321\200\320\263\321\203\320\274\320\265\320\275\321\202 \320\277\320\265\321\200\320\270\321\206\320\265\320\275\321\202\321\200\320\260", nullptr));
        label_13->setText(QApplication::translate("areanClass", "\320\221\320\276\320\273\321\214\321\210\320\260\321\217 \320\277\320\276\320\273\321\203\320\276\321\201\321\214", nullptr));
        label_14->setText(QApplication::translate("areanClass", "\320\255\320\272\321\201\321\202\321\200\320\265\321\201\320\270\321\202\320\265\321\202", nullptr));
        label_15->setText(QApplication::translate("areanClass", "\320\241\321\200\320\265\320\264\320\275\321\217\321\217 \320\260\320\275\320\276\320\274\320\260\320\273\320\270\321\217", nullptr));
        label_16->setText(QApplication::translate("areanClass", "\320\223\320\265\320\276\321\206\320\265\320\275\321\202\321\200\320\270\321\207\320\265\321\201\320\272\320\260\321\217 \320\263\321\200\320\260\320\262\320\270\321\202\320\260\321\206\320\270\320\276\320\275\320\275\320\260\321\217 \320\277\320\276\321\201\321\202\320\276\321\217\320\275\320\275\320\260\321\217", nullptr));
        pushButton->setText(QApplication::translate("areanClass", "\320\237\320\276\320\273\321\203\321\207\320\270\321\202\321\214 \320\267\320\275\320\260\321\207\320\265\320\275\320\270\321\217", nullptr));
        pushButton_2->setText(QApplication::translate("areanClass", "\320\227\320\260\320\264\320\260\321\202\321\214 \320\267\320\275\320\260\321\207\320\265\320\275\320\270\321\217", nullptr));
        pushButton_3->setText(QApplication::translate("areanClass", "\320\270\320\267\320\274\320\265\320\275\320\270\321\202\321\214 \320\267\320\275\320\260\321\207\320\265\320\275\320\270\321\217", nullptr));
        label_17->setText(QApplication::translate("areanClass", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class areanClass: public Ui_areanClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AREAN_H
