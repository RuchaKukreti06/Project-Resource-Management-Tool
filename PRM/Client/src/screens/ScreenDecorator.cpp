#include "ScreenDecorator.h"

#include <iomanip>
#include <iostream>

static const std::string HORIZONTAL = "═";  // single UTF-8 box-drawing unit

ScreenDecorator::ScreenDecorator(const std::string& title)
{
    rows_.push_back(title);
}

ScreenDecorator::ScreenDecorator(const std::string& title, const std::string& subtitle)
{
    rows_.push_back(title);
    rows_.push_back(subtitle);
}

ScreenDecorator& ScreenDecorator::withRow(const std::string& text)
{
    rows_.push_back(text);
    return *this;
}

ScreenDecorator& ScreenDecorator::withWidth(int width)
{
    innerWidth_ = width;
    return *this;
}

ScreenDecorator& ScreenDecorator::withPadding(int padding)
{
    padding_ = padding;
    return *this;
}

void ScreenDecorator::render() const
{
    printTopBorder();
    for (const auto& row : rows_) printRow(row);
    printBottomBorder();
    std::cout << "\n";
}

void ScreenDecorator::printTopBorder() const
{
    std::string border;
    for (int i = 0; i < innerWidth_; ++i) border += HORIZONTAL;
    std::cout << "╔" << border << "╗\n";
}

void ScreenDecorator::printBottomBorder() const
{
    std::string border;
    for (int i = 0; i < innerWidth_; ++i) border += HORIZONTAL;
    std::cout << "╚" << border << "╝\n";
}

void ScreenDecorator::printRow(const std::string& text) const
{
    std::string padded = std::string(padding_, ' ') + text;

    if (static_cast<int>(padded.size()) > innerWidth_) padded = padded.substr(0, innerWidth_);

    std::cout << "║" << std::left << std::setw(innerWidth_) << std::setfill(' ') << padded << "║\n";
}
