#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_digitButtons.reserve(10);
    m_binaryOpButtons.reserve(4);
    m_unaryOpButtons.reserve(2);

    initStyles();
    initConnections();
    initLookupTables();

    updateDisplays();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if(watched == ui->currExpr_LE && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEv = static_cast<QKeyEvent*>(event);
        if(!keyEv) {
            return QObject::eventFilter(watched, event);
        }

        QChar key = keyEv->text().isEmpty() ? QChar() : keyEv->text().at(0);
        if(keyEv->modifiers() & Qt::ControlModifier) {
            switch(keyEv->key()) {
                case Qt::Key_Minus:
                    ui->plusminus_PB->click();
                    return true;
                default: break;
            }
        }

        static const QHash<int, std::function<void()>> keyHandlers = {
#define KEY_CASE(key, widget) \
        {Qt::Key_##key, [this]{ ui->widget##_PB->click(); }},
            KEY_CASE(Return, equals)
            KEY_CASE(Enter, equals)

            KEY_CASE(Backspace, del)

            KEY_CASE(Escape, clear)
            KEY_CASE(ParenLeft, bra)
            KEY_CASE(ParenRight, ket)

            KEY_CASE(Plus, plus)
            KEY_CASE(Minus, minus)
            KEY_CASE(Asterisk, mult)
            KEY_CASE(Slash, div)

            KEY_CASE(Period, dot)
            KEY_CASE(Comma, dot)
#undef KEY_CASE
        };

        if(auto handler = keyHandlers.find(keyEv->key()); handler != keyHandlers.end()) {
            handler.value()();
            return true;
        }

        if(key.isDigit()) {
            int digit = key.digitValue();
            auto it = std::find_if(m_digitButtons.cbegin(), m_digitButtons.cend(),
                                   [digit](int value){ return value == digit; });
            if(it != m_digitButtons.cend()) {
                it.key()->click();
                return true;
            }
        }

        if(!key.isDigit()) {
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void MainWindow::showErr(const QString& msg) {
    m_hasErr = true;
    ui->prevExpr_LE->setText(msg.isEmpty() ? m_errStr : msg);
    updateDisplays();
}

void MainWindow::updateDisplays(void) {
    if(m_hasErr) {
        ui->currExpr_LE->setText(m_errStr);
        return;
    }

    const QString expression = m_exprEval.getCurrentValue();
    if(expression.isEmpty()) {
        ui->currExpr_LE->setText(QStringLiteral("0"));
    } else {
        ui->currExpr_LE->setText(expression);
    }
}
