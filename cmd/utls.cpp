#include "Server.hpp"
void Server::info(int fd)
{
	sendToClient(fd, ":User Autenticated and cadastraded");
	sendToClient(fd, "==================================================");
	sendToClient(fd, "==================================================");
	sendToClient(fd, "============= WELCOME TO FT_IRC ===================");
	sendToClient(fd, "===================================================");
	sendToClient(fd, "===================================================");
	sendToClient(fd, "COMANDS");
	sendToClient(fd, "=========");
	sendToClient(fd, "KICK");
	sendToClient(fd, "INVITE");
	sendToClient(fd, "JOIN");
	sendToClient(fd, "MODE");
}

void Server::message(int fd, std::string args)
{
	size_t spacePos = args.find(' ');
	if (spacePos == std::string::npos)
	{
		sendToClient(fd, "411 :No recipient given");
		return;
	}
	std::string target = args.substr(0, spacePos);
	std::string msg = args.substr(spacePos + 1);
	if (target[0] == '#')
	{
		cmdPrivmsg(fd, target, msg);
	}
	else
	{
		if (msg[0] == ':')
			msg.erase(0, 1);
		cmdPrivmsg_to_client(fd, target, msg);
	}
}

Client *Server::getChannel(std::string channelName)
{
	std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
	if (it != _channels.end())
	{
		return it->second;
	}
	return NULL;
}

std::string Server::getNickByFd(int fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		return it->second->getNickname();
	}
	return "";
}