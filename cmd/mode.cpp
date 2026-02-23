#include "Server.hpp"

void Server::cmdMode(int fd, const std::string& channel, const std::string& mode, const std::string& targetNick) {
    Chanell* chan = getChannel(channel);
    std::string nick = getNickByFd(fd);
    if (!chan || !chan->isOperator(nick)) {
        sendToClient(fd, "482 " + channel + " :You're not channel operator");
        return;
    }
    if (mode == "+o") {
        chan->addOperator(targetNick);
        sendToClient(fd, ":" + nick + " MODE " + channel + " +o " + targetNick);
    } else if (mode == "-o") {
        chan->removeOperator(targetNick);
        sendToClient(fd, ":" + nick + " MODE " + channel + " -o " + targetNick);
    }

}