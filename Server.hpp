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

class Server
{
private:
	int				_port;
	std::string		_password;
	int				_serverSocket;
	struct sockaddr_in	_serverAddr;

public:
	Server(int port, const std::string &password);
	~Server();

	// Métodos principais
	void	init();			// Inicializa o socket do servidor
	void	run();			// Loop principal do servidor
	void	acceptConnection();	// Aceita nova conexão
	void	closeServer();		// Fecha o servidor

	// Getters
	int		getPort() const;
	int		getServerSocket() const;
};

#endif
