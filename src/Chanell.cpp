#include "../inc/Chanell.hpp"
#include <sys/socket.h>
Channel::Channel(const std::string& name) : _name(name), _key(""), _limit(0), _inviteOnly(false), _topicRestricted(false), _topic("") {}
void Channel::setLimit(int limit) {
    _limit = limit;
}

void Channel::removeLimit() {
    _limit = 0;
}

bool Channel::hasLimit() const {
    return _limit > 0;
}

int Channel::getLimit() const {
    return _limit;
}

bool Channel::isFull() const {
    return hasLimit() && static_cast<int>(_clients.size()) >= _limit;
}

void Channel::setKey(const std::string& key) {
    _key = key;
}

void Channel::removeKey() {
    _key.clear();
}

bool Channel::hasKey() const {
    return !_key.empty();
}

bool Channel::checkKey(const std::string& key) const {
    return _key == key;
}

const std::string& Channel::getKey() const {
    return _key;
}

Channel::~Channel() {}

void Channel::addClient(Client* client)
{
    if(_operators.empty())
    {
        _operators.push_back(client->getNickname());
    }
    _clients.push_back(client);
}

void Channel::removeClient(Client* client)
{
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (*it == client)
        {
            _clients.erase(it);
            break;
        }
    }
}

void Channel::broadcast(const std::string& message)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        send(_clients[i]->getFd(), message.c_str(), message.length(), 0);
    }
}

bool Channel::isEmpty() const
{
    return _clients.empty();
}

const std::string& Channel::getName() const
{
    return _name;
}

void Channel::addOperator(const std::string& nickname)
{
    if (!isOperator(nickname))
    {
        _operators.push_back(nickname);
    }
}

void Channel::removeOperator(const std::string& nickname)
{
    for (std::vector<std::string>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (*it == nickname)
        {
            _operators.erase(it);
            break;
        }
    }
}

bool Channel::isOperator(const std::string& nickname) const
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == nickname)
        {
            return true;
        }
    }
    return false;
}

// Métodos para invite-only (+i)
void Channel::setInviteOnly(bool value) {
    _inviteOnly = value;
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

void Channel::addInvited(const std::string& nick) {
    for (size_t i = 0; i < _invitedUsers.size(); i++) {
        if (_invitedUsers[i] == nick)
            return;
    }
    _invitedUsers.push_back(nick);
}

void Channel::removeInvited(const std::string& nick) {
    for (std::vector<std::string>::iterator it = _invitedUsers.begin(); it != _invitedUsers.end(); ++it) {
        if (*it == nick) {
            _invitedUsers.erase(it);
            return;
        }
    }
}

bool Channel::isInvited(const std::string& nick) const {
    for (size_t i = 0; i < _invitedUsers.size(); i++) {
        if (_invitedUsers[i] == nick)
            return true;
    }
    return false;
}

// Métodos para tópico (+t)
void Channel::setTopicRestricted(bool value) {
    _topicRestricted = value;
}

bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

// Verifica se um cliente está no canal
bool Channel::hasClient(const std::string& nick) const {
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i]->getNickname() == nick)
            return true;
    }
    return false;
}

const std::vector<Client*>& Channel::getClients() const {
    return _clients;
}
