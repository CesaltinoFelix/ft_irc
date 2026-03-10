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
        std::vector<std::string> _operators;
        std::string _key;
        int _limit;
        bool _inviteOnly;
        bool _topicRestricted;
        std::string _topic;
        std::vector<std::string> _invitedUsers;
    public:
        Channel(const std::string& name);
        ~Channel();

        void addClient(Client* client);
        void removeClient(Client* client);
        void broadcast(const std::string& message);
        bool isEmpty() const;
        const std::string& getName() const;
        void addOperator(const std::string& nickname);
        bool isOperator(const std::string& nickname) const;
        void removeOperator(const std::string& nickname);

        void setKey(const std::string& key);
        void removeKey();
        bool hasKey() const;
        bool checkKey(const std::string& key) const;
        const std::string& getKey() const;

        void setLimit(int limit);
        void removeLimit();
        bool hasLimit() const;
        int getLimit() const;
        bool isFull() const;

        void setInviteOnly(bool value);
        bool isInviteOnly() const;
        void addInvited(const std::string& nick);
        void removeInvited(const std::string& nick);
        bool isInvited(const std::string& nick) const;

        void setTopicRestricted(bool value);
        bool isTopicRestricted() const;
        void setTopic(const std::string& topic);
        const std::string& getTopic() const;

        bool hasClient(const std::string& nick) const;
        const std::vector<Client*>& getClients() const;
};
#endif
