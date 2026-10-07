#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <typeinfo>

template<typename T>
T* getSenderWidget(QObject* sender) {
    auto* widget = qobject_cast<T*>(sender);
    if(!widget) {
        qWarning() << QStringLiteral("Failed to cast sender to") << typeid(T).name();
    }
    return widget;
}

void MainWindow::onNumberClicked(void) {
    auto* PB = getSenderWidget<QPushButton>(sender());
    if(!PB) {
        return;
    }

    auto it = m_digitButtons.find(PB);
    if(it == m_digitButtons.end()) {
        return;
    }

    int digit = it.value();

    beginNewEntry();

    //  Append digit to current value
    QString currentValue = m_exprEval.getCurrentValue();
    if(currentValue == QStringLiteral("0")) {
        currentValue = QString::number(digit);
    } else {
        currentValue += QString::number(digit);
    }

    m_exprEval.setCurrentValue(currentValue);
    ui->currExpr_LE->setText(currentValue);
    m_waitingForOperand = false;
}

void MainWindow::onOperatorClicked(void) {
    auto* PB = getSenderWidget<QPushButton>(sender());
    if(!PB || m_hasErr) {
        return;
    }

    auto it = m_binaryOpButtons.find(PB);
    if(it == m_binaryOpButtons.end()) {
        return;
    }

    const QString& operatorSym = it.value();

    m_exprEval.addBinaryOperator(operatorSym);

    //  Update displays
    ui->prevExpr_LE->setText(m_exprEval.getCompleteExpression());
    ui->currExpr_LE->clear();
    m_waitingForOperand = true;
}

void MainWindow::onUnaryOperatorClicked() {
    auto* PB = getSenderWidget<QPushButton>(sender());
    if(!PB || m_hasErr) return;

    auto it = m_unaryOpButtons.find(PB);
    if(it == m_unaryOpButtons.end()) return;

    const QString& operatorSym = it.value();

    // Get current value for calculation
    QString currentValue = m_exprEval.getCurrentValue();
    if(currentValue.isEmpty()) currentValue = QStringLiteral("0");

    bool ok;
    double value = currentValue.toDouble(&ok);
    if(!ok) {
        showErr(QStringLiteral("Invalid number"));
        return;
    }

    //  Calculate result for display
    std::optional<double> result = m_exprEval.applyUnaryFunction(operatorSym, value);
    if(!result) {
        showErr(QStringLiteral("Math error"));
        return;
    }

    //  Add unary operator to expression
    m_exprEval.addUnaryOperator(operatorSym);

    //  Update displays
    ui->prevExpr_LE->setText(m_exprEval.getCompleteExpression());
    QString resultStr = QString::number(*result);
    if(resultStr == QStringLiteral("0")) {
        result = m_exprEval.evaluateExpressionString(ui->prevExpr_LE->text());
        if(result) {
            resultStr = QString::number(*result);
        }
    }
    ui->currExpr_LE->setText(resultStr);

    m_waitingForOperand = true;
}

void MainWindow::onEqualsClicked() {
    if(m_hasErr) return;

    auto result = m_exprEval.evaluate();
    if(result) {
        QString fullExpr = m_exprEval.getCompleteExpression() + QStringLiteral(" =");
        ui->prevExpr_LE->setText(fullExpr);
        ui->currExpr_LE->setText(QString::number(*result));
        m_waitingForOperand = true;
    } else {
        showErr(QStringLiteral("Invalid expression"));
    }
}

void MainWindow::onDecimalClicked(void) {
    beginNewEntry();

    QString currentValue = m_exprEval.getCurrentValue();
    if(currentValue.isEmpty() || m_waitingForOperand) {
        currentValue = QStringLiteral("0.");
    } else if(!currentValue.contains('.')) {
        currentValue += '.';
    }

    m_exprEval.setCurrentValue(currentValue);
    ui->currExpr_LE->setText(currentValue);
    m_waitingForOperand = false;
}

void MainWindow::onBraClicked(void) {
    if(m_hasErr) return;

    m_exprEval.addBra();
    ui->prevExpr_LE->setText(m_exprEval.getCompleteExpression());
    ui->currExpr_LE->clear();
    m_waitingForOperand = true;
}

void MainWindow::onKetClicked(void) {
    if(m_hasErr) return;

    m_exprEval.addKet();
    ui->prevExpr_LE->setText(m_exprEval.getCompleteExpression());
    std::optional<double> result = m_exprEval.evaluateExpressionString(ui->prevExpr_LE->text());
    if(result) {
        ui->currExpr_LE->setText(QString::number(*result));
    }
    m_waitingForOperand = true;
}

void MainWindow::onClearClicked(void) {
    m_waitingForOperand = true;
    m_hasErr = false;
    m_exprEval.reset();
    ui->currExpr_LE->setText(QStringLiteral("0"));
    ui->prevExpr_LE->clear();
}

void MainWindow::onSoftClearClicked(void) {
    clearError();

    m_exprEval.setCurrentValue(QString());
    ui->currExpr_LE->setText(QStringLiteral("0"));
    m_waitingForOperand = true;
}

void MainWindow::onPlusMinusClicked(void) {
    if(m_hasErr) {
        return;
    }

    discardResult();

    QString currentValue = m_exprEval.getCurrentValue();
    if(currentValue.isEmpty() || currentValue == QStringLiteral("0")) {
        return;
    }

    bool ok;
    double val = currentValue.toDouble(&ok);
    if(!ok) {
        return;
    }

    val = -val;
    QString result = QString::number(val);

    //  Update both UI and ExpressionEvaluator state
    m_exprEval.setCurrentValue(result);
    ui->currExpr_LE->setText(result);

    m_waitingForOperand = false;
}

void MainWindow::onDeleteClicked(void) {
    if(m_hasErr) {
        return;
    }

    discardResult();

    QString currentValue = m_exprEval.getCurrentValue();

    if(currentValue.length() > 1) {
        currentValue.chop(1);
        if(currentValue == QStringLiteral("-") || currentValue.isEmpty()) {
            currentValue = QStringLiteral("0");
            m_waitingForOperand = true;
        }
    } else {
        currentValue = QStringLiteral("0");
        m_waitingForOperand = true;
    }

    //  Update both UI and evaluator state
    m_exprEval.setCurrentValue(currentValue == QStringLiteral("0") ?
                                   QString() : currentValue);
    ui->currExpr_LE->setText(currentValue);

    if(currentValue == QStringLiteral("0")) {
        m_waitingForOperand = true;
    }
}
