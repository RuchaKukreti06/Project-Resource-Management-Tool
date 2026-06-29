#pragma once

#include <httplib.h>

namespace utils
{

class GlobalExceptionHandler
{
   public:
    static void registerGlobalExceptionHandler(httplib::Server& server);
};

} // namespace utils
