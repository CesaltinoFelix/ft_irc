#ifndef SERVER_HPP
#define SERVER_HPP

#define PASS "PASS"
#define NICK "NICK"
#define USER "USER"
#define QUIT "QUIT"
#define JOIN "JOIN"

#define NOT_REGISTERED "451 :You have not registered"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include "Client.hpp"
#include "Chanell.hpp"

#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Server
{
private:
	int							_port;
	std::string					_password;
	int							_serverSocket;
	struct sockaddr_in			_serverAddr;

	std::vector<struct pollfd>	_pollFds;		// Vetor de file descriptors para poll()
	std::map<int, Client*>		_clients;		// Map de fd -> Client*
	std::map<std::string, Channel*> _channels;  

public:
	Server(int port, const std::string &password);
	~Server();

	void	init();					
	void	run();					
	void	acceptConnection();		
	void	handleClientData(int fd);	
	void	removeClient(int fd);		
	void	closeServer();			

	void	processCommand(int fd, const std::string &command);
	void	sendToClient(int fd, const std::string &message);

	void	cmdPass(int fd, const std::string &args);

	int			getPort() const;
	int			getServerSocket() const;
	std::string	getPassword() const;
	void cmd_execute(std::string cmd, std::string args, int fd);
	void set_username(std::string &username, int fd , bool id);
	void set_nickname(std::string cmd, int fd, bool id);
	void cmdJoin(int fd, const std::string& channelName);
	void cmdPrivmsg(int fd, const std::string &target, const std::string &message);

	Client* getClientByNickname(const std::string& nickname);
};

#endif
