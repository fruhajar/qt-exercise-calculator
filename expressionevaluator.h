#ifndef EXPRESSIONEVALUATOR_H
#define EXPRESSIONEVALUATOR_H

#include <QRegularExpression>
#include <QStringView>
#include <QString>
#include <QStack>
#include <QHash>
#include <QDebug>

#include <functional>
#include <optional>
#include <variant>
#include <vector>
#include <cmath>

class ExpressionEvaluator {
public:
    ExpressionEvaluator() {
        //  Setup precedence
        m_precedence[QStringLiteral("+")] = 1;
        m_precedence[QStringLiteral("-")] = 1;
        m_precedence[QStringLiteral("*")] = 2;
        m_precedence[QStringLiteral("/")] = 2;
        m_precedence[QStringLiteral("sqrt")] = 4;
        m_precedence[QStringLiteral("sqr")] = 4;
        m_precedence[QStringLiteral("neg")] = 4;

        //  Setup unary functions
        m_unaryFuncs[QStringLiteral("sqrt")] = [](double x) -> std::optional<double> {
            return x >= 0.0 ? std::make_optional(std::sqrt(x)) : std::nullopt;
        };
        m_unaryFuncs[QStringLiteral("sqr")] = [](double x) -> std::optional<double> {
            return std::make_optional(x * x);
        };
        m_unaryFuncs[QStringLiteral("neg")] = [](double x) -> std::optional<double> {
            return std::make_optional(-x);
        };
    }

    void reset() {
        m_completeExpression.clear();
        m_currentValue.clear();
        m_hasEqualsResult = false;
        m_openParenth = 0;
    }

    //  Set the current display value
    void setCurrentValue(const QString& value) {
        m_currentValue = value;
        m_hasEqualsResult = false;
    }

    //  Get the current value for display
    QString getCurrentValue() const {
        return m_currentValue;
    }

    //  Get the complete expression for display
    QString getCompleteExpression() const {
        return m_completeExpression;
    }

    std::optional<double> evaluateExpressionString(const QString& expr) const {
        return evaluateExpression(expr);
    }

    //  Add a binary operator
    void addBinaryOperator(const QString& op) {
        if(!m_currentValue.isEmpty()) {
            commitCurrentValue();
        }

        //  Check expression completeness
        if(isExpressionIncomplete()) {
            const qsizetype lastOpPos = findLastOperatorPosition();
            if(lastOpPos != -1) {
                m_completeExpression = m_completeExpression.left(lastOpPos) + op + QStringLiteral(" ");
            }
        } else {
            if(!m_completeExpression.isEmpty()) {
                m_completeExpression += QStringLiteral(" ");
            }
            m_completeExpression += op + QStringLiteral(" ");
        }

        m_currentValue.clear();
        m_hasEqualsResult = false;
    }

    // Add a unary operator
    void addUnaryOperator(const QString& symbol) {
        const QString funcName = mapUnarySymbol(symbol);

        //  Case 1: After equals
        if(m_hasEqualsResult && !m_completeExpression.isEmpty()) {
            m_completeExpression = funcName + QStringLiteral("(") + m_completeExpression + QStringLiteral(")");
            m_hasEqualsResult = false;
            m_currentValue.clear();
            return;
        }

        //  Case 2: Expression is complete and valid
        if(!isExpressionIncomplete() && !m_completeExpression.isEmpty()) {
            applyUnaryToRightmostElement(funcName);
            m_currentValue.clear();
            return;
        }

        //  Case 3: Expression is incomplete or empty
        if(!m_currentValue.isEmpty()) {
            const QString unaryExpr = funcName + QStringLiteral("(") + m_currentValue + QStringLiteral(")");
            if(!m_completeExpression.isEmpty()) {
                m_completeExpression += unaryExpr;
            } else {
                m_completeExpression = unaryExpr;
            }
            m_currentValue.clear();
        } else {
            //  Apply to 0 if no current value
            const QString unaryExpr = funcName + QStringLiteral("(0)");
            if(!m_completeExpression.isEmpty()) {
                m_completeExpression += unaryExpr;
            } else {
                m_completeExpression = unaryExpr;
            }
        }

        m_hasEqualsResult = false;
    }

    //  Add parentheses
    void addBra() {
        if(!m_currentValue.isEmpty()) {
            commitCurrentValue();
        }

        if(!m_completeExpression.isEmpty() && !isExpressionIncomplete()) {
            m_completeExpression += QStringLiteral(" ");
        }
        m_completeExpression += QStringLiteral("(");
        m_openParenth++;
        m_hasEqualsResult = false;
    }

    void addKet() {
        if(!m_currentValue.isEmpty()) {
            commitCurrentValue();
        }

        if(m_openParenth > 0) {
            m_completeExpression += QStringLiteral(")");
            m_openParenth--;
        }

        m_currentValue.clear();
        m_hasEqualsResult = false;
    }

    //  Evaluate the complete expression
    std::optional<double> evaluate() {
        //  Commit current value if present
        if(!m_currentValue.isEmpty()) {
            commitCurrentValue();
        }

        if(m_completeExpression.isEmpty()) {
            return std::nullopt;
        }

        auto result = evaluateExpression(m_completeExpression);
        if(result) {
            m_hasEqualsResult = true;
            //  Update current value to show the result
            m_currentValue = QString::number(*result);
        }

        return result;
    }

    //  Check if we can evaluate
    bool canEvaluate() const {
        return !m_completeExpression.isEmpty() || !m_currentValue.isEmpty();
    }

    //  Apply unary function to a value
    std::optional<double> applyUnaryFunction(const QString& symbol, double value) const {
        const QString funcName = mapUnarySymbol(symbol);
        auto it = m_unaryFuncs.find(funcName);
        if(it != m_unaryFuncs.end()) {
            return it.value()(value);
        }
        return std::nullopt;
    }

    bool hasResult() const { return m_hasEqualsResult; }

private:
    QString m_completeExpression;  //  Complete expression
    QString m_currentValue;
    bool m_hasEqualsResult;
    int m_openParenth;

    QHash<QString, int> m_precedence;
    QHash<QString, std::function<std::optional<double>(double)>> m_unaryFuncs;

    //  Map UI symbols to internal function names
    QString mapUnarySymbol(const QString& symbol) const {
        if(symbol == QStringLiteral("√")) {
            return QStringLiteral("sqrt");
        }

        if(symbol == QStringLiteral("±")) {
            return QStringLiteral("neg");
        }
        return symbol;
    }

    //  Commit current value to the complete expression
    void commitCurrentValue() {
        if(m_currentValue.isEmpty()) return;

        if(!m_completeExpression.isEmpty() && !isExpressionIncomplete()) {
            m_completeExpression += " ";
        }
        m_completeExpression += m_currentValue;
    }

    //  Check if expression is incomplete
    bool isExpressionIncomplete() const {
        if(m_completeExpression.isEmpty()) return false;

        const QString trimmed = m_completeExpression.trimmed();
        if(trimmed.isEmpty()) return false;

        return trimmed.endsWith(QStringLiteral("+")) || trimmed.endsWith(QStringLiteral("-")) ||
               trimmed.endsWith(QStringLiteral("*")) || trimmed.endsWith(QStringLiteral("/")) ||
               trimmed.endsWith(QStringLiteral("("));
    }

    //  Find position of last binary operator
    qsizetype findLastOperatorPosition() const {
        const QString trimmed = m_completeExpression.trimmed();
        for(qsizetype i = trimmed.length() - 1; i >= 0; --i) {
            const QChar c = trimmed[i];
            if(c == '+' || c == '-' || c == '*' || c == '/') {
                return i;
            }
        }
        return -1;
    }

    //  Apply unary operator to the rightmost complete element
    void applyUnaryToRightmostElement(const QString& funcName) {
        const QString target = findRightmostElement();
        if(!target.isEmpty()) {
            const qsizetype pos = m_completeExpression.lastIndexOf(target);
            if(pos != -1) {
                const QString replacement = funcName + QStringLiteral("(") + target + QStringLiteral(")");
                m_completeExpression.replace(pos, target.length(), replacement);
            }
        }
    }

    //  Find the rightmost complete element in the expression
    QString findRightmostElement() const {
        const QString trimmed = m_completeExpression.trimmed();
        if(trimmed.isEmpty()) return QString();

        //  Handle parentheses
        if(trimmed.endsWith(')')) {
            int parenCount = 1;
            qsizetype i = trimmed.length() - 2;
            while(i >= 0 && parenCount > 0) {
                if(trimmed[i] == ')') parenCount++;
                else if(trimmed[i] == '(') parenCount--;
                i--;
            }
            if(parenCount == 0) {
                //  Check if there's a function name before the opening paren
                qsizetype funcStart = i + 1;
                while(funcStart > 0 && trimmed[funcStart - 1].isLetter()) {
                    funcStart--;
                }
                return trimmed.mid(funcStart);
            }
        }

        //  Handle numbers
        qsizetype i = trimmed.length() - 1;
        while(i >= 0 && trimmed[i].isSpace()) i--;

        if(i >= 0 && (trimmed[i].isDigit() || trimmed[i] == '.')) {
            const qsizetype end = i + 1;
            while(i >= 0 && (trimmed[i].isDigit() || trimmed[i] == '.')) i--;

            if(i >= 0 && trimmed[i] == '-') {
                if(i == 0 || trimmed[i-1] == '(' || trimmed[i-1] == '+' ||
                   trimmed[i-1] == '-' || trimmed[i-1] == '*' || trimmed[i-1] == '/') {
                    i--;
                }
            }
            return trimmed.mid(i + 1, end - i - 1);
        }

        return QString();
    }

    //  Token types and structures
    enum class TokenType {
        Number,
        BinaryOp,
        UnaryFunc,
        LeftParen,
        RightParen
    };

    struct Token {
        TokenType type;
        std::variant<double, QString> value;
    };

    //  Tokenize the expression string
    bool tokenize(const QString& expr, std::vector<Token>& tokens) const {
        tokens.clear();
        tokens.reserve(static_cast<size_t>(expr.length()) / 2);

        qsizetype i = 0;
        const qsizetype len = expr.length();

        while(i < len) {
            const QChar ch = expr[i];

            //  Skip whitespace
            if(ch.isSpace()) {
                i++;
                continue;
            }

            //  Check for unary functions
            if(ch.isLetter()) {
                const qsizetype start = i;
                while(i < len && expr[i].isLetter()) {
                    i++;
                }
                const QString func = expr.mid(start, i - start);

                if(m_unaryFuncs.contains(func)) {
                    tokens.push_back({TokenType::UnaryFunc, func});
                } else {
                    return false;
                }
                continue;
            }

            //  Numbers
            if(ch.isDigit() || (ch == u'-' && i + 1 < len &&
                (expr[i + 1].isDigit() || expr[i + 1] == u'.') &&
                (tokens.empty() || tokens.back().type == TokenType::LeftParen ||
                 tokens.back().type == TokenType::BinaryOp))) {

                const qsizetype start = i;
                if(ch == u'-') i++;

                bool hasDecimal = false;
                while(i < len && (expr[i].isDigit() || (expr[i] == u'.' && !hasDecimal))) {
                    if(expr[i] == u'.') hasDecimal = true;
                    i++;
                }

                bool ok;
                const double value = expr.mid(start, i - start).toDouble(&ok);
                if(!ok) {
                    return false;
                }

                tokens.push_back({TokenType::Number, value});
                continue;
            }

            //  Single character tokens
            switch(ch.unicode()) {
                case u'+':
                case u'-':
                case u'*':
                case u'/':
                    tokens.push_back({TokenType::BinaryOp, QString(ch)});
                    break;
                case u'(':
                    tokens.push_back({TokenType::LeftParen, QString()});
                    break;
                case u')':
                    tokens.push_back({TokenType::RightParen, QString()});
                    break;
                default:
                    return false; // Unknown character
            }
            i++;
        }

        return true;
    }

    //  Get operator precedence
    int getPrecedence(const Token& token) const {
        if(token.type == TokenType::BinaryOp || token.type == TokenType::UnaryFunc) {
            return m_precedence.value(std::get<QString>(token.value), 0);
        }
        return 0;
    }

    //  Process a single operator during evaluation
    bool processOperator(QStack<double>& values, QStack<Token>& operators) const {
        if(operators.isEmpty()) {
            return false;
        }

        const Token op = operators.pop();

        if(op.type == TokenType::UnaryFunc) {
            if(values.isEmpty()) {
                return false;
            }

            const double arg = values.pop();
            const QString& funcName = std::get<QString>(op.value);

            auto it = m_unaryFuncs.find(funcName);
            if(it == m_unaryFuncs.end()) {
                return false;
            }

            const auto result = it.value()(arg);
            if(!result) {
                return false;
            }

            values.push(*result);
            return true;
        }

        if(op.type == TokenType::BinaryOp) {
            if(values.size() < 2) {
                return false;
            }

            const double right = values.pop();
            const double left = values.pop();
            const QString& opStr = std::get<QString>(op.value);

            std::optional<double> result;
            if(opStr == u"+") {
                result = left + right;
            } else if(opStr == u"-") {
                result = left - right;
            } else if(opStr == u"*") {
                result = left * right;
            } else if(opStr == u"/") {
                static constexpr double EPSILON = 1e-32;
                if(std::abs(right) < EPSILON) {
                    return false;
                }
                result = left / right;
            }

            if(!result) {
                return false;
            }
            values.push(*result);
            return true;
        }

        return false;
    }

    //  Evaluate expression using shunting-yard algorithm
    std::optional<double> evaluateExpression(const QString& expr) const {
        if(expr.isEmpty()) {
            return std::nullopt;
        }

        std::vector<Token> tokens;
        if(!tokenize(expr, tokens)) {
            return std::nullopt;
        }


        QStack<double> values;
        QStack<Token> operators;

        for(const auto& token : tokens) {
            switch(token.type) {
                case TokenType::Number:
                    values.push(std::get<double>(token.value));
                    break;

                case TokenType::UnaryFunc:
                    operators.push(token);
                    break;

                case TokenType::BinaryOp: {
                    const auto& opStr = std::get<QString>(token.value);
                    while(!operators.isEmpty() &&
                           operators.top().type != TokenType::LeftParen &&
                           getPrecedence(operators.top()) >= getPrecedence(token)) {
                        if(!processOperator(values, operators)) {
                            return std::nullopt;
                        }
                    }
                    operators.push(token);
                    break;
                }

                case TokenType::LeftParen:
                    operators.push(token);
                    break;

                case TokenType::RightParen:
                    while(!operators.isEmpty() &&
                           operators.top().type != TokenType::LeftParen) {
                        if(!processOperator(values, operators)) {
                            return std::nullopt;
                        }
                    }
                    if(!operators.isEmpty()) {
                        operators.pop();

                        //  Check if there's a unary function waiting
                        if(!operators.isEmpty() &&
                            operators.top().type == TokenType::UnaryFunc) {
                            if(!processOperator(values, operators)) {
                                return std::nullopt;
                            }
                        }
                    }
                    break;
            }
        }

        //  Process remaining operators
        while(!operators.isEmpty()) {
            if(!processOperator(values, operators)) {
                return std::nullopt;
            }
        }

        auto result = values.size() == 1 ? std::make_optional(values.top()) : std::nullopt;
        return result;
    }
};

#endif // EXPRESSIONEVALUATOR_H
