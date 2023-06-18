#include "Serialization.h"
#include "nlohmann/json.hpp"

Message Message::deserialize(const std::string& json)
{
    try {
        nlohmann::json jsonObject = nlohmann::json::parse(json);

        if (!jsonObject.is_object()) {
            throw std::runtime_error("Invalid JSON format");
        }

        if (!jsonObject.contains("method") || !jsonObject.contains("params")) {
            throw std::runtime_error("Missing required fields in JSON");
        }

        if (!jsonObject["method"].is_string() || !jsonObject["params"].is_array()) {
            throw std::runtime_error("Invalid data types in JSON");
        }

        std::string method = jsonObject["method"].get<std::string>();
        const auto& paramsArray = jsonObject["params"];
        std::vector<std::any> params;

        for (const auto& param : paramsArray) {
            if (param.is_null()) {
                params.push_back(std::any(nullptr));
            }
            else if (param.is_boolean()) {
                params.push_back(std::any(param.get<bool>()));
            }
            else if (param.is_number_integer()) {
                params.push_back(std::any(param.get<int>()));
            }
            else if (param.is_number_unsigned()) {
                params.push_back(std::any(param.get<unsigned int>()));
            }
            else if (param.is_number_float()) {
                params.push_back(std::any(param.get<double>()));
            }
            else if (param.is_string()) {
                params.push_back(std::any(param.get<std::string>()));
            }
            else {
                throw std::runtime_error("Unsupported data type in JSON");
            }
        }

        return Message(method, params);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to deserialize JSON: " + std::string(e.what()) + ". Received message content: " + json);
    }
}

std::string Message::serialize() const {
    try {
        nlohmann::json jsonObject;
        jsonObject["method"] = method;

        nlohmann::json paramsArray;
        for (const auto& param : parameters) {
            if (param.has_value()) {
                const std::type_info& paramType = param.type();
                if (paramType == typeid(nullptr)) {
                    paramsArray.push_back(nullptr);
                }
                else if (paramType == typeid(bool)) {
                    paramsArray.push_back(std::any_cast<bool>(param));
                }
                else if (paramType == typeid(int)) {
                    paramsArray.push_back(std::any_cast<int>(param));
                }
                else if (paramType == typeid(unsigned int)) {
                    paramsArray.push_back(std::any_cast<unsigned int>(param));
                }
                else if (paramType == typeid(double)) {
                    paramsArray.push_back(std::any_cast<double>(param));
                }
                else if (paramType == typeid(std::string)) {
                    paramsArray.push_back(std::any_cast<std::string>(param));
                }
                else {
                    throw std::runtime_error("Unsupported data type in params");
                }
            }
            else {
                paramsArray.push_back(nullptr);
            }
        }

        jsonObject["params"] = paramsArray;

        return jsonObject.dump();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to serialize Message to JSON: " + std::string(e.what()));
    }
}
