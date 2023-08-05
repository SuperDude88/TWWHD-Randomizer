#pragma once

#include <QThread>

class RandomizerThread : public QThread
{
    Q_OBJECT
public:
    RandomizerThread();
    ~RandomizerThread();

    void run() override;

signals:
    void dialogValueUpdate(int val);
    void dialogLabelUpdate(const QString& label);
    void errorUpdate(const std::string& msg, const std::string& title = "An error has occured!");
};

namespace TheMainThread {
    extern RandomizerThread* mainThread;
}