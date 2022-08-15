#include "randomizer.hpp"
#include "server/command/Log.hpp"
#include "server/utility/platform.hpp"

#include <iostream>

int main()
{
    int retVal = mainRandomize();

    if (retVal == 1)
    {
        auto message = "An error has occured!\n" + ErrorLog::getInstance().getLastErrors();
        Utility::platformLog(message.c_str());
    }

    return 0;
}
