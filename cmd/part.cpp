/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: domingos <domingos@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/15 16:16:32 by domingos          #+#    #+#             */
/*   Updated: 2026/03/15 16:16:33 by domingos         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::remove_to_chanel(Client *cl) {
  for (std::map<std::string, Channel *>::iterator it = _channels.begin();
     it != _channels.end();) {
    Channel *chan = it->second;
    std::string nick = cl->getNickname();
    bool wasInChannel = chan->hasClient(nick);
    chan->removeClient(cl);
    chan->removeOperator(nick);
    if (wasInChannel && !chan->isEmpty()) {
      std::string partMsg = ":" + nick + " QUIT :Client disconnected\r\n";
      chan->broadcast(partMsg);
    }
    if (chan->isEmpty()) {
      delete chan;
      _channels.erase(it++);
    } else {
      ++it;
    }
  }
}

void Server::cmdPart(int fd, const std::string &args)
{
  if (args.empty()) {
    sendToClient(fd, "461 PART :Not enough parameters");
    return;
  }

  std::string channelName = args;
  std::string reason = "";
  size_t sp = args.find(' ');
  if (sp != std::string::npos) {
    channelName = args.substr(0, sp);
    reason = args.substr(sp + 1);
    if (!reason.empty() && reason[0] == ':')
      reason.erase(0, 1);
  }

  Channel *channel = getChannel(channelName);
  if (!channel) {
    sendToClient(fd, "403 " + channelName + " :No such channel");
    return;
  }

  std::string nick = getNickByFd(fd);
  if (!channel->hasClient(nick)) {
    sendToClient(fd, "442 " + channelName + " :You're not on that channel");
    return;
  }

  std::string partMsg = ":" + nick + " PART " + channelName;
  if (!reason.empty())
    partMsg += " :" + reason;
  partMsg += "\r\n";
  channel->broadcast(partMsg);

  Client *client = _clients[fd];
  channel->removeClient(client);
  channel->removeOperator(nick);

  if (channel->isEmpty()) {
    delete channel;
    _channels.erase(channelName);
  }
}