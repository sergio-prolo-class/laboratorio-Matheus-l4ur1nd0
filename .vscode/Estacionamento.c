#include <stdio.h>    
#include <stdlib.h>   // Para funções de propósito geral como system (para limpar a tela).
#include <string.h>   // Para manipulação de strings.
#include <time.h>     // Para funções de tempo
#include <ctype.h>    // Para funções de caracteres.

// Definições de constantes e cores para o terminal
#define MAX_PLACAS 100 // Capacidade máxima de veículos que o sistema pode armazenar.
#define ARQUIVO_COMPLETO "registros_completos.txt" // Nome do arquivo onde os dados serão persistidos.

#define TARIFA_POR_MINUTO 0.50f // Valor da tarifa por minuto de estacionamento.
#define TARIFA_MINIMA 1.00f     // Valor mínimo a ser cobrado, mesmo que o tempo seja curto.

// Códigos de cores ANSI para formatação do texto no terminal.
#define YELLOW  "\033[1;33m"
#define GREEN   "\033[1;32m"
#define RED     "\033[1;31m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

// Definição da estrutura Veiculo
// Representa um registro de veículo no estacionamento, contendo todas as suas informações.
typedef struct {
    char placa[8];     
    time_t entrada;    
    time_t saida;       
    float valor_pago;   
    int ativo;          
} Veiculo;

// Variáveis globais para armazenar os veículos e o contador total.
// Essas variáveis são acessíveis por todas as funções do programa.
Veiculo veiculos[MAX_PLACAS]; 
int total_veiculos = 0;       // Contador de quantos veículos estão registrados no array 'veiculos'.


// Função para limpar a tela do terminal.
// Usa 'system("cls")' para Windows e 'system("clear")' para sistemas baseados em Unix/Linux.
void limparTela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Função para pausar a execução do programa até o usuário pressionar Enter.
// Primeiramente, consome quaisquer caracteres remanescentes no buffer de entrada (como o '\n' deixado por sscanf).
// Em seguida, espera que o usuário pressione Enter.
void pause() {
    printf("\nPressione Enter para retornar...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
}


void lerLinha(char *buffer, int tamanho) {
    if (fgets(buffer, tamanho, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; 
    } else {
        buffer[0] = '\0'; // Em caso de erro, inicializa o buffer para evitar lixo.
    }
}


int lerInteiro() {
    char buffer[256];
    lerLinha(buffer, sizeof(buffer));
    int valor;
    if (sscanf(buffer, "%d", &valor) == 1) {
        return valor;
    }
    return -1; // Sinaliza valor inválido
}


int lerPlaca(char *placa_destino) {
    char placa_temp[10]; 
    lerLinha(placa_temp, sizeof(placa_temp)); 

    // Converte a placa para maiúsculas para facilitar a validação
    for (int i = 0; placa_temp[i]; i++) {
        placa_temp[i] = toupper(placa_temp[i]);
    }

    int len = strlen(placa_temp);

    // Validação do padrão antigo: LLL-NNNN (8 caracteres)
    if (len == 8 && placa_temp[3] == '-') {
        int valido = 1;
        for (int i = 0; i < 3; i++) { // Primeiros 3 caracteres devem ser letras
            if (!isalpha(placa_temp[i])) {
                valido = 0;
                break;
            }
        }
        if (valido) {
            for (int i = 4; i < 8; i++) { // Últimos 4 caracteres devem ser números
                if (!isdigit(placa_temp[i])) {
                    valido = 0;
                    break;
                }
            }
        }
        if (valido) {
            strcpy(placa_destino, placa_temp);
            return 1; // Placa antiga válida
        }
    }

    // Validação do padrão Mercosul: LLLNLNN (7 caracteres)
    if (len == 7) {
        int valido = 1;
        // Primeiros 3 caracteres devem ser letras
        for (int i = 0; i < 3; i++) {
            if (!isalpha(placa_temp[i])) {
                valido = 0;
                break;
            }
        }
        // Quarto caracter deve ser um número
        if (valido && !isdigit(placa_temp[3])) {
            valido = 0;
        }
        // Quinto caracter deve ser uma letra
        if (valido && !isalpha(placa_temp[4])) {
            valido = 0;
        }
        // Últimos 2 caracteres devem ser números
        if (valido) {
            for (int i = 5; i < 7; i++) {
                if (!isdigit(placa_temp[i])) {
                    valido = 0;
                    break;
                }
            }
        }
        if (valido) {
            strcpy(placa_destino, placa_temp);
            return 1; // Placa Mercosul válida
        }
    }

    return 0; // Placa inválida (não corresponde a nenhum padrão)
}


void salvarTodosVeiculos() {
    FILE *fc = fopen(ARQUIVO_COMPLETO, "w"); 
    if (fc == NULL) {
        perror(RED "Erro ao abrir o arquivo para escrita" RESET); // Imprime erro se não conseguir abrir.
        return;
    }
    // Percorre todos os veículos e escreve suas informações no arquivo, separadas por vírgula.
    for (int i = 0; i < total_veiculos; i++) {
        fprintf(fc, "%s,%ld,%ld,%.2f,%d\n",
            veiculos[i].placa,
            veiculos[i].entrada,
            veiculos[i].saida,
            veiculos[i].valor_pago,
            veiculos[i].ativo
        );
    }
    fclose(fc); // Fecha o arquivo.
}

// Carrega os dados dos veículos do arquivo ARQUIVO_COMPLETO para a memória.
// É chamada no início do programa para restaurar o estado anterior.
void carregarDados() {
    FILE *fc = fopen(ARQUIVO_COMPLETO, "r"); // Abre o arquivo em modo de leitura.
    if (fc == NULL) {
        printf(YELLOW "Nenhum registro anterior encontrado. Iniciando com dados vazios.\n" RESET); // Mensagem se o arquivo não existe.
        return;
    }

    total_veiculos = 0; // Reseta o contador para preencher com os dados do arquivo.
    // Lê os dados do arquivo linha por linha até o limite de MAX_PLACAS ou fim do arquivo.
    while (total_veiculos < MAX_PLACAS) {
        Veiculo v;
        // Tenta ler 5 campos formatados (placa, entrada, saida, valor_pago, ativo).
        if (fscanf(fc, "%7[^,],%ld,%ld,%f,%d\n", v.placa, &v.entrada, &v.saida, &v.valor_pago, &v.ativo) == 5) {
            veiculos[total_veiculos++] = v; // Adiciona o veículo lido ao array.
        } else {
            break; // Sai do loop se a leitura falhar (fim do arquivo ou formato incorreto).
        }
    }
    fclose(fc); // Fecha o arquivo.
    printf(GREEN "%d veículos carregados do histórico.\n" RESET, total_veiculos); // Informa quantos veículos foram carregados.
    pause(); // Pausa para o usuário ver a mensagem.
}

// Tenta registrar a entrada de um veículo com a placa fornecida.
// Retorna 1 em caso de sucesso (veículo registrado), 0 em caso de falha (capacidade máxima ou placa duplicada).
int tentarRegistrarEntrada(const char *placa_nova) {
    if (total_veiculos >= MAX_PLACAS) {
        return 0; // Capacidade máxima atingida.
    }

    // Verifica se já existe um veículo ativo com a mesma placa.
    for (int i = 0; i < total_veiculos; i++) {
        if (veiculos[i].ativo && strcmp(veiculos[i].placa, placa_nova) == 0) {
            return 0; // Veículo com essa placa já está estacionado.
        }
    }

    // Cria um novo registro de veículo e preenche seus dados iniciais.
    Veiculo novo;
    strcpy(novo.placa, placa_nova);
    novo.entrada = time(NULL); // Registra o tempo atual como entrada.
    novo.saida = 0;             // Saída é 0 pois ainda não saiu.
    novo.valor_pago = 0;        // Valor pago é 0 inicialmente.
    novo.ativo = 1;             // Marca como ativo (estacionado).

    veiculos[total_veiculos++] = novo; // Adiciona o novo veículo ao array.
    salvarTodosVeiculos();             // Salva o estado atualizado no arquivo.
    return 1;                          // Sucesso.
}

int obterIndiceVeiculoAtivo(const Veiculo *lista_veiculos, int num_veiculos) {
    int indices_ativos[MAX_PLACAS]; // Array para armazenar os índices dos veículos ativos.
    int count_ativos = 0;           // Contador de veículos ativos encontrados.

    // Exibe a lista de veículos atualmente estacionados.
    printf(YELLOW "VEÍCULOS ESTACIONADOS ATUALMENTE:\n" RESET);
    for (int i = 0; i < num_veiculos; i++) {
        if (lista_veiculos[i].ativo) {
            printf("  %d - Placa: " CYAN "%s" RESET "\n", count_ativos + 1, lista_veiculos[i].placa);
            indices_ativos[count_ativos] = i; // Armazena o índice real no array global.
            count_ativos++;
        }
    }

    if (count_ativos == 0) {
        printf(RED "Nenhum veículo estacionado no momento.\n" RESET);
        return -1; // Não há veículos ativos.
    }

    // Solicita ao usuário que selecione um veículo pelo número.
    printf("\nSelecione o veículo pelo número (0 para cancelar): ");
    int escolha = lerInteiro(); // Lê a escolha do usuário de forma segura.

    if (escolha == 0) {
        printf(YELLOW "Seleção cancelada.\n" RESET);
        return -1; // Usuário cancelou.
    }
    // Valida se a escolha está dentro do intervalo dos veículos ativos listados.
    if (escolha < 1 || escolha > count_ativos) {
        printf(RED "Número inválido. Por favor, escolha um número da lista.\n" RESET);
        return -1; // Escolha inválida.
    }

    return indices_ativos[escolha - 1]; // Retorna o índice real do veículo no array global.
}


int tentarRegistrarSaida(int idx_veiculo, float *valor_calc, int *minutos_calc, int *segundos_calc) {
    // Verifica se o índice é válido e se o veículo está ativo.
    if (idx_veiculo == -1 || !veiculos[idx_veiculo].ativo) {
        return 0; // Veículo inválido ou já não está ativo.
    }

    veiculos[idx_veiculo].saida = time(NULL); // Registra o tempo atual como saída.
    // Calcula a diferença de tempo em segundos entre entrada e saída.
    double tempo_segundos_double = difftime(veiculos[idx_veiculo].saida, veiculos[idx_veiculo].entrada);
    *minutos_calc = (int)(tempo_segundos_double / 60);       // Converte segundos para minutos.
    *segundos_calc = (int)tempo_segundos_double % 60;       // Segundos restantes.

    // Calcula o valor a ser pago com base na tarifa por minuto.
    float valor = (float)(tempo_segundos_double / 60.0 * TARIFA_POR_MINUTO);
    if (valor < TARIFA_MINIMA) {
        valor = TARIFA_MINIMA; // Aplica a tarifa mínima se o valor for menor.
    }

    veiculos[idx_veiculo].valor_pago = valor; // Armazena o valor pago no registro do veículo.
    veiculos[idx_veiculo].ativo = 0;          // Marca o veículo como inativo (saiu).
    *valor_calc = valor;                      // Passa o valor calculado de volta.
    salvarTodosVeiculos();                    // Salva o estado atualizado no arquivo.
    return 1;                                 // Sucesso.
}



// Exibe o menu principal do sistema.
void exibirMenu() {
    limparTela(); // Limpa a tela antes de exibir o menu.
    printf(YELLOW "========================================\n");
    printf("        SISTEMA DE ESTACIONAMENTO       \n");
    printf("========================================\n" RESET);
    printf(" " CYAN "[1]" RESET " Registrar entrada de veículo\n");
    printf(" " CYAN "[2]" RESET " Registrar saída de veículo\n");
    printf(" " CYAN "[3]" RESET " Listar veículos estacionados\n");
    printf(" " CYAN "[4]" RESET " Gerar relatório financeiro\n");
    printf(" " CYAN "[5]" RESET " Histórico de veículos\n");
    printf(" " CYAN "[0]" RESET " Sair\n");
    printf(YELLOW "========================================\n" RESET);
    printf(" Escolha uma opção: "); // Solicita a escolha do usuário.
}

// Exibe uma lista de todos os veículos que estão atualmente estacionados.
void exibirListaVeiculos(const Veiculo *lista_veiculos, int num_veiculos) {
    limparTela();
    printf(YELLOW "VEÍCULOS ATUALMENTE ESTACIONADOS:\n" RESET);
    int encontrados = 0; // Contador para verificar se há veículos estacionados.
    for (int i = 0; i < num_veiculos; i++) {
        if (lista_veiculos[i].ativo) { // Se o veículo está ativo (estacionado).
            char entrada_str[20];
            // Formata o timestamp de entrada para uma string legível.
            strftime(entrada_str, sizeof(entrada_str), "%d/%m/%Y %H:%M:%S", localtime(&lista_veiculos[i].entrada));
            // Imprime a placa e a hora de entrada.
            printf("Placa: " CYAN "%s" RESET " | Entrada: " CYAN "%s\n" RESET, lista_veiculos[i].placa, entrada_str);
            encontrados = 1;
        }
    }
    if (!encontrados) {
        printf(RED "Nenhum veículo estacionado no momento.\n" RESET); // Mensagem se nenhum veículo for encontrado.
    }
}

// Exibe um relatório financeiro, mostrando o valor arrecadado dos veículos que já saíram.
void exibirRelatorioFinanceiro(const Veiculo *lista_veiculos, int num_veiculos) {
    limparTela();
    float total_arrecadado = 0; // Acumulador do total arrecadado.
    printf(YELLOW "RELATÓRIO FINANCEIRO (VEÍCULOS QUE JÁ SAÍRAM):\n" RESET);
    int registros_encontrados = 0; // Contador para verificar se há registros de pagamento.
    for (int i = 0; i < num_veiculos; i++) {
        // Se o veículo não está ativo (saiu) e houve um valor pago (maior que 0, para excluir entradas com 0).
        if (!lista_veiculos[i].ativo && lista_veiculos[i].valor_pago > 0) {
            printf("Placa: " CYAN "%s" RESET " | Valor pago: " GREEN "R$ %.2f\n" RESET, lista_veiculos[i].placa, lista_veiculos[i].valor_pago);
            total_arrecadado += lista_veiculos[i].valor_pago;
            registros_encontrados = 1;
        }
    }
    if (!registros_encontrados) {
        printf(RED "Nenhum registro de pagamento encontrado.\n" RESET); // Mensagem se nenhum pagamento for encontrado.
    }
    printf(YELLOW "----------------------------------------\n");
    printf("TOTAL ARRECADADO: " GREEN "R$ %.2f\n" RESET, total_arrecadado); // Imprime o total.
}

// Exibe o histórico completo de todos os veículos, tanto os que saíram quanto os que estão ativos.
void exibirHistoricoCompleto(const Veiculo *lista_veiculos, int num_veiculos) {
    limparTela();
    printf(YELLOW "HISTÓRICO COMPLETO DE VEÍCULOS (ENTRADA E SAÍDA):\n" RESET);

    int encontrados = 0; // Contador para verificar se há registros no histórico.
    for (int i = 0; i < num_veiculos; i++) {
        char entrada_str[20];
        strftime(entrada_str, sizeof(entrada_str), "%d/%m/%Y %H:%M:%S", localtime(&lista_veiculos[i].entrada));

        printf("Placa: " CYAN "%s\n" RESET, lista_veiculos[i].placa);
        printf("  Entrada: %s\n", entrada_str);

        // Se o veículo já saiu e tem um registro de saída válido.
        if (!lista_veiculos[i].ativo && lista_veiculos[i].saida != 0) {
            char saida_str[20];
            strftime(saida_str, sizeof(saida_str), "%d/%m/%Y %H:%M:%S", localtime(&lista_veiculos[i].saida));
            printf("  Saída:   %s\n", saida_str);
            printf(GREEN "  Valor pago: R$ %.2f\n" RESET, lista_veiculos[i].valor_pago);
        } else {
            printf(CYAN "  Status: ESTACIONADO ATUALMENTE\n" RESET); // Se ainda está estacionado.
        }
        printf("------------------------------------\n");
        encontrados = 1;
    }
    if (!encontrados) {
        printf(RED "Nenhum veículo no histórico.\n" RESET); // Mensagem se o histórico estiver vazio.
    }
}


int main() {
#ifdef _WIN32
    // Configura o console do Windows para usar UTF-8, permitindo a exibição de caracteres especiais.
    system("chcp 65001 > nul");
#endif

    carregarDados(); // Carrega os dados dos veículos do arquivo ao iniciar o programa.

    int opcao; // Variável para armazenar a opção escolhida pelo usuário no menu.
    do {
        exibirMenu(); // Exibe o menu de opções para o usuário.
        opcao = lerInteiro(); // Lê a opção do usuário de forma segura.

        switch (opcao) {
            case 1: { // Opção: Registrar entrada de veículo
                limparTela();
                printf(YELLOW "REGISTRO DE ENTRADA\n" RESET);
                // Prompt atualizado para os novos padrões de placa.
                printf("Placa do veículo (Ex: ABC-1234 ou ABC1E23): ");
                char placa_entrada[8]; // Buffer para armazenar a placa digitada.
                if (lerPlaca(placa_entrada)) { // Tenta ler e validar a placa.
                    // Se a placa é válida, tenta registrar a entrada do veículo.
                    if (tentarRegistrarEntrada(placa_entrada)) {
                        printf(GREEN "Veículo '%s' registrado com sucesso.\n" RESET, placa_entrada);
                    } else {
                        // Mensagem de erro se o registro falhar.
                        printf(RED "Não foi possível registrar o veículo '%s'. Verifique se a capacidade máxima foi atingida ou se já está estacionado.\n" RESET, placa_entrada);
                    }
                } else {
                    // Mensagem de erro se a placa digitada for inválida.
                    printf(RED "Placa inválida. A placa deve seguir o padrão 'LLL-NNNN' ou 'LLLNLNN'.\n" RESET);
                }
                pause(); // Pausa para o usuário ler a mensagem antes de retornar ao menu.
                break;
            }
            case 2: { // Opção: Registrar saída de veículo
                limparTela();
                printf(YELLOW "REGISTRO DE SAÍDA\n" RESET);
                // Obtém o índice do veículo que o usuário deseja registrar a saída.
                // A função 'obterIndiceVeiculoAtivo' já interage para exibir a lista e ler a escolha.
                int idx = obterIndiceVeiculoAtivo(veiculos, total_veiculos);
                if (idx != -1) { // Se um veículo válido foi selecionado.
                    char entrada_str[20];
                    // Formata e exibe a hora de entrada do veículo selecionado.
                    strftime(entrada_str, sizeof(entrada_str), "%d/%m/%Y %H:%M:%S", localtime(&veiculos[idx].entrada));

                    printf("\nVocê selecionou o veículo:\n");
                    printf("  Placa: " CYAN "%s\n" RESET, veiculos[idx].placa);
                    printf("  Entrada: " CYAN "%s\n" RESET, entrada_str);

                    // Pede confirmação ao usuário para registrar a saída.
                    printf("\nDeseja registrar a saída deste veículo? (s/n): ");
                    char confirmacao_str[5];
                    lerLinha(confirmacao_str, sizeof(confirmacao_str));
                    char confirmacao = tolower(confirmacao_str[0]); // Converte para minúscula para comparação.

                    if (confirmacao == 's') { // Se o usuário confirmar a saída.
                        float valor_pago_saida;
                        int minutos_saida, segundos_saida;
                        // Tenta registrar a saída e obtém o valor, minutos e segundos calculados.
                        if (tentarRegistrarSaida(idx, &valor_pago_saida, &minutos_saida, &segundos_saida)) {
                            char saida_str[20];
                            // Formata e exibe a hora de saída.
                            strftime(saida_str, sizeof(saida_str), "%d/%m/%Y %H:%M:%S", localtime(&veiculos[idx].saida));
                            printf(GREEN "\nSaída registrada com sucesso!\n" RESET);
                            printf("  Saída: %s\n", saida_str);
                            printf("  Tempo estacionado: %d min %02ds\n", minutos_saida, segundos_saida);
                            printf("  " GREEN "Valor a pagar: R$ %.2f\n" RESET, valor_pago_saida);
                        } else {
                            printf(RED "Erro ao registrar saída do veículo.\n" RESET);
                        }
                    } else {
                        printf(RED "\nSaída cancelada.\n" RESET); // Mensagem se o usuário cancelar.
                    }
                }
                pause(); // Pausa para o usuário ler a mensagem.
                break;
            }
            case 3: // Opção: Listar veículos estacionados
                exibirListaVeiculos(veiculos, total_veiculos); // Chama a função para exibir a lista.
                pause(); // Pausa.
                break;
            case 4: // Opção: Gerar relatório financeiro
                exibirRelatorioFinanceiro(veiculos, total_veiculos); // Chama a função para exibir o relatório.
                pause(); // Pausa.
                break;
            case 5: // Opção: Histórico de veículos
                exibirHistoricoCompleto(veiculos, total_veiculos); // Chama a função para exibir o histórico.
                pause(); // Pausa.
                break;
            case 0: // Opção: Sair do programa
                printf(GREEN "Saindo do sistema. Obrigado!\n" RESET);
                break;
            default: // Opção inválida
                printf(RED "Opção inválida. Por favor, escolha um número do menu.\n" RESET);
                pause();
                break;
        }
    } while (opcao != 0); // Continua o loop do menu até o usuário escolher sair (opção 0).

    return 0; 
}
