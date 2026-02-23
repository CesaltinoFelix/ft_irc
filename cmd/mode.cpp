#include "Server.hpp"

void Server::cmdMode(int fd, const std::string& channel, const std::string& mode, const std::string& targetNick) {
    Channel* chan = getChannel(channel);
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
    } else if (mode == "+k") {
        if (targetNick.empty()) {
            sendToClient(fd, "461 MODE :Not enough parameters");
            return;
        }
        chan->setKey(targetNick);
        sendToClient(fd, ":" + nick + " MODE " + channel + " +k " + targetNick);
    } else if (mode == "-k") {
        chan->removeKey();
        sendToClient(fd, ":" + nick + " MODE " + channel + " -k");
    }

}