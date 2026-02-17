# ft_irc - Servidor IRC Simples

Um servidor IRC básico desenvolvido em C++98 para aprender conceitos de:
- Programação em rede (sockets)
- Comunicação TCP/IP
- Desenvolvimento de servidores

## Como compilar

```bash
make
```

## Como executar

```bash
./ircserv <port> <password>
```

**Exemplo:**
```bash
./ircserv 6667 mypassword
```

- **port**: Porta onde o servidor vai escutar (use uma acima de 1024)
- **password**: Senha necessária para conectar ao servidor

## Teste rápido

Em outro terminal, você pode conectar usando:

```bash
nc localhost 6667
```

## Estrutura Básica

- `main.cpp` - Programa principal que valida argumentos e cria o servidor
- `Server.hpp` - Declaração da classe Server
- `Server.cpp` - Implementação da classe Server

## O que este código faz

1. **main.cpp**: Valida os argumentos (porta e senha) e cria um objeto Server
2. **Server::init()**: Cria o socket, faz bind na porta e coloca em modo de escuta
3. **Server::run()**: Loop infinito que aceita conexões
4. **Server::acceptConnection()**: Aceita uma conexão e fecha (versão básica)

## Próximos passos para expandir

- [x] Armazenar múltiplas conexões (usar vetor ou lista)
- [x] Usar `select()` ou `poll()` para gerenciar múltiplos clientes
- [ ] Implementar protocolo IRC (comandos, canais, usuários)
- [ ] Criar classes Client e Channel
- [x] Implementar autenticação com senha (comando PASS)

## Domigos-> Trabalhei nestas 6 funcoes e descrevei o que cada uma faz
1 - void set_nickname(std::string cmd, int fd, bool id);
2 - void set_username(std::string &username, int fd , bool id);
3 - void cmdJoin(int fd, const std::string& channelName);
4-  void cmdPrivmsg(int fd, const std::string &target, const std::string &message); 
5 - void cmdPrivmsg_to_client(int fd, const std::string &target, const std::string &message);
6-  void cmd_execute(std::string cmd, std::string args, int fd);

1 - Reposnsavel por setar o nick_name do cliente durante o cadastro.
2 - Responsavel por setar o user_name do cliente durante o cadastro.
3 - Aqui esta funcao trabalha com a classe chanell que eu criei esta mesma funcao cria um denterminado canal e adiciona um cliente ao canal.
4 - Esta função é responsável por mandar uma mensagem a todos os clientes conectados num canal
5 - Esta função é responsável por mandar mensagem no privado em um determinado cliente.
6 - Esta funcão criei-a com o intuito de executar todos os comandos passados pelos clientes

