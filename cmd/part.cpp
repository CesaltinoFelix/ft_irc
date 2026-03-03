#include "Server.hpp"
void Server::remove_to_chanel(Client *cl) {
    Channel *chan;
  for (std::map<std::string, Channel *>::iterator it = _channels.begin();
       it != _channels.end();) {
     chan = it->second;
    chan->removeClient(cl);
    if (chan->isEmpty()) {
      delete chan;
      std::map<std::string, Channel *>::iterator toErase = it++;
      _channels.erase(toErase);
    } else {
      ++it;
    }
  }
  chan->broadcast("removided client this channel");
}

void Server::cmdPart(int fd, const std::string &channelName) {
  Client *client = _clients[fd];

  if (!client->isAuthenticated() || !client->get_nick() ||
      !client->get_user()) {
    sendToClient(fd, "451 :You have not registered");
    return;
  }

  if (channelName.empty()) {
    sendToClient(fd, "461 PART :Not enough parameters");
    return;
  }

  if (_channels.find(channelName) == _channels.end()) {
    sendToClient(fd, "403 " + channelName + " :No such channel");
    return;
  }

  Channel *channel = _channels[channelName];
  channel->removeClient(client);

  std::string partMsg =
      ":" + client->getNickname() + " PART " + channelName + "\r\n";
  channel->broadcast(partMsg);

  if (channel->isEmpty()) {
    delete channel;
    _channels.erase(channelName);
  }

}