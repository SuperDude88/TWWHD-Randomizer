#include "randomizer.hpp"
#include <command/Log.hpp>
#include <utility/platform.hpp>

int main()
{
    int retVal = mainRandomize();

    if (retVal == 1)
    {
        auto message = "An error has occured!\n" + ErrorLog::getInstance().getLastErrors();
        Utility::platformLog(message.c_str());
    }

    // Close logs
    ErrorLog::getInstance().close();
    DebugLog::getInstance().close();
    BasicLog::getInstance().close();
    
    return 0;
}
