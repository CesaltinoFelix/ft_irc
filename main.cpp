#include "Server.hpp"
#include <iostream>
#include <stdlib.h>

// Função para validar argumentos
static void validateArgs(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		std::cerr << "Example: " << argv[0] << " 6667 mypassword" << std::endl;
		exit(1);
	}

	// Verifica se a porta é um número válido
	for (int i = 0; argv[1][i]; i++)
	{
		if (!isdigit(argv[1][i]))
		{
			std::cerr << "Error: Port must be a number" << std::endl;
			exit(1);
		}
	}

	int port = atoi(argv[1]);
	if (port < 1024 || port > 65535)
	{
		std::cerr << "Error: Port must be between 1024 and 65535" << std::endl;
		exit(1);
	}
}

int main(int argc, char **argv)
{
	validateArgs(argc, argv);

	int port = atoi(argv[1]);
	std::string password = argv[2];

	// Cria e inicializa o servidor
	Server server(port, password);
	server.init();
	server.run();

	return 0;
}