#include "Server.hpp"
#include <cstdlib>

// Construtor
Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverSocket(-1)
{
	std::cout << "Server object created" << std::endl;
}

// Destrutor
Server::~Server()
{
	std::cout << "Server object destroyed" << std::endl;
}

// Inicializa o socket do servidor
void Server::init()
{
	// Cria um socket TCP/IP
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

	// Configura o endereço do servidor
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;	// Escuta em todas as interfaces
	_serverAddr.sin_port = htons(_port);		// Converte porta para network byte order

	// Faz bind do socket com a porta
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

	std::cout << "Server listening on port " << _port << std::endl;
}

// Aceita uma nova conexão
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

	std::cout << "New connection from " << inet_ntoa(clientAddr.sin_addr) << std::endl;

	// Fecha a conexão (simples implementação)
	close(clientSocket);
}

// Loop principal do servidor
void Server::run()
{
	std::cout << "Server started!" << std::endl;
	std::cout << "Press Ctrl+C to stop" << std::endl;

	while (1)
	{
		acceptConnection();
	}
}

// Fecha o servidor
void Server::closeServer()
{
	if (_serverSocket >= 0)
	{
		close(_serverSocket);
		std::cout << "Server closed" << std::endl;
	}
}

// Getters
int Server::getPort() const
{
	return (_port);
}

int Server::getServerSocket() const
{
	return (_serverSocket);
}
