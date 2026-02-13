#ifndef SERVER_HPP
#define SERVER_HPP

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

class Server
{
private:
	int							_port;
	std::string					_password;
	int							_serverSocket;
	struct sockaddr_in			_serverAddr;

	std::vector<struct pollfd>	_pollFds;		// Vetor de file descriptors para poll()
	std::map<int, Client*>		_clients;		// Map de fd -> Client*

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
};

#endif
