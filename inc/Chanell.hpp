#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <iostream>
#include "Client.hpp"
#include <vector>

class Channel
{
    private:
        std::string _name;
        std::vector<Client*> _clients;
    public:
        Channel(const std::string& name);
        ~Channel();

        void addClient(Client* client);
        void removeClient(Client* client);
        void broadcast(const std::string& message);
        bool isEmpty() const;
        const std::string& getName() const;
};
#endif
