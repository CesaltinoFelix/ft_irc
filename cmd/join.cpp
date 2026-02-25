#include "Server.hpp"

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
    std::string chanNameOnly = channelName;
    std::string key = "";
    size_t spacePos = channelName.find(' ');
    if (spacePos != std::string::npos) {
        chanNameOnly = channelName.substr(0, spacePos);
        key = channelName.substr(spacePos + 1);
    }

    if (_channels.find(chanNameOnly) == _channels.end())
    {
        _channels[chanNameOnly] = new Channel(chanNameOnly);
    }
    Channel *channel = _channels[chanNameOnly];

    if (channel->hasKey()) {
        if (key.empty() || !channel->checkKey(key)) {
            sendToClient(fd, "475 " + chanNameOnly + " :Cannot join channel (+k) - bad key");
            return;
        }
    }

    if (channel->isFull()) {
        sendToClient(fd, "471 " + chanNameOnly + " :Cannot join channel (+l) - channel is full");
        return;
    }

    channel->addClient(client);
    std::string joinMsg = ":" + client->getNickname() + " JOIN " + chanNameOnly + "\r\n";
    channel->broadcast(joinMsg);
}
