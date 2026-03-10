#include "Server.hpp"
void Server::info(int fd)
{
	std::string nick = getNickByFd(fd);
	sendToClient(fd, "001 " + nick + " :Welcome to the FT_IRC Network " + nick);
	sendToClient(fd, "002 " + nick + " :Your host is ft_irc, running version 1.0");
}

void Server::message(int fd, std::string args)
{
	if (args.empty())
	{
		sendToClient(fd, "411 :No recipient given (PRIVMSG)");
		return;
	}
	size_t spacePos = args.find(' ');
	if (spacePos == std::string::npos)
	{
		sendToClient(fd, "412 :No text to send");
		return;
	}
	std::string target = args.substr(0, spacePos);
	std::string msg = args.substr(spacePos + 1);
	if (msg.empty())
	{
		sendToClient(fd, "412 :No text to send");
		return;
	}
	if (msg[0] == ':')
		msg.erase(0, 1);
	if (target[0] == '#')
	{
		cmdPrivmsg(fd, target, msg);
	}
	else
	{
		cmdPrivmsg_to_client(fd, target, msg);
	}
}

Channel *Server::getChannel(std::string channelName)
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