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

	// Métodos principais
	void	init();					// Inicializa o socket do servidor
	void	run();					// Loop principal do servidor
	void	acceptConnection();		// Aceita nova conexão
	void	handleClientData(int fd);	// Lê e processa dados de um cliente
	void	removeClient(int fd);		// Remove um cliente desconectado
	void	closeServer();			// Fecha o servidor

	// Getters
	int			getPort() const;
	int			getServerSocket() const;
	std::string	getPassword() const;
};

#endif
