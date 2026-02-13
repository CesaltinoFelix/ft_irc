#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

class Client
{
private:
	int					_fd;	
	std::string			_ipAddr;
	std::string			_nickname;
	std::string			_username;
	std::string			_buffer;		// Buffer para mensagens incompletas
	bool				_authenticated;	// Se passou a senha correta

public:
	Client(int fd, const std::string &ipAddr);
	~Client();

	int					getFd() const;
	std::string			getIpAddr() const;
	std::string			getNickname() const;
	std::string			getUsername() const;
	std::string			getBuffer() const;
	bool				isAuthenticated() const;

	void				setNickname(const std::string &nickname);
	void				setUsername(const std::string &username);
	void				setAuthenticated(bool auth);

	// Buffer management
	void				appendToBuffer(const std::string &data);
	void				clearBuffer();
	std::string&		getBufferRef();	// Retorna referência para modificar. Essa parte fiz com a ajuda do ChatGPT, achei que seria mais prático para processar os comandos. Mas preciso avaliar se é a melhor abordagem depois.
};

#endif
