#include "Client.h"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include <optional>
#include <vector>

namespace
{
    static constexpr std::size_t bufferSize = 1024;
}

struct Client::Impl
{
    using io_context = boost::asio::io_context;
    using ssl_context = boost::asio::ssl::context;
    using tcp_socket = boost::asio::ip::tcp::socket;
    using ssl_socket = boost::asio::ssl::stream<tcp_socket>;
    using tcp = boost::asio::ip::tcp;

    Impl(const std::string& i_serverAddress, unsigned short i_port,
        const std::string& pathToSertificate) :
        serverAddress(i_serverAddress),
        serverPort(i_port) 
    {
        if (!pathToSertificate.empty()) {
            sslContext = ssl_context{ ssl_context::tlsv12_client };
            sslContext->set_verify_mode(boost::asio::ssl::verify_peer);
            sslContext->load_verify_file(pathToSertificate);
        }
    }

    io_context ioContext;
    std::optional<ssl_context> sslContext;
    std::optional<tcp_socket> socket;
    std::optional<ssl_socket> sslSocket;
    std::string serverAddress;
    unsigned short serverPort;
    tcp::resolver::results_type endpoints;
};


Client::Client(const std::string& serverAddress, unsigned short serverPort,
    const std::string& sslSertificatePath)
    : m_pImpl{ new Impl(serverAddress, serverPort, sslSertificatePath) } 
{
    using tcp = boost::asio::ip::tcp;
    tcp::resolver resolver(m_pImpl->ioContext);
    m_pImpl->endpoints = resolver.resolve(
        m_pImpl->serverAddress, std::to_string(m_pImpl->serverPort));
}

void Client::connect()
{
    if (m_pImpl->sslContext.has_value()) {

        m_pImpl->sslSocket = Impl::ssl_socket{ m_pImpl->ioContext, *m_pImpl->sslContext };

        boost::asio::connect(m_pImpl->sslSocket->lowest_layer(), m_pImpl->endpoints);

        try {
        // Perform SSL handshake
        m_pImpl->sslSocket->handshake(boost::asio::ssl::stream_base::client);
        }
        catch (...) {
            throw std::runtime_error("Certificate is not accepted");
        }
    }
    else {
        m_pImpl->socket = Impl::tcp_socket{ m_pImpl->ioContext };
        boost::asio::connect(*m_pImpl->socket, m_pImpl->endpoints);
    }
}

Message Client::sendMessage(const Message& message) {
    connect();
    std::string serializedMessage = message.serialize();
    auto writeToSocket = [&](auto& socket)
    {
        boost::asio::write(*socket, boost::asio::buffer(serializedMessage));
    };

	m_pImpl->sslContext.has_value() ?
		writeToSocket(m_pImpl->sslSocket) :
		writeToSocket(m_pImpl->socket);
    auto response = receiveMessage();
    close();
    return response;
}

Message Client::receiveMessage() 
{
    std::array<char, bufferSize> buffer;
    std::string received_data;

    while (true) {

        boost::system::error_code error;
        auto readFromSocket = [&](auto& socket)
        {
            return socket.read_some(boost::asio::buffer(buffer), error);
        };

        const size_t length = m_pImpl->sslContext.has_value()
            ? readFromSocket(*m_pImpl->sslSocket)
            : readFromSocket(*m_pImpl->socket);

        if (error == boost::asio::error::eof) {
            break;  // Connection closed by the client
        }
        else if (error) {
            throw boost::system::system_error(error);
        }

        received_data.insert(received_data.end(), buffer.begin(), buffer.begin() + length);

        if (length < buffer.size()) {
            break;  // End of message
        }
    }

    return Message::deserialize(received_data);
}

void Client::close() {
    if (m_pImpl->sslContext.has_value()) {
        m_pImpl->sslSocket->lowest_layer().close();
    }
    else {
		m_pImpl->socket->close();
    }
}

Client::~Client() = default;