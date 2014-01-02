/*
 *  This file is part of vaporpp.
 *
 *  vaporpp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vaporpp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vaporpp.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "client.hpp"

#include <array>
#include <algorithm>
#include <cassert>

#include <boost/asio.hpp>

using boost::asio::io_service;
using boost::asio::ip::tcp;


//pimpl-class (private members of client):
class vlpp::client::client_impl {
	public:
		client_impl(const std::string& servername, const std::string& token, uint16_t port);
		void authenticate(const std::string& token);
		void set_led(uint16_t led, rgba_color col);
		void flush();
		io_service _io_service;
		tcp::socket _socket;
		std::vector<char> cmd_buffer;
};


//opcodes:
enum class opcodes: uint8_t {
	SET_LED = 0x01,
	AUTHENTICATE = 0x02,
	STROBE = 0xFF
};

enum { TOKEN_SIZE = 16 };

///////////


vlpp::client::client(const std::string &server, const std::string &token, uint16_t port):
	_impl(std::make_shared<vlpp::client::client_impl>(server, token, port)) {
}

void vlpp::client::authenticate(const std::string& token){
	if(!_impl){
		throw vlpp::uninitialized_error("uninitialized use of a vlpp::client");
	}
	_impl->authenticate(token);
}

void vlpp::client::set_led(uint16_t led_id, const rgba_color &col) {
	if(!_impl){
		throw vlpp::uninitialized_error("uninitialized use of a vlpp::client");
	}
	_impl->set_led(led_id, col);
}

void vlpp::client::set_leds(const std::vector<uint16_t> &led_ids, const rgba_color &col) {
	if(!_impl){
		throw vlpp::uninitialized_error("uninitialized use of a vlpp::client");
	}
	for (auto led: led_ids) {
		set_led(led, col);
	}
}

void vlpp::client::flush() {
	if(!_impl){
		throw vlpp::uninitialized_error("uninitialized use of a vlpp::client");
	}
	_impl->flush();
}

std::vector<char>& vlpp::client::access_buffer(){
	if(!_impl){
		throw vlpp::uninitialized_error("uninitialized use of a vlpp::client");
	}
	return _impl->cmd_buffer;
}

///////// now: the private stuff


vlpp::client::client_impl::client_impl(const std::string &servername, const std::string &token, uint16_t port):
	_socket(_io_service) {
	tcp::resolver _resolver(_io_service);
	tcp::resolver::query q(servername, std::to_string(port));
	auto endpoints = _resolver.resolve(q);
	boost::asio::connect(_socket, endpoints);
	if (!_socket.is_open()) {
		throw vlpp::connection_failure("cannot open socket");
	}
	authenticate(token);
}

void vlpp::client::client_impl::authenticate(const std::string &token) {
	//first check the token:
	if (token.length() != TOKEN_SIZE) {
		throw std::invalid_argument("invalid token (wrong size)");
	}
	std::array<char,TOKEN_SIZE+1> auth_data;
	auth_data[0] = static_cast<char>(opcodes::AUTHENTICATE);
	for (size_t i = 0; i < TOKEN_SIZE; ++i) {
		auth_data[i+1] = static_cast<char>(token[i]);
	}
	boost::system::error_code e;
	boost::asio::write(_socket, boost::asio::buffer(&(auth_data[0]), auth_data.size()) , e);
	if (e) {
		throw vlpp::connection_failure("write failed");
	}
}

void vlpp::client::client_impl::set_led(uint16_t led, rgba_color col) {
	cmd_buffer.push_back(static_cast<char>(opcodes::SET_LED));
	cmd_buffer.push_back(static_cast<char>((led >> 8)));
	cmd_buffer.push_back(static_cast<char>((led & 0xff)));
	cmd_buffer.push_back(static_cast<char>(col.red));
	cmd_buffer.push_back(static_cast<char>(col.green));
	cmd_buffer.push_back(static_cast<char>(col.blue));
	cmd_buffer.push_back(static_cast<char>(col.alpha));
}

void vlpp::client::client_impl::flush() {
	cmd_buffer.push_back(static_cast<char>(opcodes::STROBE));
	boost::system::error_code e;
	boost::asio::write(_socket, boost::asio::buffer(&(cmd_buffer[0]), cmd_buffer.size()), e);
	cmd_buffer.clear();
	if (e) {
		throw vlpp::connection_failure("write failed");
	}
}

