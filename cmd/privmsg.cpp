#include "Server.hpp"

void Server::cmdPrivmsg(int fd, const std::string &target, const std::string &message)
{
    std::cout <<"=====> "<< message<<std::endl;
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
        std::string fullMessage = ":" + sender->getNickname()  + target + " :" + message + "\r\n";
        channel->broadcast(fullMessage);
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
