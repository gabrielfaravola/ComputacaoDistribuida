# Biblioteca Digital gRPC

Sistema de Gerenciamento de Biblioteca Digital distribuído, implementado com Java 17, Maven e gRPC, demonstrando os quatro tipos de comunicação RPC.

## Aluno(s)

> Preencha aqui: **Nome Completo — RA**

---

## Pré-requisitos

- Java 17+
- Maven 3.8+

---

## Compilar

```bash
mvn clean package -q
```

---

## Executar

Abra **dois terminais** na raiz do projeto.

**Terminal 1 — Servidor:**
```bash
java -jar target/biblioteca-grpc-1.0-SNAPSHOT-servidor.jar
```

**Terminal 2 — Cliente:**
```bash
java -jar target/biblioteca-grpc-1.0-SNAPSHOT-cliente.jar
```

---

## RPCs implementados

| # | Tipo | Método |
|---|------|--------|
| 1 | Unary | `cadastrarLivro` |
| 2 | Server Streaming | `listarLivrosPorAutor` |
| 3 | Client Streaming | `registrarEmprestimos` |
| 4 | Bidirectional Streaming | `chatBibliotecario` |

---

## Saída esperada (cliente)

```
=== SISTEMA DE GERENCIAMENTO DE BIBLIOTECA DIGITAL ===

--- Teste 1: Cadastrar 3 livros ---
[UNARY] Cadastrando: Effective Java (ISBN: 978-0134685991)
  → id=LIV-1 | status=CADASTRADO
[UNARY] Cadastrando: Clean Code (ISBN: 978-0132350884)
  → id=LIV-2 | status=CADASTRADO
[UNARY] Cadastrando: Redes de Computadores (ISBN: 978-8576059233)
  → id=LIV-3 | status=CADASTRADO

--- Teste 2: Listar livros de autor cadastrado ---
[SERVER STREAMING] Listando livros do autor: Joshua Bloch
  → id=LIV-1 | titulo=Effective Java | ano=2018 | isbn=978-0134685991

--- Teste 3: Listar livros de autor inexistente ---
[SERVER STREAMING] Listando livros do autor: Autor Inexistente
  → ERRO [NOT_FOUND]: Nenhum livro encontrado para o autor: Autor Inexistente

--- Teste 4: Registrar 5 empréstimos ---
[CLIENT STREAMING] Registrando 5 empréstimos...
  Enviando empréstimo: usuario=Alice livro_id=LIV-1
  ...
  → Total registrado: 5 empréstimos | Tempo: Xms

--- Teste 5: Chat com bibliotecário virtual ---
[BIDIRECTIONAL STREAMING] Chat com bibliotecário virtual...
  → Enviando: usuario=Usuario1 palavra_chave=java
  ← Sugestão para 'java': "Effective Java" — Joshua Bloch
  ...

--- Teste 6: Cadastrar livro com ISBN duplicado ---
[UNARY] Cadastrando: Effective Java (duplicado) (ISBN: 978-0134685991)
  → ERRO [ALREADY_EXISTS]: Livro com ISBN 978-0134685991 já cadastrado
```

---

## Estrutura do projeto

```
biblioteca-grpc/
├── pom.xml
└── src/main/
    ├── java/br/mackenzie/biblioteca/
    │   ├── server/
    │   │   ├── BibliotecaServiceImpl.java
    │   │   └── ServidorBiblioteca.java
    │   └── client/
    │       └── ClienteBiblioteca.java
    └── proto/
        └── biblioteca.proto
```
