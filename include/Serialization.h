#pragma once

#include <string>
#include <vector>
#include <any>

struct Message {
    std::string method;
    std::vector<std::any> parameters;

    static Message deserialize(const std::string& json);
    std::string serialize() const;
};