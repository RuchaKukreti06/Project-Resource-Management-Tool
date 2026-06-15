#include "ScreenOptions.h"

#include <iostream>

ScreenOptions::ScreenOptions(std::initializer_list<std::string> options) : options_(options)
{
}

ScreenOptions& ScreenOptions::withNumbering(bool enabled)
{
    numbering_ = enabled;
    return *this;
}

ScreenOptions& ScreenOptions::withPrefix(const std::string& prefix)
{
    prefix_ = prefix;
    return *this;
}

void ScreenOptions::render() const
{
    for (int i = 0; i < static_cast<int>(options_.size()); ++i)
    {
        if (numbering_)
            std::cout << prefix_ << (i + 1) << ". " << options_[i] << "\n";
        else
            std::cout << prefix_ << options_[i] << "\n";
    }
    std::cout << "\n";
}