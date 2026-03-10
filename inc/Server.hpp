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
#include <cstdlib>
#include <csignal>
#include "Client.hpp"
#include "Chanell.hpp"

#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
#define MAX_CLIENTS 100
#define MAX_BUFFER_SIZE 4096
#define MAX_NICK_LEN 9

class Server
{
private:
	int							_port;
	std::string					_password;
	int							_serverSocket;
	struct sockaddr_in			_serverAddr;
	std::vector<struct pollfd>	_pollFds;
	std::map<int, Client *>		_clients;
	std::map<std::string, Channel *> _channels;
	static bool					_running;

public:
	Server(int port, const std::string &password);
	~Server();
	static void		signalHandler(int signum);

	void			init();
	void			run();
	void			acceptConnection();
	void			handleClientData(int fd);
	void			removeClient(int fd);
	void			closeServer();

	void			processCommand(int fd, const std::string &command);
	void			sendToClient(int fd, const std::string &message);
	void			cmdPass(int fd, const std::string &args);

	int				getPort() const;
	int				getServerSocket() const;
	std::string		getPassword() const;

	void			cmd_execute(std::string cmd, std::string args, int fd);
	void			set_username(std::string &username, int fd, bool id);
	void			set_nickname(std::string cmd, int fd, bool id);
	void			cmdJoin(int fd, const std::string &channelName);
	void			cmdInvite(int fd, const std::string &args);
	void			cmdTopic(int fd, const std::string &args);
	void			cmdPrivmsg(int fd, const std::string &target, const std::string &message);
	void			cmdPrivmsg_to_client(int fd, const std::string &target, const std::string &message);
	void			cmdMode(int fd, const std::string &channel, const std::string &mode, const std::string &targetNick);
	void			cmdKick(int fd, const std::string &channelName, const std::string &client_to_kick);
	void			cmdPart(int fd, const std::string &args);
	void			info(int fd);
	void			message(int fd, std::string args);
	void			remove_to_chanel(Client *cl);
	std::string		getNickByFd(int fd);
	Channel			*getChannel(std::string channelName);
	int				getFdByNick(const std::string &nick);
};

#endif
