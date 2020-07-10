#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;


struct Email {
    Email() {}
    Email(string from_, string to_, string body_) : from(move(from_)), to(move(to_)), body(move(body_)) {}

    string from;
    string to;
    string body;
};


class Worker {
public:
    virtual ~Worker() = default;

    virtual void Process(unique_ptr<Email> email) = 0;

    virtual void Run() {
        // только первому worker-у в пайплайне нужно это имплементировать
        throw logic_error("Unimplemented");
    }

protected:
    // реализации должны вызывать PassOn, чтобы передать объект дальше
    // по цепочке обработчиков
    void PassOn(unique_ptr<Email> email) const {
        if (next_worker) {
            next_worker->Process(move(email));
        }
    }

public:
    void SetNext(unique_ptr<Worker> next) {
        next_worker = move(next);
    }

private:
    unique_ptr<Worker> next_worker;
};


class Reader : public Worker {
public:
    Reader(istream &in) : in_(in) {}

    void Process(unique_ptr<Email> email) override {
        PassOn(move(email));
    }

    void Run() {
        string from, to, body;
        while (getline(in_, from) && getline(in_, to) && getline(in_, body)) {
            Process(make_unique<Email>(
                    move(from), move(to), move(body)
            ));
        }
    }

private:
    istream &in_;
};


class Filter : public Worker {
public:
    using Function = function<bool(const Email &)>;

public:
    explicit Filter(Function func_) : func(move(func_)){}

    void Process(unique_ptr<Email> email) override {
        if (func(*email)) {
            PassOn(move(email));
        }
    }

private:
    Function func;
};


class Copier : public Worker {
public:
    explicit Copier(string copy_to_) : copy_to(move(copy_to_)) {}

    void Process(unique_ptr<Email> email) override {
        if (email->to != copy_to) {
            auto email_copy = make_unique<Email>(*email);
            email_copy->to = copy_to;
            PassOn(move(email));
            PassOn(move(email_copy));
        } else {
            PassOn(move(email));
        }
    }
private:
    string copy_to;

};


class Sender : public Worker {
public:
    explicit Sender(ostream& out_) : out(out_) {}

    void Process(unique_ptr<Email> email) override {
        out << email->from << '\n';
        out << email->to << '\n';
        out << email->body << '\n';

        PassOn(move(email));
    }

private:
    ostream& out;
};


// реализуйте класс
class PipelineBuilder {
public:
    // добавляет в качестве первого обработчика Reader
    explicit PipelineBuilder(istream &in) {
        nodes.push_back(make_unique<Reader>(in));
    }

    // добавляет новый обработчик Filter
    PipelineBuilder &FilterBy(Filter::Function filter) {
        nodes.push_back(make_unique<Filter>(move(filter)));
        return *this;
    }

    // добавляет новый обработчик Copier
    PipelineBuilder &CopyTo(string recipient) {
        nodes.push_back(make_unique<Copier>(move(recipient)));
        return *this;
    }

    // добавляет новый обработчик Sender
    PipelineBuilder &Send(ostream &out) {
        nodes.push_back(make_unique<Sender>(out));
        return *this;
    }

    // возвращает готовую цепочку обработчиков
    unique_ptr<Worker> Build() {
        for (auto it_latter = nodes.rbegin(), it_prev = next(it_latter); it_prev != nodes.rend(); it_prev++, it_latter++) {
            (*it_prev)->SetNext(move(*it_latter));
        }
        return move(nodes[0]);
    }

private:
    vector<unique_ptr<Worker>> nodes;
};


void TestSanity() {
    string input = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "ralph@example.com\n"
            "erich@example.com\n"
            "I do not make mistakes of that kind\n"
    );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email &email) {
        return email.from == "erich@example.com";
    });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = (
            "erich@example.com\n"
            "richard@example.com\n"
            "Hello there\n"

            "erich@example.com\n"
            "ralph@example.com\n"
            "Are you sure you pressed the right button?\n"

            "erich@example.com\n"
            "richard@example.com\n"
            "Are you sure you pressed the right button?\n"
    );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    return 0;
}
