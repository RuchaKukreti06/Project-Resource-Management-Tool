#include "ScreenInput.h"
#include "utils/ConsoleInput.h"

#include <iostream>

ScreenInput& ScreenInput::on(const std::string& choice, Handler handler)
{
    handlers_[choice] = std::move(handler);
    return *this;
}

ScreenInput& ScreenInput::onInvalid(Handler handler)
{
    invalidHandler_ = std::move(handler);
    return *this;
}

void ScreenInput::prompt() const
{
    std::string choice = ConsoleInput::readLine("Enter option");

    auto it = handlers_.find(choice);
    if (it != handlers_.end())
        it->second();
    else if (invalidHandler_)
        invalidHandler_();
    else
        std::cout << "\n  ✗  Invalid option.\n\n";
}