package br.mackenzie.biblioteca.client;

import br.mackenzie.biblioteca.*;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;
import io.grpc.stub.StreamObserver;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class ClienteBiblioteca {

    private static final String HOST  = "localhost";
    private static final int    PORTA = 50051;

    private final ManagedChannel channel;
    private final BibliotecaServiceGrpc.BibliotecaServiceBlockingStub  bloqueante;
    private final BibliotecaServiceGrpc.BibliotecaServiceStub           assincrono;

    public ClienteBiblioteca() {
        channel = ManagedChannelBuilder.forAddress(HOST, PORTA)
                .usePlaintext()
                .build();
        bloqueante = BibliotecaServiceGrpc.newBlockingStub(channel);
        assincrono = BibliotecaServiceGrpc.newStub(channel);
    }

    public void encerrar() throws InterruptedException {
        channel.shutdown().awaitTermination(5, TimeUnit.SECONDS);
    }

    // ── 1) Unary – cadastrarLivro ──────────────────────────────────────────────
    private void cadastrarLivro(String titulo, String autor, int ano, String isbn) {
        System.out.printf("%n[UNARY] Cadastrando: %s (ISBN: %s)%n", titulo, isbn);
        try {
            CadastrarLivroResponse resp = bloqueante.cadastrarLivro(
                    CadastrarLivroRequest.newBuilder()
                            .setTitulo(titulo)
                            .setAutor(autor)
                            .setAno(ano)
                            .setIsbn(isbn)
                            .build());
            System.out.printf("  → id=%s | status=%s%n", resp.getId(), resp.getStatus());
        } catch (StatusRuntimeException e) {
            System.out.printf("  → ERRO [%s]: %s%n", e.getStatus().getCode(), e.getStatus().getDescription());
        }
    }

    // ── 2) Server Streaming – listarLivrosPorAutor ────────────────────────────
    private void listarLivrosPorAutor(String autor) {
        System.out.printf("%n[SERVER STREAMING] Listando livros do autor: %s%n", autor);
        try {
            bloqueante.listarLivrosPorAutor(
                    ListarLivrosRequest.newBuilder().setAutor(autor).build()
            ).forEachRemaining(livro ->
                    System.out.printf("  → id=%s | titulo=%s | ano=%d | isbn=%s%n",
                            livro.getId(), livro.getTitulo(), livro.getAno(), livro.getIsbn())
            );
        } catch (StatusRuntimeException e) {
            System.out.printf("  → ERRO [%s]: %s%n", e.getStatus().getCode(), e.getStatus().getDescription());
        }
    }

    // ── 3) Client Streaming – registrarEmprestimos ────────────────────────────
    private void registrarEmprestimos() throws InterruptedException {
        System.out.println("\n[CLIENT STREAMING] Registrando 5 empréstimos...");

        CountDownLatch concluido = new CountDownLatch(1);

        StreamObserver<Emprestimo> requestObserver = assincrono.registrarEmprestimos(
                new StreamObserver<>() {
                    @Override
                    public void onNext(ResumoEmprestimos resumo) {
                        System.out.printf("  → Total registrado: %d empréstimos | Tempo: %dms%n",
                                resumo.getTotal(), resumo.getTempoProcessamentoMs());
                    }
                    @Override public void onError(Throwable t) {
                        System.err.println("  → ERRO: " + t.getMessage());
                        concluido.countDown();
                    }
                    @Override public void onCompleted() { concluido.countDown(); }
                });

        String[][] emprestimos = {
                {"Alice",   "LIV-1"},
                {"Bob",     "LIV-2"},
                {"Carlos",  "LIV-1"},
                {"Diana",   "LIV-3"},
                {"Eduardo", "LIV-2"}
        };

        for (String[] e : emprestimos) {
            System.out.printf("  Enviando empréstimo: usuario=%s livro_id=%s%n", e[0], e[1]);
            requestObserver.onNext(Emprestimo.newBuilder()
                    .setUsuario(e[0])
                    .setLivroId(e[1])
                    .build());
        }
        requestObserver.onCompleted();
        concluido.await(10, TimeUnit.SECONDS);
    }

    // ── 4) Bidirectional Streaming – chatBibliotecario ────────────────────────
    private void chatBibliotecario() throws InterruptedException {
        System.out.println("\n[BIDIRECTIONAL STREAMING] Chat com bibliotecário virtual...");

        CountDownLatch concluido = new CountDownLatch(1);

        StreamObserver<MensagemChat> requestObserver = assincrono.chatBibliotecario(
                new StreamObserver<>() {
                    @Override
                    public void onNext(SugestaoLivro sugestao) {
                        System.out.printf("  ← Sugestão para '%s': \"%s\" — %s%n",
                                sugestao.getPalavraChave(), sugestao.getSugestao(), sugestao.getAutor());
                    }
                    @Override public void onError(Throwable t) {
                        System.err.println("  → ERRO: " + t.getMessage());
                        concluido.countDown();
                    }
                    @Override public void onCompleted() { concluido.countDown(); }
                });

        String[][] mensagens = {
                {"Usuario1", "java"},
                {"Usuario1", "distribuido"},
                {"Usuario1", "algoritmo"}
        };

        for (String[] m : mensagens) {
            System.out.printf("  → Enviando: usuario=%s palavra_chave=%s%n", m[0], m[1]);
            requestObserver.onNext(MensagemChat.newBuilder()
                    .setUsuario(m[0])
                    .setPalavraChave(m[1])
                    .build());
        }
        requestObserver.onCompleted();
        concluido.await(10, TimeUnit.SECONDS);
    }

    // ── main ──────────────────────────────────────────────────────────────────
    public static void main(String[] args) throws InterruptedException {
        ClienteBiblioteca cliente = new ClienteBiblioteca();

        try {
            System.out.println("=== SISTEMA DE GERENCIAMENTO DE BIBLIOTECA DIGITAL ===");

            // ── Teste 1: Cadastrar 3 livros (Unary) ───────────────────────────
            System.out.println("\n--- Teste 1: Cadastrar 3 livros ---");
            cliente.cadastrarLivro("Effective Java",              "Joshua Bloch",      2018, "978-0134685991");
            cliente.cadastrarLivro("Clean Code",                  "Robert C. Martin",  2008, "978-0132350884");
            cliente.cadastrarLivro("Redes de Computadores",       "Andrew Tanenbaum",  2011, "978-8576059233");

            // ── Teste 2: Listar livros de autor cadastrado (Server Streaming) ─
            System.out.println("\n--- Teste 2: Listar livros de autor cadastrado ---");
            cliente.listarLivrosPorAutor("Joshua Bloch");

            // ── Teste 3: Listar livros de autor inexistente ───────────────────
            System.out.println("\n--- Teste 3: Listar livros de autor inexistente ---");
            cliente.listarLivrosPorAutor("Autor Inexistente");

            // ── Teste 4: Registrar 5 empréstimos (Client Streaming) ───────────
            System.out.println("\n--- Teste 4: Registrar 5 empréstimos ---");
            cliente.registrarEmprestimos();

            // ── Teste 5: Chat com 3 mensagens (Bidirectional Streaming) ───────
            System.out.println("\n--- Teste 5: Chat com bibliotecário virtual ---");
            cliente.chatBibliotecario();

            // ── Teste 6: ISBN duplicado (deve retornar ALREADY_EXISTS) ────────
            System.out.println("\n--- Teste 6: Cadastrar livro com ISBN duplicado ---");
            cliente.cadastrarLivro("Effective Java (duplicado)", "Joshua Bloch", 2018, "978-0134685991");

        } finally {
            cliente.encerrar();
        }
    }
}
