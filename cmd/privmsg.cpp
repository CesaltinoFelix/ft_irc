/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: domingos <domingos@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/15 16:16:18 by domingos          #+#    #+#             */
/*   Updated: 2026/03/15 16:16:19 by domingos         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::cmdPrivmsg(int fd, const std::string &target, const std::string &message)
{
    Client *sender = _clients[fd];
    if (target.empty())
    {
        sendToClient(fd, "461 PRIVMSG :Not enough parameters");
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
        for (size_t i = 0; i < channel->getClients().size(); i++)
        {
            if (channel->getClients()[i]->getFd() != fd)
                send(channel->getClients()[i]->getFd(), fullMessage.c_str(), fullMessage.length(), MSG_NOSIGNAL);
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
