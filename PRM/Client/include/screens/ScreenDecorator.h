#ifndef SCREEN_DECORATOR_H
#define SCREEN_DECORATOR_H

#include <string>
#include <vector>

class ScreenDecorator
{
   public:
    explicit ScreenDecorator(const std::string& title);
    ScreenDecorator(const std::string& title, const std::string& subtitle);
    ScreenDecorator() = default;
    ScreenDecorator& withRow(const std::string& text);
    ScreenDecorator& withWidth(int width);
    ScreenDecorator& withPadding(int padding);
    void render() const;

   private:
    std::vector<std::string> rows_;
    int innerWidth_ = 46;
    int padding_ = 4;

    void printTopBorder() const;
    void printBottomBorder() const;
    void printRow(const std::string& text) const;
};

#endif