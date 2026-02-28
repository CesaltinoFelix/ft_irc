#include "Server.hpp"

void Server::cmdInvite(int fd, const std::string& args)
{
	size_t spacePos = args.find(' ');
	if (spacePos == std::string::npos) {
		sendToClient(fd, "461 INVITE :Not enough parameters");
		return;
	}
	std::string targetNick = args.substr(0, spacePos);
	std::string channelName = args.substr(spacePos + 1);

	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendToClient(fd, "403 " + channelName + " :No such channel");
		return;
	}

	std::string inviterNick = getNickByFd(fd);
	if (!channel->isOperator(inviterNick)) {
		sendToClient(fd, "482 " + channelName + " :You're not channel operator");
		return;
	}

	if (!channel->hasClient(inviterNick)) {
		sendToClient(fd, "442 " + channelName + " :You're not on that channel");
		return;
	}

	int inviteeFd = getFdByNick(targetNick);
	if (inviteeFd == -1) {
		sendToClient(fd, "401 " + targetNick + " :No such nick");
		return;
	}

	if (channel->hasClient(targetNick)) {
		sendToClient(fd, "443 " + targetNick + " " + channelName + " :is already on channel");
		return;
	}

	channel->addInvited(targetNick);

	sendToClient(inviteeFd, ":" + inviterNick + " INVITE " + targetNick + " " + channelName);

	sendToClient(fd, "341 " + inviterNick + " " + targetNick + " " + channelName);
}

int Server::getFdByNick(const std::string& nick)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second->getNickname() == nick)
			return it->first;
	}
	return -1;
}
