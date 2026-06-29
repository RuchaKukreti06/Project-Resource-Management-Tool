#ifndef SCREEN_INPUT_H
#define SCREEN_INPUT_H

#include <functional>
#include <string>
#include <unordered_map>

class ScreenInput
{
   public:
    using Handler = std::function<void()>;

    ScreenInput& on(const std::string& choice, Handler handler);
    ScreenInput& onInvalid(Handler handler);

    void prompt() const;

   private:
    std::unordered_map<std::string, Handler> handlers_;
    Handler invalidHandler_;
};

#endif