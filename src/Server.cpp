#include "Server.h"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include <iostream>
#include <optional>

#include <filesystem>

Server::Server(int port, const ServerSSLData& data) : 
	m_port(port), m_sslData(data)
{
}

void Server::start()
{
	namespace asio = boost::asio;
	using io_context = asio::io_context;
	using ssl_context = asio::ssl::context;
	using tcp = asio::ip::tcp;
	using tcp_socket = tcp::socket;
	using ssl_socket = asio::ssl::stream<tcp_socket>;
	using tcp_acceptor = tcp::acceptor;
	using error_code = boost::system::error_code;
	using system_error = boost::system::system_error;

	try {
		io_context ioContext;
		std::optional<ssl_context> sslContext;
		if (!m_sslData.empty()) {
			sslContext = ssl_context{ssl_context::sslv23};

			sslContext->use_certificate_file(m_sslData.certificate, ssl_context::pem);

			sslContext->use_private_key_file(m_sslData.privateKey, ssl_context::pem);

			sslContext->set_options(ssl_context::default_workarounds | ssl_context::no_sslv2 | ssl_context::single_dh_use);
		}

		tcp_acceptor acceptor(ioContext, tcp::endpoint(tcp::v4(), m_port));

		std::cout << "Server started on port " << m_port << std::endl;

		while (true) {

			try {
				std::optional<tcp_socket> tcpSocket;
				std::optional<ssl_socket> sslSocket;
				if (sslContext.has_value()) {
					sslSocket = ssl_socket{ ioContext, *sslContext };

					acceptor.accept(sslSocket->lowest_layer());
					sslSocket->handshake(asio::ssl::stream_base::server);
				}
				else {
					tcpSocket = tcp_socket{ ioContext };
					acceptor.accept(*tcpSocket);
				}

				std::array<char, 1024> buffer;
				std::vector<char> received_data;

				// Receive the message from the client
				while (true) {

					error_code error;
					auto readFromSocket = [&](auto& socket)
					{
						return socket.read_some(asio::buffer(buffer), error);
					};

					const size_t length = sslContext.has_value()
						? readFromSocket(*sslSocket) 
						: readFromSocket(*tcpSocket);

					if (error == asio::error::eof) {
						break;  // Connection closed by the client
					}
					else if (error) {
						throw system_error(error);
					}

					received_data.insert(received_data.end(), buffer.begin(), buffer.begin() + length);

					if (length < buffer.size()) {
						break;  // End of message
					}
				}

				// Deserialize the received message
				std::string json_data(received_data.begin(), received_data.end());
				Message request = Message::deserialize(json_data);

				// Process the request and get the response
				Message response = processRequest(request);

				// Serialize the response
				std::string json_response = response.serialize();

				// Send the response back to the client
				auto writeToSocket = [&](auto& socket)
				{
					asio::write(socket, asio::buffer(json_response));
				};

				sslSocket.has_value() ? writeToSocket(*sslSocket) : writeToSocket(*tcpSocket);

				if (sslSocket.has_value()) {
					sslSocket->lowest_layer().close();
				}
				else {
					tcpSocket->close();
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error starting the server: " << e.what() << std::endl;
	}
}

Message Server::processRequest(Message& request) {
	const std::string& methodName = request.method;
	auto& params = request.parameters;

	// Check if the requested method is registered
	auto it = m_functions.find(methodName);
	if (it == m_functions.end()) {
		// Method not found, send an error response
		return Message{ "error", { "Method not found: " + methodName } };
	}

	// Extract the registered RPC function for the method
	const auto& rpcFunction = it->second;


	try {
		// Call the RPC function with the extracted arguments
		auto result = rpcFunction(std::move(params));

		// Create a success response with the result
		return Message{ "success", { result } };
	}
	catch (const std::exception& e) {
		// Error occurred during RPC function execution, send an error response
		return Message{ "error", { std::string("RPC error: ") + e.what() } };
	}
}
