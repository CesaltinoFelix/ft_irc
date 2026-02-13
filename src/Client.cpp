#include "Client.hpp"

Client::Client(int fd, const std::string &ipAddr)
	: _fd(fd), _ipAddr(ipAddr), _authenticated(false) , _nick(false) , _user(false)
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

void Client::setNickname(const std::string &nickname , bool id)
{
	_nickname = nickname;
	_nick = id;
}

void Client::setUsername(const std::string &username, bool id)
{
	_username = username;
	_user = id;
}

void Client::setAuthenticated(bool auth)
{
	_authenticated = auth;
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

bool Client::get_nick()const
{
	return this->_nick;
}

bool Client::get_user()const
{
	return this->_user;
}
