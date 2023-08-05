#include "randomizer_thread.hpp"

#include <iostream>
#include <algorithm>
#include <QDirIterator>

#include <randomizer.hpp>
#include <command/Log.hpp>
#include <gui/update_dialog_header.hpp>

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
    }
}

namespace TheMainThread {
    RandomizerThread* mainThread = nullptr;
}