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
	bool				_nick;
	bool				_user;
	int					_invalidCmdCount;

public:
	Client(int fd, const std::string &ipAddr);
	~Client();

	int					getFd() const;
	std::string			getIpAddr() const;
	std::string			getNickname() const;
	std::string			getUsername() const;
	std::string			getBuffer() const;
	bool				isAuthenticated() const;

	void				setNickname(const std::string &nickname, bool id);
	void				setUsername(const std::string &username , bool id);
	void				setAuthenticated(bool auth);
	// Buffer management
	void				appendToBuffer(const std::string &data);
	void				clearBuffer();
	std::string&		getBufferRef();
	bool get_nick() const;
	bool get_user()const;
	int getInvalidCmdCount() const;
	void incrementInvalidCmdCount();
};

#endif
