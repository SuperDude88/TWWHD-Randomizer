
#pragma once

// replace with utility::platformlog in actual project
#include <iostream>

#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception)                           \
    {std::cout << "Error in " << __FILE__ << ":" << __LINE__ \
               << " (function " << __FUNCTION__ << ") - "    \
               << (exception).what() << std::endl;           \
     std::abort();}
// and possibly replace abort with some cleanup code?