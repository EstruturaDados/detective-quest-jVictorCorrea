#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define TAMANHO_ALFABETO 26 
#define TAMANHO_TABELA 50 

typedef struct {
    char nome [50];
    char suspeito [50];
    int ocupado;
} Suspeitos;

struct No {
    char valor [50];
    struct No* esquerda;
    struct No* direita;
    struct No* pai;         //Precisei fazer essa alteração na estrutura do No, para conseguir gerar a opção de retornar para o pai.
    char item [100];        //Item do comodo
    bool act;               //Sinaliza se em determinado no, ha alguma pista
};

struct NoTrie {
    struct NoTrie* filhos [TAMANHO_ALFABETO];
    char comodo [50];
    bool end;
};

Suspeitos tabela_hash [TAMANHO_TABELA];

void normalizar (const char* entrada, char* saida) {
    int j = 0;
    for (int i = 0; entrada [i] != '\0'; i++) {
        char c = entrada[i];
        if (c >= 'A' && c <= 'Z'){
            c+=32;
        }

        if ( c >= 'a' && c <= 'z') {
            saida[j++] = c;
        }
    }
    saida[j] =  '\0';
}

void limparBufferEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
};

int funcao_hash (const char* chave) {
    int soma = 0;
    for (int i = 0; chave[i] != '\0'; i++) {
        soma += chave[i];
    }
    return soma % TAMANHO_TABELA;
}

void inserir_linear (const char* item, const char *lista_suspeitos[], int n_elem_suspeitos) {
    char buffer[100];
    normalizar(item, buffer);
    int indice = funcao_hash(buffer);

    for (int i = 0; i < TAMANHO_TABELA; i++) {
        int idx_suspeito = rand() % n_elem_suspeitos;
        int pos = (indice+i) % TAMANHO_TABELA;

        if (tabela_hash[pos].ocupado == 0 || tabela_hash[pos].ocupado == -1) {
            strcpy(tabela_hash[pos].nome, buffer);
            strcpy(tabela_hash[pos].suspeito, lista_suspeitos[idx_suspeito]);
            tabela_hash[pos].ocupado = 1;
            return;
        }
    }
}

int buscar_linear (const char* item) {
    int indice = funcao_hash(item);
    for (int  i = 0; i < TAMANHO_TABELA; i++) {
        int pos =(indice + i) %TAMANHO_TABELA;

        if (tabela_hash[pos].ocupado == 0) {
            return -1;
        }

        if (tabela_hash[pos].ocupado == 1 && strcmp(tabela_hash[pos].nome, item) == 0) {
            return pos;
        }
    }
    return -1;
}

void remover_linear (const char* item) {
    int pos = buscar_linear(item);
    if (pos == -1) {
        printf("%s nao encontrado\n", item);
        return;
    }

    tabela_hash[pos].ocupado = -1;
    printf("%s removido com sucesso.\n", item);

}

struct No* criarNo (const char* valor, const char* nome) {
    struct No* novo = (struct No*) malloc(sizeof(struct No));
    if (novo == NULL) {
        printf("Erro na alocacao de memoria!\n");
        exit(1);
    }

    strcpy(novo->valor, valor);
    strcpy(novo->item, nome);
    novo->esquerda = NULL;
    novo->direita = NULL;
    novo->pai = NULL;

    if (strcmp(nome,"0") != 0) {   //Verifica se o item e um item valido
        novo->act = true;       //Apenas marca como true, caso o item inserido seja válido.
    } else {
        novo->act = false;
    }

    return novo;
}

struct NoTrie* criarNoTrie (){
    struct NoTrie* novoNo = (struct NoTrie*) malloc (sizeof(struct NoTrie));
    novoNo->end = false;
    for (int i = 0; i < TAMANHO_ALFABETO; i++) {
        novoNo->filhos[i] = NULL;
    }

    return novoNo;
}

void listarItens (struct NoTrie* raiz, char* buffer, int nivel) {
    if (raiz == NULL){
        return;
    }
    if (raiz->end) {
        buffer[nivel] = '\0';
        int pos = buscar_linear(buffer);

        char suspeito_hash [50];

        if (pos == -1) {
            strcpy(suspeito_hash,"Nao encontrado"); 
        } else {
            strcpy(suspeito_hash,tabela_hash[pos].suspeito);
        }
        printf("%-24s | %-24s | %-24s\n", buffer, raiz->comodo, suspeito_hash);
    }

    for (int i = 0; i < TAMANHO_ALFABETO; i++) {
        if (raiz->filhos[i] != NULL) {
            buffer[nivel] = 'a' + i;
            listarItens (raiz->filhos[i], buffer, nivel + 1);
        }
    }
}

struct No* inserir (struct No* raiz, const char* valor, const char* nome) {
    if (raiz == NULL) {
        return criarNo(valor, nome);
    }

    if (strcmp(valor, raiz->valor) < 0) {
        raiz->esquerda = inserir(raiz->esquerda, valor, nome);
        raiz->esquerda->pai = raiz;
    } else {
        raiz->direita = inserir(raiz->direita, valor, nome);
        raiz->direita->pai = raiz;
    }

    return raiz;
}

void inserirNoTrie (struct NoTrie* raiz, const char* word, const char* comodo) {
    struct NoTrie* atual = raiz;

    for (int i = 0; word[i] != '\0'; i++) {
        int indice = word[i] - 'a';

        if (atual->filhos[indice] == NULL) {
            atual->filhos[indice] = criarNoTrie();
        }
        atual = atual->filhos[indice];
    }
    atual->end = true;
    strcpy(atual->comodo, comodo);
}

struct No* avanca_direita (struct No* raiz) {       // Procura o elemento a direita mais fundo possivel
    if (raiz == NULL) {
        return NULL;
    } else if (raiz->direita != NULL) {
        return avanca_direita(raiz->direita);
    }
    else if (raiz->esquerda != NULL) {          //Possibilita explorar a esquerda, caso a direta nao exista
        return avanca_direita(raiz->esquerda);
    } else {
        return raiz;
    }
}

struct No* avanca_esquerda (struct No* raiz) {      // Avanca para o elemente mais a esquerda possivel
    if (raiz == NULL) {
        return NULL;
    } else if (raiz->esquerda != NULL) {
        return avanca_esquerda(raiz->esquerda);
    }
    else if (raiz->direita != NULL) {
        return avanca_esquerda(raiz->direita);
    } else {
        return raiz;
    }
}

struct No* procure_vazio (struct No* raiz) {        //Verifica se determinado comodo ou seu irmão está vazio, caso não, verifica o pai;
    if (raiz == NULL) {
        return NULL;
    }

    if (!(raiz->act)) {
        return raiz;
    }
    
    if (raiz->pai != NULL) {
        if ((raiz->pai->direita) != NULL && !(raiz->pai->direita->act)) {
            return raiz->pai->direita;
        } else if ((raiz->pai->esquerda) != NULL && !(raiz->pai->esquerda->act)) {
            return raiz->pai->esquerda;
        } else {
            return procure_vazio(raiz->pai);
        }
    } else {
        return NULL;
    }
}

bool insere_esquerda_direita (struct No* raiz, char* n) {      // Da preferencia em adicionar o item ao comodo mais a esquerda
    if (procure_vazio(avanca_esquerda(raiz)) == NULL) {       // caso nao encontre, vai para o comodo mais a direita
        if (procure_vazio(avanca_direita(raiz)) == NULL) {
            return false;
        } else {
            strcpy(procure_vazio(avanca_direita(raiz))->item,n);
            procure_vazio(avanca_direita(raiz))->act = true;
            strcpy(n,"0");
            return true; 
        }
    } else {
        strcpy(procure_vazio(avanca_esquerda(raiz))->item,n);
        procure_vazio(avanca_esquerda(raiz))->act = true;
        strcpy(n,"0");
        return true;
    }
}

bool insere_direita_esquerda (struct No* raiz, char* n) {      // Da preferencia em adicionar o item ao comodo mais a direita
    if (procure_vazio(avanca_direita(raiz)) == NULL) {       // caso nao encontre, vai para o comodo mais a esquerda
        if (procure_vazio(avanca_esquerda(raiz)) == NULL) {
            return false;
        } else {    
            strcpy(procure_vazio(avanca_esquerda(raiz))->item,n);
            procure_vazio(avanca_esquerda(raiz))->act = true;
            strcpy(n,"0");
            return true;
        }
    } else {
        strcpy(procure_vazio(avanca_direita(raiz))->item,n);
        procure_vazio(avanca_direita(raiz))->act = true;
        strcpy(n,"0");
        return true; 
    }
}

bool verifica_distri (struct No* raiz, char *lista_itens[], int n_elem_itens, const char *lista_suspeitos[], int n_elem_suspeitos) {      //Verifica se todos os itens foram distribuidos
    
    for (int i = 0; i < n_elem_itens; i++) {
        if (strcmp(lista_itens[i], "0") != 0) {
            inserir_linear(lista_itens[i], lista_suspeitos, n_elem_suspeitos);
            int c = rand () % 2;        //Decide entre incluir o item no comodo mais a esquerda possivel ou mais a direita possivel
            bool k;
            if (c) {
                k = insere_esquerda_direita(raiz, lista_itens[i]);
            } else {
                k = insere_direita_esquerda(raiz, lista_itens[i]);
            }

            if (!k) {
                return false;
            }
        }
    }

    return true;
}

char* seleciona_item (char *lista[], char *item, int tamanho) {
    int start = rand() % tamanho;
    for (int i = 0; i < tamanho; i++) {
        int idx_item = (start+i) % tamanho;

        if(strcmp(lista[idx_item],"0") != 0) {
            strcpy(item, lista [idx_item]);
            strcpy(lista[idx_item],"0");
            return item;
        }
    }
    return item;
}

bool verifica_item_lista (char *lista_itens [], int num_elem_itens) {
    for (int i = 0; i < num_elem_itens; i++) {
        if (strcmp(lista_itens[i], "0") != 0) {
            return true;
        }
    }
    return false;
}

struct No* inserir_dinamico (struct No* raiz, const char *lista_comodos [], int num_elem_comodos, char *lista_itens [], int num_elem_itens, int num_elemen_suspeitos, const char *lista_suspeitos[]) {   //Constrói dinamicamente a casa e distribui os itens aleatoriamente entre eles. 
    if (lista_comodos == NULL || lista_itens == NULL) {
        return (raiz = NULL);
    }    
    for (int i = 0; i < num_elem_comodos; i++) {
        char item [100] = "0";
        int random = rand() % 5;   

        if (random == 4) {      //Adiciona 20% de chance de adicionar um item no comodo criado
            strcpy(item,seleciona_item (lista_itens, item, num_elem_itens));        //Pega um item aleatório
            inserir_linear(item, lista_suspeitos, num_elemen_suspeitos);
        }

        raiz = inserir(raiz, lista_comodos[i], item);
    }

    bool verificacao = verifica_distri(raiz, lista_itens, num_elem_itens, lista_suspeitos, num_elemen_suspeitos);

    if (!verificacao) {
        return (raiz = NULL);
    }   

    return raiz;
}

void liberar (struct No* raiz) {
    if (raiz != NULL) {
        liberar(raiz->esquerda);
        liberar(raiz->direita);
        free(raiz);
    }
}

void list_copy (const char *lista[], char *copia[], int n) {        //Copia a lista de itens para uma lista a ser manipulada
    if (lista == NULL) {
        copia = NULL;
    }
    for (int i = 0; i < n; i++) {
        copia[i] = strdup(lista[i]);
         if (copia[i] == NULL) {
            printf("Erro na alocacao de memoria.\n");
            exit(1);
        }
    }
}

void verifica_item_find (struct No* raiz, struct NoTrie* raiztrie, bool ver, char *item, char *normalizado, char *comodo, int *cont_item) {
    if (ver) {
        system("clear");
        printf("Voce encontrou %s!\n", item);
        normalizar(item, normalizado);
        inserirNoTrie(raiztrie, normalizado, comodo);
        strcpy(item,"0");
        raiz->act = false;
        (*cont_item)++;
        printf("Pressione ENTER para continuar...");
        getchar();
        system ("clear");
    }
}

void selecione_culpado (const char *lista_itens[], const char *lista_suspeitos[], int n_elem_itens, int n_elem_suspeitos, char *culpado) {
    int idx_culpado = 0;
    int qtd_provas = 0;

    for (int i = 0; i < n_elem_suspeitos; i++) {
        int provas = 0;
        char buffer[100];
        
        for (int j = 0; j < n_elem_itens; j++) {
           normalizar(lista_itens[j], buffer);

            if (buscar_linear(buffer) != -1 && strcmp(tabela_hash[buscar_linear(buffer)].suspeito,lista_suspeitos[i]) == 0) {
                provas++;
            }
        }

        if (provas > qtd_provas) {
            idx_culpado = i;
            qtd_provas = provas;
        }
    }
    strcpy(culpado,lista_suspeitos[idx_culpado]);
}

void acusacao (const char *lista_suspeitos[], int n_elem_suspeitos, char *culpado, int *opcao) {
    bool k = true;
    while (k) {
        system("clear");
        printf("Segue abaixo a lista de suspeitos:\n");
    
        for (int i = 0; i < n_elem_suspeitos; i++) {
            printf("%d. %s\n",i+1, lista_suspeitos[i]);
        }

        printf("\n0. Sair\n");

        printf("Selecione uma das opcoes acima:");
        int idx;
        scanf("%d", &idx);
        limparBufferEntrada();

        if (idx == 0) {
        k = false;
        } else if (idx > 0 && idx <= n_elem_suspeitos) {
            int idx_acusacao = idx-1;
            bool w = true;

            while (w) {
                char confirma[50];
                system("clear");
                printf("Certeza que deseja acusar %s pelo assinado do Senhor Jose (s/n)? ", lista_suspeitos[idx_acusacao]);
                scanf("%s", confirma);
                limparBufferEntrada();

                if (strcmp(confirma,"s") == 0 || strcmp(confirma,"S") == 0 || strcmp(confirma,"y") == 0 || strcmp(confirma,"Y") == 0) {
                    if (strcmp(culpado, lista_suspeitos[idx_acusacao]) == 0) {
                        system("clear");
                        printf("PARABENS VOCE DESCOBRIU O CULPADO\n");
                        printf("%s foi para a prisao.", lista_suspeitos[idx_acusacao]);
                        *opcao = 0;
                        printf("\nPressione ENTER para continuar...");
                        getchar();
                    } else {
                        system("clear");
                        printf("VOCE PRENDEU UM INOCENTE!!!\n");
                        printf("%s passara o restante dos seus dias na prisao, enquanto o verdadeiro assassino continua livre...", lista_suspeitos[idx_acusacao]);
                        *opcao = 0;
                        printf("\nPressione ENTER para continuar...");
                        getchar();
                    }
                    return;
                } else if (strcmp(confirma,"n") == 0 || strcmp(confirma,"N") == 0) {
                    w = false;
                } else {
                    printf("\nOpcao invalida.");
                    printf("\nTente novamente.");
                    printf("\nPressione ENTER para continuar...");
                    getchar();
                }
            }
        } else {
            printf("\nOpcao invalida.");
            printf("\nTente novamente.");
            printf("\nPressione ENTER para continuar...");
            getchar();
        }

    }

}

void msg_inicial () {
    printf("O Senhor Jose acaba de ser assassinado...");
    getchar();
    printf("E voce recebeu a designacao para encontrar o culpado...");
    getchar();
    printf("Vasculhe a casa para encontrar provas e descobrir que foi o culpado...");
    getchar();
    printf("Boa sorte!");
    getchar();
    printf("...ou nao");
    getchar();
}

void inicializar_tabela_hash () {
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela_hash[i].ocupado = 0;
    }
}

int main () {

    srand(time(NULL));
    inicializar_tabela_hash ();
    int opcao;
    int nivel = 0;
    char item_normalized [100];
    char buffer [100];
    int cont_item = 0;          //Conta quantos itens ja foram encontrados
    char culpado[100];
    
    const char *lista_comodos [] = {
        "Hall de Entrada",
        "Sala",
        "Biblioteca",
        "Quarto",
        "Cozinha",
        "Banheiro",
        "Closet",
        "Quarto de hospedes",
        "Banheiro dos hospedes",
        "Sala de jantar"
    };

    const char *lista_suspeitos [] = {
        "Carolinda",
        "Sniper",
        "Professor",
        "Agregada",
        "Dona",
        "Delegada",
        "Engenheiro"
    };


    const char *lista_itens_fixa [] = {
        "adaga ensanguentada",
        "rastro de sangue",
        "sapato sujo",
        "dente",
        "copo sujo com batom",
        "celular quebrado",
        "lasca de espelho quebrado",
        "digital"
    };

    int n_elem_comodos = sizeof(lista_comodos) / sizeof(lista_comodos[0]);
    int n_elem_itens = sizeof(lista_itens_fixa) / sizeof(lista_itens_fixa[0]);
    int n_elem_suspeitos = sizeof(lista_suspeitos) / sizeof(lista_suspeitos[0]);
    
    char **lista_itens_uso = (char **) malloc(n_elem_itens * sizeof(char *));        
                                                        
    list_copy(lista_itens_fixa, lista_itens_uso, n_elem_itens);     //Faço uma cópia da lista de itens, para evitar perda da memória dos itens, após as manipulacoes
                                                                    //E alterações que a funcao inserir_dinamico fará

    struct No* raiz_jogo = inserir_dinamico(NULL, lista_comodos, n_elem_comodos, lista_itens_uso, n_elem_itens, n_elem_suspeitos, lista_suspeitos);
    struct No* movel = raiz_jogo;

    struct NoTrie* itens_find = criarNoTrie();
    selecione_culpado (lista_itens_fixa, lista_suspeitos, n_elem_itens, n_elem_suspeitos, culpado);

    for (int i = 0; i < n_elem_itens; i++) {
        free(lista_itens_uso[i]);
    }
    
    free(lista_itens_uso);

    

    msg_inicial();
    do {

        if (raiz_jogo == NULL) {
            printf("Erro critico! A mansao nao pode ser gerada!\n");
            return 1;
        }

        
        verifica_item_find (movel, itens_find, movel->act, movel->item, item_normalized, movel->valor, &cont_item);

        printf("\n==================================================");
        printf("\n     DETECTIVE QUEST");
        printf("\n==================================================\n");
        printf("Onde atualmente voce esta: %s\n\n", movel->valor);


        if (movel->esquerda != NULL) {
            printf("1. Ir para: %s\n", movel->esquerda->valor);
        }

        if (movel->direita != NULL) {
            printf("2. Ir para: %s\n", movel->direita->valor);
        }

        if (movel->pai != NULL) {
            printf("3. Voltar para: %s\n", movel->pai->valor);
        }

        printf("\n");
        if (itens_find != NULL) {
            printf("8. Listar itens encontrados\n");
        }

        if (itens_find != NULL) {
            printf("9. ACUSAR\n");
        }

        printf("0. Sair\n");
        printf("Escolha uma opcao: ");
        scanf ("%d", &opcao);
        getchar();

        switch (opcao) {

        case 1:
            if (movel->esquerda != NULL) {
                movel = movel->esquerda;
                system("clear");
                break;
            } else {
                system("clear");
                break;
            }
            
        case 2:
            if (movel->direita != NULL) {
                movel = movel->direita;
                system("clear");
                break;
            } else {
                system("clear");
                break;
            }

        case 3:
            if (movel->pai != NULL) {
                movel = movel->pai;
                system("clear");
                break;
            } else {
                system("clear");
                break;
            }

        case 8:
            if (itens_find != NULL) {
                system("clear");
                printf("\n---------------------------------------------------------------------------------------------------");
                printf("\n                                   --- ITENS NA ENCONTRADOS (%d/%d) ---", cont_item, n_elem_itens);
                printf("\n---------------------------------------------------------------------------------------------------\n");
                printf("%-24s | %-24s | %-24s\n", "ITEM", "ONDE FOI ENCONTRADO", "SUSPEITO");
                listarItens (itens_find, buffer, 0);
                printf("\nPressione ENTER para continuar...");
                getchar();
                system("clear");
                break;
            } else {
                break;
            }

        case 9:
            acusacao (lista_suspeitos, n_elem_suspeitos, culpado, &opcao);
            break;

        case 0:
            printf("\nSaindo...");
            break;
        
        default:
            break;
        }


    } while (opcao != 0);

    liberar(raiz_jogo);

    return 0;
}


