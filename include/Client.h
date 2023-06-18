#pragma once

#include <string>
#include <memory>

#include "Serialization.h"

class Client {
public:
    Client(const std::string& serverAddress, unsigned short serverPort,
        const std::string& sslSertificatePath = {});


    Message sendMessage(const Message& message);


    ~Client();

private:
    void close();
    void connect();
    Message receiveMessage();
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};