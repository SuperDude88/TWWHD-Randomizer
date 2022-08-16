#include "randomizer_thread.hpp"
#include "../randomizer.hpp"
#include "../server/command/Log.hpp"

#include <iostream>

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
