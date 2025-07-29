#include <stdio.h>
#include <string.h>

#define MAX_TAREFAS 10
#define TAM_DESC 100

char tarefas[MAX_TAREFAS][TAM_DESC];
int total = 0;

void adicionar() {
    if (total >= MAX_TAREFAS) {
        printf("Limite de tarefas atingido.\n");
        return;
    }

    printf("Digite a descricao da tarefa: ");
    getchar();
    fgets(tarefas[total], TAM_DESC, stdin);
    tarefas[total][strcspn(tarefas[total], "\n")] = '\0';

    total++;
    printf("Tarefa adicionada com sucesso!\n\n");
}

void listar() {
    if (total == 0) {
        printf("Nenhuma tarefa cadastrada.\n\n");
        return;
    }

    printf("TAREFAS:\n");
    for (int i = 0; i < total; i++) {
        printf("%d - %s\n", i + 1, tarefas[i]);
    }
    printf("\n");
}

void excluir() {
    int num;
    printf("Digite o numero da tarefa para excluir: ");
    scanf("%d", &num);

    if (num < 1 || num > total) {
        printf("Numero invalido.\n\n");
        return;
    }

    for (int i = num - 1; i < total - 1; i++) {
        strcpy(tarefas[i], tarefas[i + 1]);
    }

    total--;
    printf("Tarefa excluida com sucesso!\n\n");
}

int main() {
    int opcao;

    do {
        printf("=== MENU ===\n");
        printf("1 - Adicionar Tarefa\n");
        printf("2 - Listar Tarefas\n");
        printf("3 - Excluir Tarefa\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        if (opcao == 1) {
            adicionar();
        } else if (opcao == 2) {
            listar();
        } else if (opcao == 3) {
            excluir();
        } else if (opcao == 0) {
            printf("Saindo...\n");
        } else {
            printf("Opcao invalida.\n\n");
        }

    } while (opcao != 0);

    return 0;
}
