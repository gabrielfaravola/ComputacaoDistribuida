package br.mackenzie.biblioteca.server;

import io.grpc.Server;
import io.grpc.ServerBuilder;

import java.io.IOException;
import java.util.concurrent.TimeUnit;

public class ServidorBiblioteca {

    private static final int PORTA = 50051;
    private Server server;

    public void iniciar() throws IOException {
        server = ServerBuilder.forPort(PORTA)
                .addService(new BibliotecaServiceImpl())
                .build()
                .start();

        System.out.println("Servidor Biblioteca iniciado na porta " + PORTA);

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("Encerrando servidor...");
            try {
                ServidorBiblioteca.this.parar();
            } catch (InterruptedException e) {
                e.printStackTrace(System.err);
            }
        }));
    }

    public void parar() throws InterruptedException {
        if (server != null) {
            server.shutdown().awaitTermination(30, TimeUnit.SECONDS);
        }
    }

    public void aguardarTermino() throws InterruptedException {
        if (server != null) {
            server.awaitTermination();
        }
    }

    public static void main(String[] args) throws IOException, InterruptedException {
        ServidorBiblioteca servidor = new ServidorBiblioteca();
        servidor.iniciar();
        servidor.aguardarTermino();
    }
}
