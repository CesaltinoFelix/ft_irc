#include "Server.hpp"
#include <cstdlib>

void Server::cmdMode(int fd, const std::string& channel, const std::string& mode, const std::string& targetNick) {
    Channel* chan = getChannel(channel);
    std::string nick = getNickByFd(fd);

    if (!chan) {
        sendToClient(fd, "403 " + channel + " :No such channel");
        return;
    }

    if (!chan->hasClient(nick)) {
        sendToClient(fd, "442 " + channel + " :You're not on that channel");
        return;
    }

    if (!chan->isOperator(nick)) {
        sendToClient(fd, "482 " + channel + " :You're not channel operator");
        return;
    }

    std::string modeMsg;

    if (mode == "-l") {
        chan->removeLimit();
        modeMsg = ":" + nick + " MODE " + channel + " -l\r\n";
        chan->broadcast(modeMsg);
        return;
    } else if (mode == "+l") {
        if (targetNick.empty()) {
            sendToClient(fd, "461 MODE :Not enough parameters");
            return;
        }
        for (size_t i = 0; i < targetNick.size(); i++) {
            if (!isdigit(targetNick[i])) {
                sendToClient(fd, "472 " + targetNick + " :Invalid limit");
                return;
            }
        }
        long limit = strtol(targetNick.c_str(), NULL, 10);
        if (limit <= 0 || limit > 2147483647L) {
            sendToClient(fd, "472 " + targetNick + " :Invalid limit");
            return;
        }
        chan->setLimit(static_cast<int>(limit));
        modeMsg = ":" + nick + " MODE " + channel + " +l " + targetNick + "\r\n";
        chan->broadcast(modeMsg);
        return;
    }

    if (mode == "+o") {
        if (targetNick.empty()) {
            sendToClient(fd, "461 MODE :Not enough parameters");
            return;
        }
        if (!chan->hasClient(targetNick)) {
            sendToClient(fd, "441 " + targetNick + " " + channel + " :They aren't on that channel");
            return;
        }
        chan->addOperator(targetNick);
        modeMsg = ":" + nick + " MODE " + channel + " +o " + targetNick + "\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "-o") {
        if (targetNick.empty()) {
            sendToClient(fd, "461 MODE :Not enough parameters");
            return;
        }
        if (!chan->hasClient(targetNick)) {
            sendToClient(fd, "441 " + targetNick + " " + channel + " :They aren't on that channel");
            return;
        }
        chan->removeOperator(targetNick);
        modeMsg = ":" + nick + " MODE " + channel + " -o " + targetNick + "\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "+k") {
        if (targetNick.empty()) {
            sendToClient(fd, "461 MODE :Not enough parameters");
            return;
        }
        chan->setKey(targetNick);
        modeMsg = ":" + nick + " MODE " + channel + " +k " + targetNick + "\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "-k") {
        chan->removeKey();
        modeMsg = ":" + nick + " MODE " + channel + " -k\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "+i") {
        chan->setInviteOnly(true);
        modeMsg = ":" + nick + " MODE " + channel + " +i\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "-i") {
        chan->setInviteOnly(false);
        modeMsg = ":" + nick + " MODE " + channel + " -i\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "+t") {
        chan->setTopicRestricted(true);
        modeMsg = ":" + nick + " MODE " + channel + " +t\r\n";
        chan->broadcast(modeMsg);
    } else if (mode == "-t") {
        chan->setTopicRestricted(false);
        modeMsg = ":" + nick + " MODE " + channel + " -t\r\n";
        chan->broadcast(modeMsg);
    } else {
        sendToClient(fd, "472 " + mode + " :is unknown mode char to me");
    }
}