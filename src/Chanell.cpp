#include "../inc/Chanell.hpp"
Channel::Channel(const std::string& name) : _name(name) {}

Channel::~Channel() {}

void Channel::addClient(Client* client)
{
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


