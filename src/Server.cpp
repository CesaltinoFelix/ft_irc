#include "Server.hpp"
#include <cstdlib>

bool Server::_running = true;

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverSocket(-1)
{
}

Server::~Server()
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	for(std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		delete it->second;
	}

	_clients.clear();
	_channels.clear();
}

void Server::init()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
	{
		std::cerr << "Error creating socket" << std::endl;
		exit(1);
	}

	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error setting socket options" << std::endl;
		exit(1);
	}

	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error setting socket to non-blocking" << std::endl;
		exit(1);
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_serverAddr.sin_port = htons(_port);

	if (bind(_serverSocket, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
	{
		std::cerr << "Error binding socket" << std::endl;
		exit(1);
	}

	if (listen(_serverSocket, 5) < 0)
	{
		std::cerr << "Error listening on socket" << std::endl;
		exit(1);
	}

	struct pollfd serverPollFd;
	serverPollFd.fd = _serverSocket;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	_pollFds.push_back(serverPollFd);

	std::cout << GRE << "Server listening on port " << _port << WHI << std::endl;
}

void Server::acceptConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	int clientSocket = accept(_serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
	if (clientSocket < 0)
		return;

	if (static_cast<int>(_clients.size()) >= MAX_CLIENTS)
	{
		std::string msg = "ERROR :Server is full\r\n";
		send(clientSocket, msg.c_str(), msg.length(), MSG_NOSIGNAL);
		close(clientSocket);
		return;
	}

	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientSocket);
		return;
	}

	struct pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	_pollFds.push_back(clientPollFd);

	std::string clientIp = inet_ntoa(clientAddr.sin_addr);
	Client *newClient = new Client(clientSocket, clientIp);
	_clients[clientSocket] = newClient;
}

void Server::handleClientData(int fd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytesRead <= 0)
	{
		removeClient(fd);
		return;
	}

	Client *client = _clients[fd];
	std::string data(buffer, bytesRead);
	for (std::string::iterator it = data.begin(); it != data.end(); )
	{
		if (*it == '\0')
			it = data.erase(it);
		else
			++it;
	}
	client->appendToBuffer(data);

	if (client->getBuffer().size() > MAX_BUFFER_SIZE)
	{
		sendToClient(fd, "ERROR :Input line too long");
		removeClient(fd);
		return;
	}

	std::string &clientBuffer = client->getBufferRef();
	size_t pos;
	while ((pos = clientBuffer.find('\n')) != std::string::npos)
	{
		std::string command = clientBuffer.substr(0, pos);

		if (!command.empty() && command[command.length() - 1] == '\r')
			command.erase(command.length() - 1);

		clientBuffer.erase(0, pos + 1);

		if (!command.empty())
		{
			processCommand(fd, command);
			if (_clients.find(fd) == _clients.end())
				return;
		}
	}
}

void Server::removeClient(int fd)
{
	Client *client = NULL;
	std::map<int, Client *>::iterator cit = _clients.find(fd);
	if (cit != _clients.end())
		client = cit->second;

	if (client)
		remove_to_chanel(client);

	for (std::vector<struct pollfd>::iterator it = _pollFds.begin();
		 it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			break;
		}
	}
	if (cit != _clients.end())
	{
		delete cit->second;
		_clients.erase(cit);
	}
	close(fd);
}

void Server::signalHandler(int signum)
{
	(void)signum;
	_running = false;
}

void Server::run()
{
	while (_running)
	{
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount < 0)
		{
			if (!_running)
				break;
			break;
		}

		for (size_t i = 0; i < _pollFds.size(); i++)
		{
			if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (_pollFds[i].fd != _serverSocket)
				{
					removeClient(_pollFds[i].fd);
					break;
				}
			}
			else if (_pollFds[i].revents & POLLIN)
			{
				if (_pollFds[i].fd == _serverSocket)
				{
					acceptConnection();
				}
				else
				{
					handleClientData(_pollFds[i].fd);
					break;
				}
			}
		}
	}
}

void Server::closeServer()
{
	if (_serverSocket >= 0)
		close(_serverSocket);
}

int Server::getPort() const
{
	return (_port);
}

int Server::getServerSocket() const
{
	return (_serverSocket);
}

std::string Server::getPassword() const
{
	return (_password);
}

void Server::sendToClient(int fd, const std::string &message)
{
	std::string fullMessage = message + "\r\n";
	send(fd, fullMessage.c_str(), fullMessage.length(), MSG_NOSIGNAL);
}

void Server::processCommand(int fd, const std::string &command)
{
	std::string cmd;
	std::string args;

	std::string trimmed = command;
	size_t start = trimmed.find_first_not_of(' ');
	if (start == std::string::npos)
		return;
	trimmed = trimmed.substr(start);

	size_t spacePos = trimmed.find(' ');
	if (spacePos != std::string::npos)
	{
		cmd = trimmed.substr(0, spacePos);
		size_t argStart = trimmed.find_first_not_of(' ', spacePos);
		if (argStart != std::string::npos)
			args = trimmed.substr(argStart);
		else
			args = "";
	}
	else
	{
		cmd = trimmed;
		args = "";
	}
	cmd_execute(cmd, args, fd);
}

void Server::cmd_execute(std::string cmd, std::string args, int fd)
{
	if (_clients.find(fd) == _clients.end())
		return;
	Client *cliente = _clients[fd];

	std::string upperCmd = cmd;
	for (size_t i = 0; i < upperCmd.size(); i++)
		upperCmd[i] = toupper(upperCmd[i]);

	if (upperCmd == "PASS")
	{
		cmdPass(fd, args);
		return;
	}

	if (upperCmd == "QUIT")
	{
		removeClient(fd);
		return;
	}

	if (upperCmd == "PING")
	{
		if (!cliente->isAuthenticated())
		{
			sendToClient(fd, "451 :You have not registered");
			return;
		}
		if (args.empty())
			sendToClient(fd, "PONG ft_irc");
		else
			sendToClient(fd, "PONG " + args);
		return;
	}

	if (upperCmd == "NICK")
	{
		if (!cliente->isAuthenticated())
		{
			sendToClient(fd, "451 :You have not registered");
			return;
		}
		bool wasRegistered = cliente->get_nick() && cliente->get_user();
		set_nickname(args, fd, true);
		if (_clients.find(fd) == _clients.end())
			return;
		if (!wasRegistered && cliente->isAuthenticated()
			&& cliente->get_nick() && cliente->get_user())
			info(fd);
		return;
	}

	if (upperCmd == "USER")
	{
		if (!cliente->isAuthenticated())
		{
			sendToClient(fd, "451 :You have not registered");
			return;
		}
		if (!cliente->get_nick())
		{
			sendToClient(fd, "451 :You must set NICK first");
			return;
		}
		bool wasRegistered = cliente->get_nick() && cliente->get_user();
		set_username(args, fd, true);
		if (_clients.find(fd) == _clients.end())
			return;
		if (!wasRegistered && cliente->isAuthenticated()
			&& cliente->get_nick() && cliente->get_user())
			info(fd);
		return;
	}

	if (!cliente->isAuthenticated() || !cliente->get_nick() || !cliente->get_user())
	{
		cliente->incrementInvalidCmdCount();
		if (cliente->getInvalidCmdCount() > 10)
		{
			sendToClient(fd, "ERROR :Too many invalid commands before registration");
			removeClient(fd);
			return;
		}
		sendToClient(fd, "451 :You have not registered");
		return;
	}

	if (upperCmd == "JOIN")
	{
		if (args.empty())
		{
			sendToClient(fd, "461 JOIN :Not enough parameters");
			return;
		}
		cmdJoin(fd, args);
	}
	else if (upperCmd == "INVITE")
		cmdInvite(fd, args);
	else if (upperCmd == "TOPIC")
		cmdTopic(fd, args);
	else if (upperCmd == "PRIVMSG")
	{
		if (args.empty())
		{
			sendToClient(fd, "461 PRIVMSG :Not enough parameters");
			return;
		}
		message(fd, args);
	}
	else if (upperCmd == "PART")
		cmdPart(fd, args);
	else if (upperCmd == "MODE")
	{
		if (args.empty())
		{
			sendToClient(fd, "461 MODE :Not enough parameters");
			return;
		}
		size_t firstSpace = args.find(' ');
		if (firstSpace == std::string::npos)
		{
			sendToClient(fd, "461 MODE :Not enough parameters");
			return;
		}
		std::string channel = args.substr(0, firstSpace);
		std::string mode, targetNick;
		size_t secondSpace = args.find(' ', firstSpace + 1);
		if (secondSpace != std::string::npos)
		{
			mode = args.substr(firstSpace + 1, secondSpace - firstSpace - 1);
			targetNick = args.substr(secondSpace + 1);
		}
		else
		{
			mode = args.substr(firstSpace + 1);
			targetNick = "";
		}
		cmdMode(fd, channel, mode, targetNick);
	}
	else if (upperCmd == "KICK")
	{
		size_t sp = args.find(' ');
		if (sp == std::string::npos) {
			sendToClient(fd, "461 KICK :Not enough parameters");
		} else {
			std::string channel = args.substr(0, sp);
			std::string nick = args.substr(sp + 1);
			cmdKick(fd, channel, nick);
		}
	}
	else
		sendToClient(fd, "421 " + cmd + " :Unknown command");
}

void Server::set_nickname(std::string cmd, int fd, bool id)
{
	Client *cliente = _clients[fd];
	if(cmd.empty())
	{
		sendToClient(fd, "431 :No nickname given");
		return;
	}
	size_t sp = cmd.find(' ');
	if (sp != std::string::npos)
		cmd = cmd.substr(0, sp);
	if(cmd.empty())
	{
		sendToClient(fd, "431 :No nickname given");
		return;
	}
	if(cmd.size() > MAX_NICK_LEN)
	{
		sendToClient(fd, "432 " + cmd + " :Erroneous nickname (too long)");
		return;
	}
	if (!isalpha(cmd[0]) && cmd[0] != '_')
	{
		sendToClient(fd, "432 " + cmd + " :Erroneous nickname");
		return;
	}
	for (size_t i = 0; i < cmd.size(); i++)
	{
		if (!isalnum(cmd[i]) && cmd[i] != '_' && cmd[i] != '-'
			&& cmd[i] != '[' && cmd[i] != ']' && cmd[i] != '\\'
			&& cmd[i] != '`' && cmd[i] != '^' && cmd[i] != '{'
			&& cmd[i] != '}')
		{
			sendToClient(fd, "432 " + cmd + " :Erroneous nickname");
			return;
		}
	}
	for(std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != fd && it->second->getNickname() == cmd)
		{
			sendToClient(fd, "433 * " + cmd + " :Nickname is already in use");
			return;
		}
	}
	std::string oldNick = cliente->getNickname();
	cliente->setNickname(cmd, id);
	if (!oldNick.empty() && oldNick != cmd)
	{
		for (std::map<std::string, Channel*>::iterator cit = _channels.begin(); cit != _channels.end(); ++cit)
		{
			Channel *chan = cit->second;
			if (chan->hasClient(cmd))
			{
				if (chan->isOperator(oldNick))
				{
					chan->removeOperator(oldNick);
					chan->addOperator(cmd);
				}
				std::string nickMsg = ":" + oldNick + " NICK " + cmd + "\r\n";
				chan->broadcast(nickMsg);
			}
		}
	}
}

void Server::set_username(std::string &username, int fd, bool id)
{
	Client *cliente = _clients[fd];
	if (cliente->get_user())
	{
		sendToClient(fd, "462 :You may not reregister");
		return;
	}
	std::string user = username;
	size_t sp = username.find(' ');
	if (sp != std::string::npos)
		user = username.substr(0, sp);
	if(user.empty())
	{
		sendToClient(fd, "461 USER :Not enough parameters");
		return;
	}
	cliente->setUsername(user, id);
}

void Server::cmdPass(int fd, const std::string &args)
{
	Client *client = _clients[fd];

	if (client->isAuthenticated())
	{
		sendToClient(fd, "462 :You may not reregister");
		return;
	}

	if (args.empty())
	{
		sendToClient(fd, "461 PASS :Not enough parameters");
		return;
	}
	if (args == _password)
		client->setAuthenticated(true);
	else
	{
		sendToClient(fd, "464 :Password incorrect");
		removeClient(fd);
	}
}

