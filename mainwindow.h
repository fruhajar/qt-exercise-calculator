#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QHash>

#include "expressionevaluator.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

//  Define numpad widget references
#define NUMPAD_WIDGETS(X) \
    X(0, PB_0) \
    X(1, PB_1) \
    X(2, PB_2) \
    X(3, PB_3) \
    X(4, PB_4) \
    X(5, PB_5) \
    X(6, PB_6) \
    X(7, PB_7) \
    X(8, PB_8) \
    X(9, PB_9)

//  Define applicable binary operators
#define BINARY_OPS(X) \
    X("+", plus_PB) \
    X("-", minus_PB) \
    X("*", mult_PB) \
    X("/", div_PB)

//  Define applicable unary operators
//  May cause issues in portability
#define UNARY_OPS(X) \
    X("√", sqrt_PB) \
    X("sqr", square_PB)

//  Define special widgets
#define SPECIALS(X) \
    X(dot_PB, onDecimalClicked) \
    X(equals_PB, onEqualsClicked) \
    X(del_PB, onDeleteClicked) \
    X(clear_PB, onClearClicked) \
    X(softClear_PB, onSoftClearClicked) \
    X(bra_PB, onBraClicked) \
    X(ket_PB, onKetClicked)  \
    X(plusminus_PB, onPlusMinusClicked)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onNumberClicked(void);

    void onOperatorClicked(void);
    void onUnaryOperatorClicked(void);

    void onEqualsClicked(void);
    void onClearClicked(void);
    void onSoftClearClicked(void);
    void onDecimalClicked(void);
    void onDeleteClicked(void);
    void onPlusMinusClicked(void);

    void onBraClicked(void);
    void onKetClicked(void);

private:
    Ui::MainWindow *ui;
    ExpressionEvaluator m_exprEval;

    bool m_waitingForOperand = true;
    bool m_hasErr = false;

    QHash<QPushButton*, int> m_digitButtons;
    QHash<QPushButton*, QString> m_binaryOpButtons;
    QHash<QPushButton*, QString> m_unaryOpButtons;

    inline static const QString m_errStr = QStringLiteral("Error");
    inline static const QString m_borderColor = QStringLiteral("rgb(177, 186, 205)");
    inline static const QString m_bgColor = QStringLiteral("rgb(18, 20, 28)");
    inline static const QString m_buttonColor = QStringLiteral("rgb(43, 46, 63)");

    /*!
     * \brief Fill both lineEdits with error strings
     */
    void showErr(const QString& msg);

    //  Drop a stale error so the next keystroke starts clean
    void clearError(void);
    //  Drop a finished equals result so the next keystroke starts a new entry
    void discardResult(void);
    //  Both of the above, for handlers that begin a fresh operand
    void beginNewEntry(void);

    /*!
     * \brief Prepare widget stylesheets
     */
    void initStyles(void);
    /*!
     * \brief Connect relevant widgets
     */
    void initConnections(void);
    /*!
     * \brief Prepare tables for easy symbol lookups
     */
    void initLookupTables(void);

    //  Update currExpr lineEdit with the current expression
    void updateDisplays(void);
};
#endif // MAINWINDOW_H
