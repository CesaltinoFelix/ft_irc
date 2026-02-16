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
    // Se canal não existe → cria
    if (_channels.find(channelName) == _channels.end())
    {
        _channels[channelName] = new Channel(channelName);
    }
    Channel *channel = _channels[channelName];
    channel->addClient(client);
    std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
    channel->broadcast(joinMsg);
}
