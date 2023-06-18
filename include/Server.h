#pragma once

#include <string>
#include <map>
#include <functional>
#include <any>
#include <tuple>
#include <stdexcept>

#include "func_traits.h"
#include "Serialization.h"


struct ServerSSLData
{
    std::string certificate;
    std::string privateKey;

    bool empty() const noexcept {
        return certificate.empty() && privateKey.empty();
    }
};

class Server {
public:
    Server(int port, const ServerSSLData& sslData = {});
    void start();

    template <typename Callable>
    void addFunction(std::string name, Callable i_function);

private:
    Message processRequest(Message& request);

private:
    using FunctionType = std::function<std::any(std::vector<std::any>&&)>;
    std::map<std::string, FunctionType> m_functions;

    int m_port;
    const ServerSSLData m_sslData;
};

template<typename Callable>
inline void Server::addFunction(std::string name, Callable i_function)
{
    m_functions[name] = [i_function, name](std::vector<std::any>&& args)
        -> std::any
    {
        using traits = func_traits<Callable>;
        if (args.size() != traits::input_size) {
			throw std::runtime_error("Function "
                + name + " accepts " + std::to_string(traits::input_size)
                + " arguments. Provided: " + std::to_string(args.size()));
        }

        auto tuple = traits::getArgsFromVector(args);

        if constexpr (std::is_same<typename traits::result_type, void>()) {
            std::apply(i_function, std::move(tuple));
            return nullptr;
        }
        else {
            return std::apply(i_function, std::move(tuple));
        }
    };
}
