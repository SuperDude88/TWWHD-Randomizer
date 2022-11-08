#include "randomizer_thread.hpp"

#include <iostream>

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
    int retVal = mainRandomize();

    if (retVal != 0)
    {
        emit errorUpdate(ErrorLog::getInstance().getLastErrors());
    }
}
