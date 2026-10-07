#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::initConnections(void) {
#define CONNECT_NUM(num, widget) \
    connect(ui->widget, &QPushButton::clicked, this, &MainWindow::onNumberClicked);

    NUMPAD_WIDGETS(CONNECT_NUM)
#undef CONNECT_NUM

#define CONNECT_BINARY_OP(symbol, widget) \
    connect(ui->widget, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);

    BINARY_OPS(CONNECT_BINARY_OP)
#undef CONNECT_BINARY_OP

#define CONNECT_UNARY_OP(symbol, widget) \
    connect(ui->widget, &QPushButton::clicked, this, &MainWindow::onUnaryOperatorClicked);

    UNARY_OPS(CONNECT_UNARY_OP)
#undef CONNECT_UNARY_OP

#define CONNECT_SPECIAL(widget, handler) \
    connect(ui->widget, &QPushButton::clicked, this, &MainWindow::handler);

    SPECIALS(CONNECT_SPECIAL)
#undef CONNECT_SPECIAL

    ui->currExpr_LE->installEventFilter(this);
}

void MainWindow::initStyles(void) {
    setStyleSheet(
        QStringLiteral(
            "QPushButton {"
                "background-color: %1;"
                "color: white;"
                "border-radius: 10px;"
            "}"
            "QPushButton::pressed {"
                "background-color: gray;"
            "}"
            "QLineEdit {"
                "background-color: %2;"
                "color: white;"
                "border: 2px solid %3;"
            "}"
            "QFrame {"
                "background-color: black;"
                "border: 2px solid %3;"
            "}"
        ).arg(m_buttonColor, m_bgColor, m_borderColor)
    );


    QList<QPushButton*> operationButtonList = {
#define LIST_UNARY_OP_WIDGETS(symbol, widget) ui->widget,
        UNARY_OPS(LIST_UNARY_OP_WIDGETS)
#undef LIST_UNARY_OP_WIDGETS

#define LIST_BINARY_OP_WIDGETS(symbol, widget) ui->widget,
            BINARY_OPS(LIST_BINARY_OP_WIDGETS)
#undef LIST_BINARY_OP_WIDGETS
    };

    for(auto PB : operationButtonList) {
        PB->setStyleSheet(
            QStringLiteral(
                "QPushButton {"
                    "background-color: rgb(64, 59, 43);"
                "}"
            )
        );
    }

    QList<QPushButton*> specialButtonList = {
#define LIST_SPECIAL_WIDGETS(widget, handler) ui->widget,
        SPECIALS(LIST_SPECIAL_WIDGETS)
#undef LIST_SPECIAL_WIDGETS
    };

    for(auto PB : specialButtonList) {
        PB->setStyleSheet(
            QStringLiteral(
                "QPushButton {"
                    "background-color: %1;"
                "}"
            ).arg(m_bgColor)
        );
    }

    ui->equals_PB->setStyleSheet(
        QStringLiteral(
            "QPushButton {"
                "background-color: rgb(120, 110, 80);"
            "}"
        )
    );
}

void MainWindow::initLookupTables(void) {
#define BUILD_NUMBER_MAP(num, widget) \
    m_digitButtons[ui->widget] = num;

    NUMPAD_WIDGETS(BUILD_NUMBER_MAP)
#undef BUILD_NUMBER_MAP

#define BUILD_BINARY_MAPS(symbol, widget) \
    m_binaryOpButtons[ui->widget] = symbol; \

    BINARY_OPS(BUILD_BINARY_MAPS)
#undef BUILD_BINARY_MAPS

#define BUILD_UNARY_MAPS(symbol, widget) \
    m_unaryOpButtons[ui->widget] = symbol; \

    UNARY_OPS(BUILD_UNARY_MAPS)
#undef BUILD_UNARY_MAPS
}
