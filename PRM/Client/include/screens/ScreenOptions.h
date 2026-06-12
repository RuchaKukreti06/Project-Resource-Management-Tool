#ifndef SCREEN_OPTIONS_H
#define SCREEN_OPTIONS_H

#include <initializer_list>
#include <string>
#include <vector>

class ScreenOptions
{
   public:
    ScreenOptions(std::initializer_list<std::string> options);

    ScreenOptions& withNumbering(bool enabled);
    ScreenOptions& withPrefix(const std::string& prefix);

    void render() const;

   private:
    std::vector<std::string> options_;
    bool numbering_ = true;
    std::string prefix_;
};

#endif