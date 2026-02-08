#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

class Client
{
private:
	int					_fd;			// File descriptor do socket
	std::string			_ipAddr;		// Endereço IP do cliente
	std::string			_nickname;		// Nickname do cliente
	std::string			_username;		// Username do cliente
	std::string			_buffer;		// Buffer para mensagens incompletas
	bool				_authenticated;	// Se passou a senha correta

public:
	Client(int fd, const std::string &ipAddr);
	~Client();

	// Getters
	int					getFd() const;
	std::string			getIpAddr() const;
	std::string			getNickname() const;
	std::string			getUsername() const;
	std::string			getBuffer() const;
	bool				isAuthenticated() const;

	// Setters
	void				setNickname(const std::string &nickname);
	void				setUsername(const std::string &username);
	void				setAuthenticated(bool auth);

	// Buffer management
	void				appendToBuffer(const std::string &data);
	void				clearBuffer();
	std::string&		getBufferRef();	// Retorna referência para modificar
};

#endif
