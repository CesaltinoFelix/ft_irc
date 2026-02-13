#include "Server.hpp"
#include <cstdlib>

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverSocket(-1)
{
	std::cout << "Server object created" << std::endl;
}

Server::~Server()
{
	// Limpar todos os clientes
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();
	std::cout << "Server object destroyed" << std::endl;
}

void Server::init()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
	{
		std::cerr << "Error creating socket" << std::endl;
		exit(1);
	}

	// Permite reutilizar a porta (evita "Address already in use")
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error setting socket options" << std::endl;
		exit(1);
	}

	// Configurar socket como non-blocking. Descobri que isso é importante para o modelo de multiplexação com poll()
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

	std::cout << "Server listening on port " << _port << std::endl;
}

void Server::acceptConnection()
{
	struct sockaddr_in	clientAddr;
	socklen_t			addrLen = sizeof(clientAddr);

	int clientSocket = accept(_serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
	if (clientSocket < 0)
	{
		std::cerr << "Error accepting connection" << std::endl;
		return;
	}

	// Configurar socket do cliente como non-blocking
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error setting client socket to non-blocking" << std::endl;
		close(clientSocket);
		return;
	}

	struct pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	_pollFds.push_back(clientPollFd);

	// Criar objeto Client e adicionar ao map
	std::string clientIp = inet_ntoa(clientAddr.sin_addr);
	Client *newClient = new Client(clientSocket, clientIp);
	_clients[clientSocket] = newClient;

	std::cout << "New connection from " << clientIp;
	std::cout << " (fd: " << clientSocket << ")" << std::endl;
	std::cout << "Total clients: " << _clients.size() << std::endl;
}

void Server::handleClientData(int fd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
			std::cout << "Client disconnected (fd: " << fd << ")" << std::endl;
		else
			std::cerr << "Error reading from client (fd: " << fd << ")" << std::endl;
		removeClient(fd);
		return;
	}

	Client *client = _clients[fd];
	client->appendToBuffer(std::string(buffer, bytesRead));

	// Processar comandos completos (terminam com \r\n ou \n)
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
			std::cout << "Received from fd " << fd << ": " << command << std::endl;
			processCommand(fd, command);
		}
	}
}

void Server::removeClient(int fd)
{

	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			break;
		}
	}

	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		delete it->second;
		_clients.erase(it);
	}

	close(fd);
	std::cout << "Client removed. Total clients: " << _clients.size() << std::endl;
}

void Server::run()
{
	std::cout << "Server started!" << std::endl;
	std::cout << "Press Ctrl+C to stop" << std::endl;

	while (1)
	{
		// poll() espera por eventos em todos os file descriptors
		// -1 = espera indefinidamente
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);

		if (pollCount < 0)
		{
			std::cerr << "Error in poll()" << std::endl;
			break;
		}

		// Verificar cada fd para ver se tem eventos
		for (size_t i = 0; i < _pollFds.size(); i++)
		{
			if (_pollFds[i].revents & POLLIN)
			{
				if (_pollFds[i].fd == _serverSocket)
				{
					acceptConnection();
				}
				else
				{
					handleClientData(_pollFds[i].fd);
				}
			}
		}
	}
}

void Server::closeServer()
{
	if (_serverSocket >= 0)
	{
		close(_serverSocket);
		std::cout << "Server closed" << std::endl;
	}
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
	send(fd, fullMessage.c_str(), fullMessage.length(), 0);
}

void Server::processCommand(int fd, const std::string &command)
{
	std::string cmd;
	std::string args;

	size_t spacePos = command.find(' ');
	if (spacePos != std::string::npos)
	{
		cmd = command.substr(0, spacePos);
		args = command.substr(spacePos + 1);
	}
	else
	{
		cmd = command;
		args = "";
	}

	for (size_t i = 0; i < cmd.length(); i++)
		cmd[i] = toupper(cmd[i]);

	std::cout << "Command: " << cmd << ", Args: " << args << std::endl;
	if (cmd == "PASS")
		cmdPass(fd, args);
	if(cmd == "NICK")
		cmdNick(fd, args);
	else if (cmd == "USER")
		cmdUser(fd, args);
	else if (!_clients[fd]->isAuthenticated())
	{
		sendToClient(fd, "ERROR :You must authenticate first with PASS");
	}
	else
	{
		sendToClient(fd, "421 " + cmd + " :Unknown command");
	}
}


void Server::cmdNick(int fd, const std::string &args)
{
	Client *client = _clients[fd];

	if (args.empty())
	{
		sendToClient(fd, "431 :No nickname given");
		return;
	}

	std::string nickname = args;

	// Remove trailing spaces or something? But for now, assume args is the nickname.

	// Check for spaces
	if (nickname.find(' ') != std::string::npos)
	{
		sendToClient(fd, "432 " + nickname + " :Erroneous nickname");
		return;
	}

	// Check length (IRC standard: 1-9 chars)
	if (nickname.length() < 1 || nickname.length() > 9)
	{
		sendToClient(fd, "432 " + nickname + " :Erroneous nickname");
		return;
	}

	// Check if already in use
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != fd && it->second->getNickname() == nickname)
		{
			sendToClient(fd, "433 " + nickname + " :Nickname is already in use");
			return;
		}
	}

	// Set nickname
	client->setNickname(nickname);
	std::cout << "Client fd " << fd << " set nickname to " << nickname << std::endl;

	// If authenticated and has username, set registered
	if (client->isAuthenticated() && !client->getUsername().empty())
	{
		client->setRegistered(true);
		std::cout << "Client fd " << fd << " is now registered" << std::endl;
		// Send welcome message? But since no USER, maybe not yet.
	}
}

void Server::cmdUser(int fd, const std::string &args)
{
	(void)fd;
	(void)args;
	// Implementação do comando USER
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
	{
		client->setAuthenticated(true);
		std::cout << "Client fd " << fd << " authenticated successfully" << std::endl;
		// Não enviamos mensagem de sucesso aqui - o cliente ainda precisa de NICK e USER
	}
	else
	{
		sendToClient(fd, "464 :Password incorrect");
		std::cout << "Client fd " << fd << " failed authentication (wrong password)" << std::endl;
	}
}
