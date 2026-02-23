#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int codigo;
    char *nome;
    float preco;
    int quantidade;
} Produto;

Produto* adicionar_produto(Produto *vetor, int *indice){
    
    Produto *temp = realloc(vetor, (*indice + 1) * sizeof(Produto));
    if (temp == NULL){
        printf("Erro ao realocar memória!\n");
        return vetor;
    }

    vetor = temp;

    vetor[*indice].nome = malloc(100 * sizeof(char));
    if (vetor[*indice].nome == NULL){
        printf("Erro ao alocar memória para o nome!\n");
        return vetor;
    }

    printf("\n--- Adicionar Produto ---\n");

    vetor[*indice].codigo = *indice + 1;

    printf("Nome: ");
    scanf(" %[^\n]", vetor[*indice].nome);

    printf("Preço: ");
    scanf("%f", &vetor[*indice].preco);

    printf("Quantidade: ");
    scanf("%d", &vetor[*indice].quantidade);

    (*indice)++;

    printf("Produto cadastrado com sucesso!\n");

    return vetor;
}

void listar_produtos(Produto *vetor, int indice){
    
    if (indice == 0){
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    float valor_total_estoque = 0;
    
    printf("\n--- Lista de Produtos ---\n");
    printf("+----------------------------------------------------------------------------+\n");
    printf("| Codigo | Nome                 | Preco     | Quantidade     | Valor Estoque |\n");
    printf("+----------------------------------------------------------------------------+\n");
    
    for(int i = 0; i < indice; i++){
        
        float valor_produto = vetor[i].preco * vetor[i].quantidade;
        valor_total_estoque += valor_produto;
        
        printf("| %-6d | %-20s | %-9.2f | %-14d | %-13.2f |\n",
               vetor[i].codigo,
               vetor[i].nome,
               vetor[i].preco,
               vetor[i].quantidade,
               valor_produto);
    }

    printf("+----------------------------------------------------------------------------+\n");
    printf("Valor total do estoque: R$ %.2f\n", valor_total_estoque);
}

Produto* buscar_produto(Produto *vetor, int indice){
    int codigo;

    printf("Informe o código do produto: ");
    scanf("%d", &codigo);

    for(int i = 0; i < indice; i++){
        if(vetor[i].codigo == codigo){
            return &vetor[i];
        }
    }

    return NULL;
}

void atualizar_estoque(Produto *vetor, int indice){
    
    if (indice == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }
    
    printf("--- Atualizar Estoque ---\n");
    Produto *produto_atualizar = buscar_produto(vetor, indice);
    
    if(produto_atualizar != NULL){
        
        int nova_quantidade;
        printf("Nova quantidade: ");
        scanf("%d", &nova_quantidade);

        produto_atualizar->quantidade = nova_quantidade;

        printf("Estoque atualizado com sucesso!\n");

    } else {
        printf("Produto não encontrado.\n");
    }
}

void remover_produto(Produto **vetor, int *indice){

    if(*indice == 0){
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    printf("--- Remover Produto ---\n");
    Produto *produto_remover = buscar_produto(*vetor, *indice);

    if(produto_remover == NULL){
        printf("Produto não encontrado.\n");
        return;
    }

    int posicao = produto_remover - *vetor;

    free((*vetor)[posicao].nome);

    for(int i = posicao; i < *indice - 1; i++){
        (*vetor)[i] = (*vetor)[i + 1];
    }

    (*indice)--;

    if(*indice > 0){
        Produto *temp = realloc(*vetor, (*indice) * sizeof(Produto));
        if(temp != NULL){
            *vetor = temp;
        }
    } 
    else {
        free(*vetor);
        *vetor = NULL;
    }

    printf("Produto removido com sucesso!\n");
}

void liberar_memoria(Produto *produtos, int indice){
    for(int i = 0; i < indice; i++){
        free(produtos[i].nome);
    }
    free(produtos);
}

int ler_opcao() {
    int opcao;

    while (1) {
        printf("> ");

        if (scanf("%d", &opcao) == 1) {
            while (getchar() != '\n');
            return opcao;
        }

        printf("Entrada inválida! Digite apenas números.\n");
        while (getchar() != '\n');
    }
}

int main(){

    int opMenu;
    Produto *produtos = NULL;
    int indice = 0;

    printf("-------- Inicializando Sistema de Cadastro de Produtos --------\n");

    while(true){

        printf("\n--- Menu ---\n");
        printf("[1] Adicionar Produto\n");
        printf("[2] Listar Todos os Produtos\n");
        printf("[3] Buscar Produto por Código\n");
        printf("[4] Atualizar Estoque\n");
        printf("[5] Remover Produto\n");
        printf("[6] Sair do Programa\n\n");

        opMenu = ler_opcao();

        switch(opMenu){

            case 1:
                produtos = adicionar_produto(produtos, &indice);
                break;

            case 2:
                listar_produtos(produtos, indice);
                break;

            case 3: {
                Produto *encontrado = buscar_produto(produtos, indice);

                if(encontrado != NULL){

                    float valor_total = encontrado->preco * encontrado->quantidade;

                    printf("+----------------------------------------------------------------------------+\n");
                    printf("| Codigo | Nome                 | Preco     | Quantidade     | Valor Estoque |\n");
                    printf("+----------------------------------------------------------------------------+\n");

                    printf("| %-6d | %-20s | %-9.2f | %-14d | %-13.2f |\n",
                        encontrado->codigo,
                        encontrado->nome,
                        encontrado->preco,
                        encontrado->quantidade,
                        valor_total);

                    printf("+----------------------------------------------------------------------------+\n");

                } else {
                    printf("Produto não encontrado.\n");
                }

                break;
            }

            case 4:
                atualizar_estoque(produtos, indice);
                break;

            case 5:
                remover_produto(&produtos, &indice);
                break;

            case 6:
                liberar_memoria(produtos, indice);
                printf("Saindo do programa...\n");
                return 0;

            default:
                printf("Opção inválida!\n");
        }
    }
}