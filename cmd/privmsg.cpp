#include "Server.hpp"

void Server::cmdPrivmsg(int fd, const std::string &target, const std::string &message)
{
    Client *sender = _clients[fd];
    if (target.empty() || message.empty())
    {
        sendToClient(fd, "412 :No text to send");
        return;
    }
    if (target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
        {
            sendToClient(fd, "403 " + target + " :No such channel");
            return;
        }

        Channel *channel = _channels[target];

        if (!channel->hasClient(sender->getNickname()))
        {
            sendToClient(fd, "404 " + target + " :Cannot send to channel (not a member)");
            return;
        }

        std::string fullMessage = ":" + sender->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        // Send to all channel members except sender
        for (size_t i = 0; i < channel->getClients().size(); i++)
        {
            if (channel->getClients()[i]->getFd() != fd)
                send(channel->getClients()[i]->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
        }
    }
    else
    {
        sendToClient(fd, "401 " + target + " :No such nick");
    }
}

void Server::cmdPrivmsg_to_client(int fd, const std::string &target, const std::string &message)
{
    Client* sender = _clients[fd];

    for (std::map<int, Client*>::iterator it = _clients.begin();
         it != _clients.end();
         ++it)
    {
        Client* receiver = it->second;

        if (receiver->getNickname() == target)
        {
            sendToClient(receiver->getFd(),
                ":" + sender->getNickname() +
                " PRIVMSG " + target +
                " :" + message);
            return;
        }
    }
    sendToClient(fd, "401 " + target + " :No such nick/channel");
}
