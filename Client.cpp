#include "Client.hpp"

Client::Client(int fd, const std::string &ipAddr)
	: _fd(fd), _ipAddr(ipAddr), _authenticated(false), _registered(false)
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return _fd;
}

std::string Client::getIpAddr() const
{
	return _ipAddr;
}

std::string Client::getNickname() const
{
	return _nickname;
}

std::string Client::getUsername() const
{
	return _username;
}

std::string Client::getBuffer() const
{
	return _buffer;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setAuthenticated(bool auth)
{
	_authenticated = auth;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setRegistered(bool reg)
{
	_registered = reg;
}

// Buffer management... Acha que isso pode ser melhorado depois, mas por enquanto vamos manter simples
void Client::appendToBuffer(const std::string &data)
{
	_buffer += data;
}

void Client::clearBuffer()
{
	_buffer.clear();
}

std::string& Client::getBufferRef()
{
	return _buffer;
}
