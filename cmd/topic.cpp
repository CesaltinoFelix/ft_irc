#include "Server.hpp"

void Server::cmdTopic(int fd, const std::string& args)
{
	if (args.empty()) {
		sendToClient(fd, "461 TOPIC :Not enough parameters");
		return;
	}

	std::string channelName;
	std::string newTopic;
	size_t spacePos = args.find(' ');
	if (spacePos != std::string::npos) {
		channelName = args.substr(0, spacePos);
		newTopic = args.substr(spacePos + 1);
		if (!newTopic.empty() && newTopic[0] == ':')
			newTopic.erase(0, 1);
	} else {
		channelName = args;
	}

	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendToClient(fd, "403 " + channelName + " :No such channel");
		return;
	}

	std::string nick = getNickByFd(fd);

	if (!channel->hasClient(nick)) {
		sendToClient(fd, "442 " + channelName + " :You're not on that channel");
		return;
	}

	if (spacePos == std::string::npos) {
		if (channel->getTopic().empty()) {
			sendToClient(fd, "331 " + nick + " " + channelName + " :No topic is set");
		} else {
			sendToClient(fd, "332 " + nick + " " + channelName + " :" + channel->getTopic());
		}
		return;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(nick)) {
		sendToClient(fd, "482 " + channelName + " :You're not channel operator");
		return;
	}

	channel->setTopic(newTopic);

	std::string topicMsg = ":" + nick + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	channel->broadcast(topicMsg);
}
