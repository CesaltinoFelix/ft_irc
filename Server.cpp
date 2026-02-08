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

	// Configurar socket como non-blocking
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error setting socket to non-blocking" << std::endl;
		exit(1);
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;	// Escuta em todas as interfaces
	_serverAddr.sin_port = htons(_port);		// Converte porta para network byte order

	if (bind(_serverSocket, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
	{
		std::cerr << "Error binding socket" << std::endl;
		exit(1);
	}

	// Coloca o socket em modo de escuta (max 5 conexões na fila)
	if (listen(_serverSocket, 5) < 0)
	{
		std::cerr << "Error listening on socket" << std::endl;
		exit(1);
	}

	// Adicionar o socket do servidor ao vetor de poll
	struct pollfd serverPollFd;
	serverPollFd.fd = _serverSocket;
	serverPollFd.events = POLLIN;	// Interesse em leitura (novas conexões)
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

	// Adicionar ao vetor de poll
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
		// Cliente desconectou ou erro
		if (bytesRead == 0)
			std::cout << "Client disconnected (fd: " << fd << ")" << std::endl;
		else
			std::cerr << "Error reading from client (fd: " << fd << ")" << std::endl;
		removeClient(fd);
		return;
	}

	// Adicionar dados ao buffer do cliente
	Client *client = _clients[fd];
	client->appendToBuffer(std::string(buffer, bytesRead));

	// Processar comandos completos (terminam com \r\n ou \n)
	std::string &clientBuffer = client->getBufferRef();
	size_t pos;
	while ((pos = clientBuffer.find('\n')) != std::string::npos)
	{
		std::string command = clientBuffer.substr(0, pos);
		// Remover \r se existir
		if (!command.empty() && command[command.length() - 1] == '\r')
			command.erase(command.length() - 1);

		clientBuffer.erase(0, pos + 1);

		// Por agora, apenas mostrar o comando recebido
		std::cout << "Received from fd " << fd << ": " << command << std::endl;

		// TODO: Processar comandos IRC aqui (PASS, NICK, USER, etc.)
	}
}

void Server::removeClient(int fd)
{
	// Remover do vetor de poll
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			break;
		}
	}

	// Remover do map e deletar o objeto
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
					// Nova conexão no socket do servidor
					acceptConnection();
				}
				else
				{
					// Dados de um cliente existente
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
