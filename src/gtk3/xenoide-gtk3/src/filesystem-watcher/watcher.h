
#pragma once 

#include <map>
#include <string>

class Watcher {
public:
    explicit Watcher(const std::string &folder);

private:
    std::string folder;
};
