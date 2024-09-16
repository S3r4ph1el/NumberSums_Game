//----------------------------------------------------------------//
// Universidade de Brasília - Projeto da Disciplina de APC 2024.1 //
//                 Autor: Enzo Araújo Teles                       //
//                   Matrícula: 222001458                         //
//----------------------------------------------------------------//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR "cls"
#define SLEEP(seconds) Sleep(1000 * (seconds))
#define pause() system("pause")
#else
#include <unistd.h>
#define SLEEP(seconds) sleep(seconds)
#define CLEAR "clear"
#define pause() system("pause")
#endif

char name[50];      // Nome do jogador dentro do programa
char option;        // Opção do menu
int difficulty = 1; // 1 - Beginner, 2 - Intermediate, 3 - Advanced
int phase = 1;      // Sempre começa na fase 1
long pos;           // Offset para pegar a fase correta
int rows, cols;     // 4 - Beginner, 6 - Intermediate, 7 - Advanced

typedef struct // Struct para armazenar os jogadores
{
    char name[50];
    int points;
} Data;

Data players[1000];

void pressEnterToContinue()
{
    printf("\nPress \033[1m<<Enter>>\033[0m to continue...");
    while (getchar() != '\n')
        ;
}

void printMatriz(int matriz[rows][cols], int gabarito[2][cols], int lifes)
{
    printf("\033[40m                    \033[1;31m##### GAME #####                    \033[0m\n\n");

    printf("     ");

    for (int i = 0; i < rows; i++)
    {
        if (gabarito[0][i] == -1)
        {
            printf("   ");
        }
        else
        {
            printf("%3d", gabarito[0][i]);
        }
    }

    printf("                           Name: \033[33m%s\033[0m", name);
    printf("\n    -");

    for (int i = 0; i < cols; i++)
    {
        printf("---");
    }

    printf("                           Lifes: \033[31m%d\033[0m", lifes);

    printf("\n");

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (j == 0)
            {
                if (gabarito[1][i] == -1)
                {
                    printf("    |");
                }
                else
                {
                    printf("%3d |", gabarito[1][i]);
                }
            }
            if (matriz[i][j] == -1)
            {
                printf("   ");
            }
            else
            {
                printf("%3d", matriz[i][j]);
            }
        }
        printf("\n");
    }
}

void invalidOption()
{
    system(CLEAR);

    printf("\033[40m          \033[1;31m##### INVALID OPTION #####          \033[0m\n\n");
    printf("\033[1;31mPlease, choose a valid option.\033[0m\n");
    SLEEP(2);
}

void addingPoints(int points)
{
    FILE *ranking;

    ranking = fopen("../ranking.bin", "r+b");

    if (ranking == NULL)
    {
        printf("\033[31m>>>> Error opening file ranking.txt <<<<\033[0m\n");
        SLEEP(3);
        return;
    }

    for (int i = 0; i < 1000; i++) // Adicionando os pontos ao jogador
    {
        if (strcmp(players[i].name, name) == 0)
        {
            players[i].points += points;
            fseek(ranking, i * sizeof(Data), SEEK_SET);
            fwrite(&players[i], sizeof(Data), 1, ranking);
            printf("Congratulations, %s! You now have \033[32m%d\033[0m points!\n", players[i].name, players[i].points);
            break;
        }
    }

    fclose(ranking);
}

void loadingOffsets(int level, int offsets) // Carregar os offsets de cada fase
{
    FILE *file;

    level--;
    offsets--;

    file = fopen("../offsets.txt", "r");

    if (file == NULL)
    {
        printf("\033[31m>>>> Error opening file <<<<\033[0m\n");
        SLEEP(3);
        return;
    }

    int row, col;

    fscanf(file, "%d %d", &row, &col); // Pegando a quantidade de fases de cada dificuldade

    long offset[row][col];

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            fscanf(file, "%ld", &offset[i][j]); // Pegando os offsets
        }
    }

    pos = offset[level][offsets];

    fclose(file);
}

int play(int difficulty)
{
    system(CLEAR);

    FILE *file;
    char filename[10000];

    switch (difficulty)
    {
    case 1:
        rows = 4;
        cols = 4;
        strcpy(filename, "../beginner.txt");
        break;
    case 2:
        rows = 6;
        cols = 6;
        strcpy(filename, "../intermediate.txt");
        break;
    case 3:
        rows = 7;
        cols = 7;
        strcpy(filename, "../advanced.txt");
        break;
    }

    file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("\033[31m>>>> Error opening file <<<<\033[0m\n");
        SLEEP(3);
        return 1;
    }

    loadingOffsets(difficulty, phase);
    fseek(file, pos, SEEK_SET);

    int matriz[rows][cols];
    int gabarito[2][cols]; // 0 - Soma das linhas, 1 - Soma das colunas
    int respostas[rows][cols];
    int lifes = 5;

    memset(matriz, 0, sizeof(matriz));
    memset(gabarito, 0, sizeof(gabarito));
    memset(respostas, 0, sizeof(respostas));

    for (int i = 0; i < rows; i++) // Preenchendo a matriz com os valores do arquivo
    {
        for (int j = 0; j < cols; j++)
        {
            fscanf(file, "%d", &matriz[i][j]);
        }
    }

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fscanf(file, "%d", &gabarito[i][j]);
        }
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fscanf(file, "%d", &respostas[i][j]);
        }
    }

    fclose(file); // Zona de preenchimento vai até aqui

    int row, col; // Variáveis para receber a linha e coluna do usuário
    int win = 0;  // Variável para verificar se o usuário ganhou
    char op;

    while (1)
    {

        system(CLEAR);

        printMatriz(matriz, gabarito, lifes);

        printf("\n\033[1mChoose a number to erase (row col): \033[0m");
        scanf("%d %d", &row, &col);
        getchar();

        row--;
        col--;

        if (row < 0 || row >= rows || col < 0 || col >= cols)
        {
            printf("\033[31m>>>> Invalid position! <<<<\033[0m\n");
            SLEEP(2);
            continue;
        }

        if (matriz[row][col] == -1)
        {
            printf("\033[31m>>>> This number has already been erased! <<<<\033[0m\n");
            SLEEP(2);
            continue;
        }

        printf("\n\033[1mAre you sure you want to erase the number \033[31m%d\033[0m? \033[1m(y/n): \033[0m", matriz[row][col]);
        scanf("%c", &op);
        getchar();

        if (tolower(op) == 'y')
        {
            if (respostas[row][col] == 0)
            {
                matriz[row][col] = -1;
                printf("\033\n[32m>>>> Number erased! <<<<\033[0m\n");
                pressEnterToContinue();
                for (int i = 0; i < cols; i++) // Verificando se a linha ou coluna foi completada
                {
                    if (gabarito[0][i] != -1)
                    {
                        int sum = 0;
                        for (int j = 0; j < rows; j++)
                        {
                            if (matriz[j][i] != -1)
                            {
                                sum += matriz[j][i];
                            }
                        }
                        if (sum == gabarito[0][i])
                        {
                            gabarito[0][i] = -1;
                        }
                    }
                }
                for (int i = 0; i < cols; i++)
                {
                    if (gabarito[1][i] != -1)
                    {
                        int sum = 0;
                        for (int j = 0; j < cols; j++)
                        {
                            if (matriz[i][j] != -1)
                            {
                                sum += matriz[i][j];
                            }
                        }
                        if (sum == gabarito[1][i])
                        {
                            gabarito[1][i] = -1;
                        }
                    }
                }
            }
            else
            {
                lifes--;
                printf("\033\n[31m>>>> Wrong number! <<<<\033[0m\n");
                pressEnterToContinue();
            }
        }

        if (lifes == 0)
        {
            system(CLEAR);

            printf("\033[40m          \033[1;31m##### GAME OVER #####          \033[0m\n\n");
            printf("\033[31mYou lost all your lifes! Better luck next time!\033[0m\n\n");

            pressEnterToContinue();
            return 2;
        }

        win = 1;

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (gabarito[i][j] != -1)
                {
                    win = 0;
                    break;
                }
            }
        }

        if (win)
        {
            system(CLEAR);

            printf("\033[40m          \033[1;32m##### YOU WIN THE PHASE #####          \033[0m\n\n");
            printf("\033[32mCongratulations! You completed the phase!\033[0m\n\n");
            printf("You won \033[32m%d\033[0m points!\n\n", difficulty * 50);

            addingPoints(difficulty * 50);

            pressEnterToContinue();

            return 1;
        }
    }
}

void saveNewPlayers(char name[50])
{
    FILE *ranking;

    ranking = fopen("../ranking.bin", "r+b");

    if (ranking == NULL)
    {
        printf("\033[31m>>>> Error opening file ranking.txt <<<<\033[0m\n");
        SLEEP(3);
        return;
    }

    int i = 0;
    int playerExists = 0;

    while (fread(&players[i], sizeof(Data), 1, ranking) == 1) // Verificar se o jogador já existe no banco de dados
    {
        if (strcmp(players[i].name, name) == 0)
        {
            printf("\033[40m                                          \033\n[0m");
            printf("\033[40m\033[33m      >>>> Player already exists! <<<<      \033[0m\n");
            printf("\033[40m                                          \033\n\n[0m");
            SLEEP(1);
            return;
        }
        i++;
    }

    if (!playerExists) // Se o jogador não existir, adicionar ao banco de dados
    {
        fseek(ranking, 0, SEEK_END);

        strcpy(players[i].name, name); // Adicionando o nome do jogador
        players[i].points = 0;         // Inicializando os pontos do jogador

        fwrite(&players[i], sizeof(Data), 1, ranking);

        system(CLEAR);

        printf("\033[40m                                                  \033\n[0m");
        printf("\033[40m\033[33m      >>>> New player added to Ranking! <<<<      \033[0m\n");
        printf("\033[40m                                                  \033\n[0m");
        SLEEP(1);

        fclose(ranking);
    }
}

void changeDifficult()
{

    char op;

    system(CLEAR);

    printf("\033[40m          \033[1;31m##### CHANGE DIFFICULT #####          \033[0m\n\n");
    printf("\033[1;31mChoose a difficult:\033[0m\n\n");
    printf("1 - \033[32mBeginner\033[0m\n");
    printf("2 - \033[33mIntermidiate\033[0m\n");
    printf("3 - \033[31mAdvanced\033[0m\n\n");
    printf("4 - Back to Game Configuration\n\n");

    printf("\033[1mOption:\033[0m ");
    scanf("%c", &op);
    getchar();

    switch (op)
    {
    case '1':
        system(CLEAR);

        difficulty = 1;
        phase = 1;
        printf("The game is now set to \033[32mBeginner\033[0m mode!\n\n");
        printf("You will receive 50 points for each level completed, which will be added to your \033[33mRanking\033[0m.\n\n");
        pressEnterToContinue();
        break;
    case '2':
        system(CLEAR);

        difficulty = 2;
        phase = 1;
        printf("The game is now set to \033[33mIntermediate\033[0m mode!\n\n");
        printf("You will receive 100 points for each level completed, which will be added to your \033[33mRanking\033[0m.\n\n");
        pressEnterToContinue();
        break;
    case '3':
        system(CLEAR);

        difficulty = 3;
        phase = 1;
        printf("The game is now set to \033[31mAdvanced\033[0m mode!\n\n");
        printf("You will receive 150 points for each level completed, which will be added to your \033[33mRanking\033[0m.\n\n");
        pressEnterToContinue();
        break;
    case '4':
        return;
    default:

        invalidOption();
        break;
    }
}

int compare(const void *a, const void *b)
{
    Data *player1 = (Data *)a;
    Data *player2 = (Data *)b;

    return player2->points - player1->points;
}

void clearRanking()
{
    FILE *ranking;

    ranking = fopen("../ranking.bin", "wb");

    if (ranking == NULL)
    {
        printf("\033[31m>>>> Error opening file ranking.txt <<<<\033[0m\n");
        SLEEP(3);
        return;
    }

    fclose(ranking);

    system(CLEAR);

    printf("\n\n\033[40m          \033[31m##### RANKING CLEARED #####          \033[0m\n\n");

    pressEnterToContinue();
}

void configuration()
{
    char op;
    while (1)
    {
        system(CLEAR);

        printf("\033[40m          \033[1;31m##### GAME CONFIGURATION #####          \033[0m\n\n");
        printf("\033[1;31mChoose a option:\033[0m\n\n");
        printf("1 - Ranking Clear\n");
        printf("2 - Change Difficult\n\n");
        printf("3 - Back to Menu Screen\n\n");

        printf("\033[1mOption:\033[0m ");
        scanf("%c", &op);
        getchar();

        switch (op)
        {
        case '1':
            printf("\033[1;31mAre you sure you want to clear the ranking? \033[0m(y/n): ");
            scanf("%c", &op);
            getchar();
            if (tolower(op) == 'y')
            {
                clearRanking();
            }
            else
            {
                break;
            }
        case '2':

            changeDifficult();
            break;

        case '3':

            return;

        default:

            invalidOption();
            break;
        }
    }
}

void ranking()
{

    system(CLEAR);

    FILE *ranking;

    ranking = fopen("../ranking.bin", "rb");

    if (ranking == NULL)
    {
        printf("\033[31m>>>> Error opening file ranking.txt <<<<\033[0m\n");
        SLEEP(3);
        return;
    }

    fseek(ranking, 0, SEEK_END);
    long size = ftell(ranking);  // Armazena os bytes total do arquivo
    int n = size / sizeof(Data); // Divide o total de bytes pelo tamanho da struct para saber a quantidade de jogadores

    if (n == 0)
    {
        printf("\033[40m\033[1;31m############ RANKING ############\033[0m\n\n");
        printf("\033[31mThere are no players in the ranking yet!\033[0m\n\n");
        pressEnterToContinue();
        return;
    } // Verificar se o ranking está vazio

    Data *rankingPlayers = malloc(size); // Struct para armazenar os jogadores
    if (rankingPlayers == NULL)
    {
        printf("\033[31m>>>> Error allocating memory <<<<\033[0m\n");
        SLEEP(3);
        return;
    }

    for (int i = 0; i < n; i++)
    {
        fseek(ranking, i * sizeof(Data), SEEK_SET);
        fread(&rankingPlayers[i], sizeof(Data), 1, ranking);
    }
    fclose(ranking);

    qsort(rankingPlayers, n, sizeof(Data), compare);

    printf("\033[40m\033[1;31m  ########## RANKING ##########  \033[0m\n\n");
    if (n > 0)
    {
        printf("\033[31m      >>>>> FIRST PLACE <<<<<      \033[0m\n\n");
        printf("\033[33m            Name:\033[0m %s\n            \033[33mPoints:\033[0m %d\n\n", rankingPlayers[0].name, rankingPlayers[0].points);
    }
    if (n > 1)
    {
        printf("\033[31m      >>>>> SECOND PLACE <<<<<      \033[0m\n\n");
        printf("\033[33m            Name:\033[0m %s\n            \033[33mPoints:\033[0m %d\n\n", rankingPlayers[1].name, rankingPlayers[1].points);
    }
    if (n > 2)
    {
        printf("\033[31m      >>>>> THIRD PLACE <<<<<      \033[0m\n\n");
        printf("\033[33m            Name:\033[0m %s\n            \033[33mPoints:\033[0m %d\n\n", rankingPlayers[2].name, rankingPlayers[2].points);
    }
    if (n > 3)
    {
        printf("\033[31m            Other Players: \033[0m\n\n");

        for (int i = 3; i < n; i++)
        {
            printf("\033[33m            Name:\033[0m %s\n            \033[33mPoints:\033[0m %d\n\n", rankingPlayers[i].name, rankingPlayers[i].points);
        }
    }

    free(rankingPlayers);
    pressEnterToContinue();
}

void instruction()
{
    system(CLEAR);

    printf("\033[40m          \033[1;31m##### INSTRUCTIONS #####          \033[0m\n\n");

    printf("The game consists of a matrix of varying sizes depending on the difficulty of the game:\n\n");
    printf("\033[32mBeginner: 4x4\033[0m\n");
    printf("\033[33mIntermediate: 6x6\033[0m\n");
    printf("\033[31mAdvanced: 7x7\033[0m\n\n");

    printf("The goal is to erase numbers that are not part of the matrix, which will give the corresponding sum and complete all the rows and columns of the table.\n");
    printf("But be careful, if you get the wrong number to erase, you will lose one of your << 5 >> lives. If your lives reach << 0 >>, you may be in for a \033[31msurprise...\033[0m\n\n");

    printf("If you \033[32mwin\033[0m, you will earn points and increase your \033[33m$$ ranking $$\033[0m within the game.\n\n");
    printf("Have a good game!!!\n\n");

    pressEnterToContinue();
}

void introductionScreen()
{
    system(CLEAR);

    printf("                \033[31mNumber Sums - By @S3r4ph1el\033[0m\n");
    printf("-------------------------------------------------------------\n\n");
    printf(" A project for the Algorithm and Computer Programming course\n at University of Brasilia based on the \033[1;33mNumber Sums\033[0m game.\n\n");

    SLEEP(5);

    system(CLEAR);

    printf("\033[40m                                          \033\n[0m");
    printf("\033[40m\033[33m           Enter a Player Name:           \033[0m\n");
    printf("\033[40m                                          \033\n\n[0m");
    printf("              ");

    fgets(name, 50, stdin); // Pegar nome do usuário
    name[strcspn(name, "\n")] = 0;

    SLEEP(1);

    saveNewPlayers(name);

    SLEEP(1);
    system(CLEAR);

    printf("\033[40m   Hello, \033[1;33m%s\033[0m\033[40m! Welcome to the Number Sums game!   \033[0m\n", name);

    SLEEP(2);
}

char menuScreen(char option)
{
    char *string;

    if (difficulty == 1)
    {
        string = "Beginner";
    }
    else if (difficulty == 2)
    {
        string = "Intermediate";
    }
    else if (difficulty == 3)
    {
        string = "Advanced";
    }

    printf("\033[1;31mChoose a option:\033[0m\n\n");
    if (difficulty == 1)
    {
        printf("1 - Play                          \033[1mDifficulty:\033[0m \033[32m%s\033[0m\n", string);
    }
    else if (difficulty == 2)
    {
        printf("1 - Play                          \033[1mDifficulty:\033[0m \033[33m%s\033[0m\n", string);
    }
    else if (difficulty == 3)
    {
        printf("1 - Play                          \033[1mDifficulty:\033[0m \033[31m%s\033[0m\n", string);
    }
    printf("2 - Game Configuration            \033[1mPhase:\033[0m %d\n", phase);
    printf("3 - Ranking\n");
    printf("4 - Instructions\n\n");
    printf("5 - Exit\n\n");

    printf("\033[1mOption:\033[0m ");
    scanf("%c", &option);
    getchar();

    return option;
}

int main()
{

    introductionScreen();

    while (1)
    {

        system(CLEAR);
        option = menuScreen(option);

        switch (option)
        {
        case '1':

            if (play(difficulty) == 1)
            {
                phase++;
                if (phase == 5) // Verificar se o jogador completou todas as fases da dificuldade
                {
                    difficulty++;
                    phase = 1;
                }
                if (difficulty == 4) // Verificar se o jogador completou todas as dificuldades
                {
                    difficulty = 1;
                    phase = 1;
                    system(CLEAR);

                    printf("\033[40m          \033[1;32m##### YOU WIN #####          \033[0m\n\n");
                    printf("\033[32mCongratulations! You completed the game!\033[0m\n\n");

                    pressEnterToContinue();
                }
            }
            break;

        case '2':

            configuration();
            break;

        case '3':

            ranking();
            break;

        case '4':

            instruction();
            break;

        case '5':

            system(CLEAR);

            printf("Exiting Game...\n");
            SLEEP(2);

            system(CLEAR);

            printf("Goodbye, %s!\n", name);
            SLEEP(1);

            return 0;

        default:
            invalidOption();
            break;
        }
    }
}