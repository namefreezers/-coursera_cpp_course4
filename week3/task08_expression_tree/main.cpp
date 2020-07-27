#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"

#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

string Print(const Expression *e) {
    if (!e) {
        return "Null expression provided";
    }
    stringstream output;
    output << e->ToString() << " = " << e->Evaluate();
    return output.str();
}

class ValueExpression : public Expression {
public:
    explicit ValueExpression(int value_);

    int Evaluate() const override;

    std::string ToString() const override;

private:
    int value;
};

ValueExpression::ValueExpression(int value_) : value(value_) {}

int ValueExpression::Evaluate() const {
    return value;
}

string ValueExpression::ToString() const {
    return to_string(value);
}

class BinaryExpression : public Expression {
public:
    explicit BinaryExpression(ExpressionPtr left_, ExpressionPtr right_) : left(move(left_)), right(move(right_)) {}

    std::string ToString() const override {
        return "(" + left->ToString() + ")" + SignToString() + "(" + right->ToString() + ")";
    }

private:
    virtual std::string SignToString() const = 0;

protected:
    ExpressionPtr left, right;
};

class SumExpression : public BinaryExpression {
public:
    using BinaryExpression::BinaryExpression;

    int Evaluate() const override {
        return left->Evaluate() + right->Evaluate();
    }

private:
    std::string SignToString() const override {
        return "+";
    }
};

class ProductExpression : public BinaryExpression {
public:
    using BinaryExpression::BinaryExpression;

    int Evaluate() const override {
        return left->Evaluate() * right->Evaluate();
    }

private:
    std::string SignToString() const override {
        return "*";
    }
};

ExpressionPtr Value(int value) {
    return make_unique<ValueExpression>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<SumExpression>(move(left), move(right));
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<ProductExpression>(move(left), move(right));
}

void Test() {
    ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
    ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

    ExpressionPtr e2 = Sum(move(e1), Value(5));
    ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

    ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}

#pragma clang diagnostic pop