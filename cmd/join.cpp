#include "Server.hpp"

void Server::cmdJoin(int fd, const std::string &channelName)
{
    Client *client = _clients[fd];

    if (!client->isAuthenticated() || !client->get_nick() || !client->get_user())
    {
        sendToClient(fd, "451 :You have not registered");
        return;
    }

    if (channelName.empty())
    {
        sendToClient(fd, "461 JOIN :Not enough parameters");
        return;
    }

    std::string chanNameOnly = channelName;
    std::string key = "";
    size_t spacePos = channelName.find(' ');
    if (spacePos != std::string::npos) {
        chanNameOnly = channelName.substr(0, spacePos);
        key = channelName.substr(spacePos + 1);
    }

    if (chanNameOnly.empty() || chanNameOnly[0] != '#')
    {
        sendToClient(fd, "476 " + chanNameOnly + " :Bad Channel Mask");
        return;
    }

    // Validate channel name characters
    for (size_t i = 1; i < chanNameOnly.size(); i++)
    {
        if (chanNameOnly[i] == ' ' || chanNameOnly[i] == ',' 
            || chanNameOnly[i] == '\x07' || chanNameOnly[i] == '\r'
            || chanNameOnly[i] == '\n' || chanNameOnly[i] == '\0')
        {
            sendToClient(fd, "476 " + chanNameOnly + " :Bad Channel Mask");
            return;
        }
    }

    // Limit channel name length
    if (chanNameOnly.size() > 50)
    {
        sendToClient(fd, "476 " + chanNameOnly.substr(0, 50) + " :Bad Channel Mask");
        return;
    }

    if (_channels.find(chanNameOnly) == _channels.end())
    {
        _channels[chanNameOnly] = new Channel(chanNameOnly);
    }
    Channel *channel = _channels[chanNameOnly];

    // Check if already in channel
    if (channel->hasClient(client->getNickname()))
    {
        return;
    }

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

    if (channel->isInviteOnly()) {
        if (!channel->isInvited(client->getNickname())) {
            sendToClient(fd, "473 " + chanNameOnly + " :Cannot join channel (+i) - invite only");
            return;
        }
        channel->removeInvited(client->getNickname());
    }

    channel->addClient(client);
    std::string joinMsg = ":" + client->getNickname() + " JOIN " + chanNameOnly + "\r\n";
    channel->broadcast(joinMsg);
    if (!channel->getTopic().empty())
        sendToClient(fd, "332 " + client->getNickname() + " " + chanNameOnly + " :" + channel->getTopic());
}
