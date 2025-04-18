#include "randomizer_thread.hpp"

#include <QDirIterator>

#include <randomizer.hpp>
#include <command/Log.hpp>

RandomizerThread::RandomizerThread()
{

}

RandomizerThread::~RandomizerThread()
{

}

void RandomizerThread::run()
{
    TheMainThread::mainThread = this;
    int retVal = mainRandomize();

    if (retVal != 0)
    {
        emit errorUpdate(ErrorLog::getInstance().getLastErrors());
        ErrorLog::getInstance().clearLastErrors();
    }
}

namespace TheMainThread {
    RandomizerThread* mainThread = nullptr;
}