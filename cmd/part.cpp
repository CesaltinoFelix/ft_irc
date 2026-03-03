#include "Server.hpp"

void Server::remove_to_chanel(Client *cl) {
  for (std::map<std::string, Channel *>::iterator it = _channels.begin();
     it != _channels.end();) {
    Channel *chan = it->second;
    chan->removeClient(cl);
    if (chan->isEmpty()) {
      delete chan;
      std::map<std::string, Channel *>::iterator toErase = it++;
      _channels.erase(toErase);
    } else {
      ++it;
    }
  }
}