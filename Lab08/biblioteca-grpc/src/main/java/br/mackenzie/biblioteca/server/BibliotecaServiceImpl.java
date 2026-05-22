package br.mackenzie.biblioteca.server;

import br.mackenzie.biblioteca.*;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

public class BibliotecaServiceImpl extends BibliotecaServiceGrpc.BibliotecaServiceImplBase {

    // ── Persistência em memória ────────────────────────────────────────────────
    private final Map<String, Livro> acervo = new HashMap<>();   // id  -> Livro
    private final Map<String, String> isbnIndex = new HashMap<>(); // isbn -> id
    private final AtomicInteger contadorId = new AtomicInteger(1);

    // Tabela simples para o chat bidirecional: palavra-chave -> sugestão
    private static final Map<String, String[]> SUGESTOES = new LinkedHashMap<>();
    static {
        SUGESTOES.put("java",        new String[]{"Effective Java",             "Joshua Bloch"});
        SUGESTOES.put("redes",       new String[]{"Redes de Computadores",       "Andrew Tanenbaum"});
        SUGESTOES.put("banco",       new String[]{"Sistemas de Banco de Dados",  "Ramez Elmasri"});
        SUGESTOES.put("algoritmo",   new String[]{"Algoritmos: Teoria e Prática","Thomas Cormen"});
        SUGESTOES.put("grpc",        new String[]{"gRPC: Up and Running",        "Kasun Indrasiri"});
        SUGESTOES.put("distribuido", new String[]{"Designing Distributed Systems","Brendan Burns"});
        SUGESTOES.put("python",      new String[]{"Python Fluente",              "Luciano Ramalho"});
        SUGESTOES.put("segurança",   new String[]{"The Web Application Hacker's Handbook","Dafydd Stuttard"});
    }

    // ── 1) Unary – cadastrarLivro ──────────────────────────────────────────────
    @Override
    public void cadastrarLivro(CadastrarLivroRequest request,
                               StreamObserver<CadastrarLivroResponse> responseObserver) {

        System.out.printf("[cadastrarLivro] titulo=%s | autor=%s | ano=%d | isbn=%s%n",
                request.getTitulo(), request.getAutor(), request.getAno(), request.getIsbn());

        // Validação de campos obrigatórios
        if (request.getTitulo().isBlank() || request.getAutor().isBlank() || request.getIsbn().isBlank()) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("titulo, autor e isbn são obrigatórios")
                    .asRuntimeException());
            return;
        }

        // Verifica ISBN duplicado
        if (isbnIndex.containsKey(request.getIsbn())) {
            responseObserver.onError(Status.ALREADY_EXISTS
                    .withDescription("Livro com ISBN " + request.getIsbn() + " já cadastrado")
                    .asRuntimeException());
            return;
        }

        String id = "LIV-" + contadorId.getAndIncrement();
        Livro livro = Livro.newBuilder()
                .setId(id)
                .setTitulo(request.getTitulo())
                .setAutor(request.getAutor())
                .setAno(request.getAno())
                .setIsbn(request.getIsbn())
                .build();

        acervo.put(id, livro);
        isbnIndex.put(request.getIsbn(), id);

        System.out.printf("[cadastrarLivro] Livro cadastrado com id=%s%n", id);

        responseObserver.onNext(CadastrarLivroResponse.newBuilder()
                .setId(id)
                .setStatus("CADASTRADO")
                .build());
        responseObserver.onCompleted();
    }

    // ── 2) Server Streaming – listarLivrosPorAutor ────────────────────────────
    @Override
    public void listarLivrosPorAutor(ListarLivrosRequest request,
                                     StreamObserver<Livro> responseObserver) {

        System.out.printf("[listarLivrosPorAutor] autor=%s%n", request.getAutor());

        List<Livro> encontrados = acervo.values().stream()
                .filter(l -> l.getAutor().equalsIgnoreCase(request.getAutor()))
                .toList();

        if (encontrados.isEmpty()) {
            responseObserver.onError(Status.NOT_FOUND
                    .withDescription("Nenhum livro encontrado para o autor: " + request.getAutor())
                    .asRuntimeException());
            return;
        }

        for (Livro livro : encontrados) {
            System.out.printf("[listarLivrosPorAutor] enviando id=%s titulo=%s%n",
                    livro.getId(), livro.getTitulo());
            responseObserver.onNext(livro);
        }
        responseObserver.onCompleted();
    }

    // ── 3) Client Streaming – registrarEmprestimos ────────────────────────────
    @Override
    public StreamObserver<Emprestimo> registrarEmprestimos(
            StreamObserver<ResumoEmprestimos> responseObserver) {

        long inicio = System.currentTimeMillis();
        List<Emprestimo> recebidos = new ArrayList<>();

        return new StreamObserver<>() {
            @Override
            public void onNext(Emprestimo emprestimo) {
                System.out.printf("[registrarEmprestimos] usuario=%s | livro_id=%s%n",
                        emprestimo.getUsuario(), emprestimo.getLivroId());
                recebidos.add(emprestimo);
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("[registrarEmprestimos] Erro: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                long tempo = System.currentTimeMillis() - inicio;
                System.out.printf("[registrarEmprestimos] Concluído: %d empréstimos em %dms%n",
                        recebidos.size(), tempo);

                responseObserver.onNext(ResumoEmprestimos.newBuilder()
                        .setTotal(recebidos.size())
                        .setTempoProcessamentoMs(tempo)
                        .build());
                responseObserver.onCompleted();
            }
        };
    }

    // ── 4) Bidirectional Streaming – chatBibliotecario ────────────────────────
    @Override
    public StreamObserver<MensagemChat> chatBibliotecario(
            StreamObserver<SugestaoLivro> responseObserver) {

        return new StreamObserver<>() {
            @Override
            public void onNext(MensagemChat mensagem) {
                System.out.printf("[chatBibliotecario] usuario=%s | palavra_chave=%s%n",
                        mensagem.getUsuario(), mensagem.getPalavraChave());

                String chave = mensagem.getPalavraChave().toLowerCase();

                // Busca a palavra-chave ou retorna sugestão padrão
                String[] sugestao = SUGESTOES.entrySet().stream()
                        .filter(e -> chave.contains(e.getKey()))
                        .map(Map.Entry::getValue)
                        .findFirst()
                        .orElse(new String[]{"Clean Code", "Robert C. Martin"});

                responseObserver.onNext(SugestaoLivro.newBuilder()
                        .setPalavraChave(mensagem.getPalavraChave())
                        .setSugestao(sugestao[0])
                        .setAutor(sugestao[1])
                        .build());
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("[chatBibliotecario] Erro: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                System.out.println("[chatBibliotecario] Chat encerrado.");
                responseObserver.onCompleted();
            }
        };
    }
}
