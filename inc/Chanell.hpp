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

        // Métodos para limite (+l)
        void setLimit(int limit);                 // define limite
        void removeLimit();                       // remove limite
        bool hasLimit() const;                    // verifica se há limite
        int getLimit() const;                     // retorna limite
        bool isFull() const;                      // verifica se canal está cheio
};
#endif
