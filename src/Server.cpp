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
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
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
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

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

	std::map<int, Client *>::iterator it = _clients.find(fd);
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

	std::string commandUpper = cmd;

	for (size_t i = 0; i < commandUpper.size(); i++)
		commandUpper[i] = std::toupper(commandUpper[i]);

	bool isPreAuthCmd = (commandUpper == PASS || commandUpper == NICK ||
			commandUpper == USER || commandUpper == QUIT);

	if (!_clients[fd]->isAuthenticated() && !isPreAuthCmd) {
        sendToClient(fd, NOT_REGISTERED);
        return;
    }
	cmd_execute(commandUpper, args, fd);
}

void Server::cmd_execute(std::string cmd, std::string args, int fd)
{

	Client *cliente = _clients[fd];
	std::cout << cmd << std::endl;
	if (cmd == PASS) {
		cmdPass(fd, args);
		sendToClient(fd, "NICK: ");
	} else if (cmd == NICK) {
		set_nickname(args, fd, true);
		if (_clients[fd]->get_nick())
			sendToClient(fd, "USER: ");
	} else if (cmd == USER) {
		set_username(args, fd, true);
		if (cliente->isAuthenticated() && cliente->get_nick() && cliente->get_user())
		{
			sendToClient(fd, ":User Autenticated and cadastraded");
			sendToClient(fd, "==================================================");
			sendToClient(fd, "==================================================");
			sendToClient(fd, "============= WELCOME TO FT_IRC ===================");
			sendToClient(fd, "===================================================");
			sendToClient(fd, "===================================================");
			sendToClient(fd, "COMANDS");
			sendToClient(fd, "KICK");
			sendToClient(fd, "INVITE");
			sendToClient(fd, "JOIN");
			sendToClient(fd, "MODE");
		}
	} else if (cmd == QUIT) {
		removeClient(fd);
	} else if (cmd == JOIN) {
		cmdJoin(fd, args);
	} else if (cmd == PRIVMSG) {
		size_t spacePos = args.find(' ');
		if (spacePos == std::string::npos)
		{
			sendToClient(fd, "411 :No recipient given");
			return;
		}

		std::string target = args.substr(0, spacePos); // #avisos
		std::string msg = args.substr(spacePos + 1);   // :Olá galera

		if (msg[0] == ':')
			msg.erase(0, 1); // remove o :

		cmdPrivmsg(fd, target, msg);
	}

	else
	{
		sendToClient(fd, "UNKNOWN  COMAND");
	}
}

void Server::cmdPrivmsg(int fd, const std::string &target, const std::string &message)
{
	Client *sender = _clients[fd];

	if (target.empty() || message.empty())
	{
		sendToClient(fd, "412 :No text to send");
		return;
	}
	if (target[0] == '#')
	{
		if (_channels.find(target) == _channels.end())
		{
			sendToClient(fd, "403 " + target + " :No such channel");
			return;
		}

		Channel *channel = _channels[target];
		std::string fullMessage = ":" + sender->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		channel->broadcast(fullMessage);
	}
	else
	{
		sendToClient(fd, "401 " + target + " :No such nick");
	}
}

void Server::set_nickname(std::string nick, int fd, bool id)
{
	if (nick.empty()) {
		sendToClient(fd, "431 :No nickname given");
		return;
	}

	Client* existing = getClientByNickname(nick);
	
	if (existing != NULL && existing->getFd() != fd) {
		sendToClient(fd, "433 * " + nick + " :Nickname is already in use");
		return;
	}

	Client *cliente = _clients[fd];
	cliente->setNickname(nick, id);
	std::cout << "fd" << fd << " set nickname: " << nick << std::endl;
}

void Server::set_username(std::string &args, int fd, bool id)
{
	Client *cliente = _clients[fd];

	if (!cliente->isAuthenticated()) {
		sendToClient(fd, NOT_REGISTERED);
		return;
	}

	std::string username = args;
	size_t spacePosition = args.find(' ');

	if (spacePosition != std::string::npos)
		username = args.substr(0, spacePosition);

	if (username.empty()) {
		sendToClient(fd, "461 USER :Not enough parameters");
		return;
	}
	cliente->setUsername(username, id);
	std::cout << "fd " << fd << " set username: " << username << std::endl;
}

// Cefelix > pessoal, aqui comecei fazendo o parser dos Comandos IRC. Por enquanto só implementei o PASS, mas a ideia é ir implementando os outros aos poucos. O modelo é bem simples: separar o comando dos argumentos, converter o comando para maiúsculas e depois usar if/else para chamar a função correspondente. Sei que isso não é super escalável, mas para um projeto pequeno como esse acho que é suficiente. Se fosse algo maior, aí sim eu consideraria uma abordagem mais sofisticada, tipo map de string -> função ou algo do tipo. O que vocês acham?
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
		std::cout << "--->" << args << std::endl;
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

void Server::cmdJoin(int fd, const std::string &channelName)
{
	Client *client = _clients[fd];

	if (!client->isAuthenticated() || !client->get_nick() || !client->get_user())
	{
		sendToClient(fd, "451 :You have not registered");
		return;
	}

	if (channelName.empty() || channelName[0] != '#')
	{
		sendToClient(fd, "476 :Bad Channel Mask");
		return;
	}

	// Se canal não existe → cria
	if (_channels.find(channelName) == _channels.end())
	{
		_channels[channelName] = new Channel(channelName);
	}
	Channel *channel = _channels[channelName];
	channel->addClient(client);
	std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
	channel->broadcast(joinMsg);
}

Client* Server::getClientByNickname(const std::string& nickname) {
	
	std::map<int, Client*>::iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second->getNickname() == nickname) {
			return (it->second);
		}
	}
	return (NULL);
}
