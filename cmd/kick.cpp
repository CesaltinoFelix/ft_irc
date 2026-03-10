#include "Server.hpp"

void Server::cmdKick(int fd, const std::string &channelName, const std::string &client_to_kick)
{
  if (channelName.empty() || client_to_kick.empty()) {
    sendToClient(fd, "461 KICK :Not enough parameters");
    return;
  }

  std::map<int, Client *>::iterator citer = _clients.find(fd);
  if (citer == _clients.end())
    return;
  Client *caller = citer->second;
  if (!caller->isAuthenticated() || !caller->get_nick() || !caller->get_user()) {
    sendToClient(fd, "451 :You have not registered");
    return;
  }

  Channel *channel = getChannel(channelName);
  if (!channel) {
    sendToClient(fd, "403 " + channelName + " :No such channel");
    return;
  }

  std::string callerNick = getNickByFd(fd);
  if (!channel->hasClient(callerNick)) {
    sendToClient(fd, "442 " + channelName + " :You're not on that channel");
    return;
  }
  if (!channel->isOperator(callerNick)) {
    sendToClient(fd, "482 " + channelName + " :You're not channel operator");
    return;
  }
  int targetFd = getFdByNick(client_to_kick);
  if (targetFd == -1) {
    sendToClient(fd, "401 " + client_to_kick + " :No such nick");
    return;
  }

  if (!channel->hasClient(client_to_kick)) {
    sendToClient(fd, "441 " + client_to_kick + " " + channelName + " :They aren't on that channel");
    return;
  }

  Client *target = _clients[targetFd];
  channel->removeClient(target);
  channel->removeOperator(client_to_kick);

  std::string kickMsg = ":" + callerNick + " KICK " + channelName + " " + client_to_kick + "\r\n";
  channel->broadcast(kickMsg);
  sendToClient(targetFd, ":" + callerNick + " KICK " + channelName + " " + client_to_kick);

  if (channel->isEmpty()) {
    delete channel;
    _channels.erase(channelName);
  }
}