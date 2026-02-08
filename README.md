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
- [ ] Implementar autenticação com senha