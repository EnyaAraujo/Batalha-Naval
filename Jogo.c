#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define TAMANHO_TABULEIRO 10
#define NUM_NAVIOS 5

typedef struct {
    char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
    char nome_jogador[10];
    int acertos;
    int bot;
} TabuleiroJogador;

typedef struct {
    TabuleiroJogador *jogador;
    TabuleiroJogador *oponente;
    sem_t *meu_semaforo;
    sem_t *semaforo_oponente;
} ArgumentosThread;


void limpar_tela();
void inicializar_tabuleiro(TabuleiroJogador *jogador);
void exibir_tabuleiro_posicionamento(TabuleiroJogador *jogador);
void posicionar_navios_humano(TabuleiroJogador *jogador);
void posicionar_navios_aleatoriamente(TabuleiroJogador *jogador);
void exibir_tabuleiros_jogo(TabuleiroJogador *jogador, TabuleiroJogador *oponente);
int realizar_jogada_humano(TabuleiroJogador *jogador, TabuleiroJogador *oponente);
int realizar_jogada_bot(TabuleiroJogador *jogador, TabuleiroJogador *oponente);
void *thread_jogador(void *args);

int main() {
    pthread_t thread_p1, thread_p2;
    TabuleiroJogador jogador1, jogador2;
    sem_t sem_p1, sem_p2;
    int modo_de_jogo;

    srand(time(NULL));

   
    printf("Bem-vindo ao Batalha Naval!\n");
    printf("Escolha o modo de jogo:\n");
    printf("1. Single Player (vs. Computador)\n");
    printf("2. Multiplayer (2 Jogadores)\n");
    printf("Sua escolha: ");
    scanf("%d", &modo_de_jogo);

    // Inicializa jogadores
    sprintf(jogador1.nome_jogador, "Jogador 1");
    jogador1.bot = 0;
    inicializar_tabuleiro(&jogador1);

    if (modo_de_jogo == 1) { // Single Player
        sprintf(jogador2.nome_jogador, "Computador");
        jogador2.bot = 1;
        inicializar_tabuleiro(&jogador2);

        printf("\n--- %s, posicione seus navios! ---\n", jogador1.nome_jogador);
        posicionar_navios_humano(&jogador1);
        
        printf("\nO Computador está posicionando seus navios...\n");
        posicionar_navios_aleatoriamente(&jogador2);
        sleep(2);

    } else { // Multiplayer
        sprintf(jogador2.nome_jogador, "Jogador 2");
        jogador2.bot = 0;
        inicializar_tabuleiro(&jogador2);

        printf("\n--- %s, posicione seus navios! ---\n", jogador1.nome_jogador);
        posicionar_navios_humano(&jogador1);
        printf("\n--- %s, posicione seus navios! ---\n", jogador2.nome_jogador);
        posicionar_navios_humano(&jogador2);
    }
    
    printf("\nAmbos os jogadores estão prontos. A batalha vai começar!\n");
    sleep(3);

    
    sem_init(&sem_p1, 0, 1);
    sem_init(&sem_p2, 0, 0);

    ArgumentosThread args_p1 = {&jogador1, &jogador2, &sem_p1, &sem_p2};
    ArgumentosThread args_p2 = {&jogador2, &jogador1, &sem_p2, &sem_p1};

    pthread_create(&thread_p1, NULL, thread_jogador, (void *)&args_p1);
    pthread_create(&thread_p2, NULL, thread_jogador, (void *)&args_p2);

    pthread_join(thread_p1, NULL);
    pthread_join(thread_p2, NULL);

    sem_destroy(&sem_p1);
    sem_destroy(&sem_p2);

    return 0;
}


// controla o fluxo de jogo para cada thread
void *thread_jogador(void *args) {
    ArgumentosThread *arg = (ArgumentosThread *)args;
    int total_acertos_necessarios = 17; // 5 + 4 + 3 + 3 + 2 

    while (1) {
        sem_wait(arg->meu_semaforo);

        
        if (arg->oponente->acertos == total_acertos_necessarios) {
            sem_post(arg->semaforo_oponente);
            break;
        }

        int foi_acerto;
        do {
            limpar_tela();
            printf("\n--------------------------------------------------\n");
            printf("Vez de %s\n", arg->jogador->nome_jogador);
            
           
            if(arg->jogador->bot) {
                exibir_tabuleiros_jogo(arg->oponente, arg->jogador);
            } else {
                exibir_tabuleiros_jogo(arg->jogador, arg->oponente);
            }

            
            if (arg->jogador->bot) {
                foi_acerto = realizar_jogada_bot(arg->jogador, arg->oponente);
            } else {
                foi_acerto = realizar_jogada_humano(arg->jogador, arg->oponente);
            }

            
            if (foi_acerto) {
                arg->jogador->acertos++;

                
                if (arg->jogador->acertos == total_acertos_necessarios) {
                    limpar_tela();
                    printf("\n\nFIM DE JOGO!\n");
                    printf("**********************************************\n");
                    printf("* %s AFUNDOU TODOS OS NAVIOS E VENCEU!    *\n", arg->jogador->nome_jogador);
                    printf("**********************************************\n");
                    
                    
                    if(arg->jogador->bot) exibir_tabuleiros_jogo(arg->oponente, arg->jogador);
                    else exibir_tabuleiros_jogo(arg->jogador, arg->oponente);

                    foi_acerto = 0;
                } else {
                    printf("\nACERTOU! %s joga novamente.\n", arg->jogador->nome_jogador);
                    sleep(3); 
                }
            } else {
    
                sleep(2);
            }

        } while (foi_acerto);

        
        if (arg->jogador->acertos == total_acertos_necessarios) {
            sem_post(arg->semaforo_oponente);
            break;
        }

        
        sem_post(arg->semaforo_oponente);
    }

    pthread_exit(NULL);
}

void limpar_tela() {
    system("clear || cls");
}

void inicializar_tabuleiro(TabuleiroJogador *jogador) {
    jogador->acertos = 0;
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            jogador->tabuleiro[i][j] = '~';
        }
    }
}

void exibir_tabuleiro_posicionamento(TabuleiroJogador *jogador) {
    printf("\n  ");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) printf("%d ", i);
    printf("\n");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        printf("%d ", i);
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            printf("%c ", jogador->tabuleiro[i][j]);
        }
        printf("\n");
    }
}

void posicionar_navios_humano(TabuleiroJogador *jogador) {
    int tamanhos_navios[NUM_NAVIOS] = {5, 4, 3, 3, 2};
    for (int i = 0; i < NUM_NAVIOS; i++) {
        int tamanho = tamanhos_navios[i];
        int linha, coluna;
        char orientacao_char;
        int valido;

        limpar_tela();
        printf("--- %s, posicione seus navios ---\n", jogador->nome_jogador);
        exibir_tabuleiro_posicionamento(jogador);

        do {
            valido = 1;
            printf("\nPosicione o navio de tamanho %d.\n", tamanho);
            printf("Digite a linha, a coluna e a orientação (H/V): ");
            scanf("%d %d %c", &linha, &coluna, &orientacao_char);
            orientacao_char = toupper(orientacao_char);

            if (linha < 0 || linha >= TAMANHO_TABULEIRO || coluna < 0 || coluna >= TAMANHO_TABULEIRO || (orientacao_char != 'H' && orientacao_char != 'V')) {
                printf("Entrada inválida. Tente novamente.\n");
                valido = 0; continue;
            }

            if (orientacao_char == 'H') {
                if (coluna + tamanho > TAMANHO_TABULEIRO) {
                    printf("Posição inválida: o navio sai do tabuleiro.\n");
                    valido = 0; continue;
                }
                for (int j = 0; j < tamanho; j++) {
                    if (jogador->tabuleiro[linha][coluna + j] == 'N') {
                        printf("Posição inválida: sobreposição de navios.\n");
                        valido = 0; break;
                    }
                }
            } else { // Vertical
                if (linha + tamanho > TAMANHO_TABULEIRO) {
                    printf("Posição inválida: o navio sai do tabuleiro.\n");
                    valido = 0; continue;
                }
                for (int j = 0; j < tamanho; j++) {
                    if (jogador->tabuleiro[linha + j][coluna] == 'N') {
                        printf("Posição inválida: sobreposição de navios.\n");
                        valido = 0; break;
                    }
                }
            }
        } while (!valido);

        if (orientacao_char == 'H') {
            for (int j = 0; j < tamanho; j++) jogador->tabuleiro[linha][coluna + j] = 'N';
        } else {
            for (int j = 0; j < tamanho; j++) jogador->tabuleiro[linha + j][coluna] = 'N';
        }
    }
    limpar_tela();
    printf("--- %s, todos os seus navios foram posicionados! ---\n", jogador->nome_jogador);
    exibir_tabuleiro_posicionamento(jogador);
    sleep(3);
    limpar_tela();
}

void posicionar_navios_aleatoriamente(TabuleiroJogador *jogador) {
    int tamanhos_navios[NUM_NAVIOS] = {5, 4, 3, 3, 2};
    for (int i = 0; i < NUM_NAVIOS; i++) {
        int tamanho = tamanhos_navios[i];
        int linha, coluna, orientacao;
        int valido;
        do {
            valido = 1;
            orientacao = rand() % 2;
            if (orientacao == 0) {
                linha = rand() % TAMANHO_TABULEIRO;
                coluna = rand() % (TAMANHO_TABULEIRO - tamanho + 1);
                for (int j = 0; j < tamanho; j++) {
                    if (jogador->tabuleiro[linha][coluna + j] == 'N') {
                        valido = 0; break;
                    }
                }
            } else {
                linha = rand() % (TAMANHO_TABULEIRO - tamanho + 1);
                coluna = rand() % TAMANHO_TABULEIRO;
                for (int j = 0; j < tamanho; j++) {
                    if (jogador->tabuleiro[linha + j][coluna] == 'N') {
                        valido = 0; break;
                    }
                }
            }
        } while (!valido);

        if (orientacao == 0) {
            for (int j = 0; j < tamanho; j++) jogador->tabuleiro[linha][coluna + j] = 'N';
        } else {
            for (int j = 0; j < tamanho; j++) jogador->tabuleiro[linha + j][coluna] = 'N';
        }
    }
}

void exibir_tabuleiros_jogo(TabuleiroJogador *jogador, TabuleiroJogador *oponente) {
    printf("\nSEU TABULEIRO (%s)\t\t\tTABULEIRO DE ATAQUE (vs. %s)\n", jogador->nome_jogador, oponente->nome_jogador);
    printf("  ");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) printf("%d ", i);
    printf("\t\t  ");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) printf("%d ", i);
    printf("\n");

    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        printf("%d ", i);
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            printf("%c ", jogador->tabuleiro[i][j]);
        }
        
        printf("\t\t%d ", i);
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            if (oponente->tabuleiro[i][j] == 'X' || oponente->tabuleiro[i][j] == 'o') {
                printf("%c ", oponente->tabuleiro[i][j]);
            } else {
                printf("~ ");
            }
        }
        printf("\n");
    }
}

int realizar_jogada_humano(TabuleiroJogador *jogador, TabuleiroJogador *oponente) {
    int linha, coluna, valido;
    do {
        valido = 1;
        printf("\n%s, digite a linha e a coluna para atirar (0-9 0-9): ", jogador->nome_jogador);
        scanf("%d %d", &linha, &coluna);
        
        if (linha < 0 || linha >= TAMANHO_TABULEIRO || coluna < 0 || coluna >= TAMANHO_TABULEIRO) {
            printf("Coordenadas fora do tabuleiro. Tente novamente.\n");
            valido = 0;
        } else if (oponente->tabuleiro[linha][coluna] == 'X' || oponente->tabuleiro[linha][coluna] == 'o') {
            printf("Você já atirou nesta posição. Tente novamente.\n");
            valido = 0;
        }
    } while (!valido);

    if (oponente->tabuleiro[linha][coluna] == 'N') {
        printf("FOGO! Você acertou um navio!\n");
        oponente->tabuleiro[linha][coluna] = 'X';
        return 1;
    } else {
        printf("ÁGUA! Você errou.\n");
        oponente->tabuleiro[linha][coluna] = 'o';
        return 0;
    }
}

int realizar_jogada_bot(TabuleiroJogador *jogador, TabuleiroJogador *oponente) {
    int linha, coluna;
    do {
        linha = rand() % TAMANHO_TABULEIRO;
        coluna = rand() % TAMANHO_TABULEIRO;
    } while (oponente->tabuleiro[linha][coluna] == 'X' || oponente->tabuleiro[linha][coluna] == 'o');

    printf("\n%s atira na posição %d, %d...\n", jogador->nome_jogador, linha, coluna);
    sleep(1);

    if (oponente->tabuleiro[linha][coluna] == 'N') {
        printf("FOGO! %s acertou um navio!\n", jogador->nome_jogador);
        oponente->tabuleiro[linha][coluna] = 'X';
        return 1;
    } else {
        printf("ÁGUA! %s errou o tiro.\n", jogador->nome_jogador);
        oponente->tabuleiro[linha][coluna] = 'o';
        return 0;
    }
}
    
