#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX_CMD_LENGTH 256
#define MAX_CMDS 1024

char *cmdTable[MAX_CMDS];
int cmdCount = 0;

// Cargar comandos de /usr/bin a memoria
void LoadCommands() {
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir("/usr/bin");
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL && cmdCount < MAX_CMDS) {
            if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                cmdTable[cmdCount] = strdup(entry->d_name);
                cmdCount++;
            }
        }
        closedir(dir);
    } else {
        perror("No se pudo abrir el directorio /usr/bin");
        exit(EXIT_FAILURE);
    }

    dir = opendir("/bin");
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL && cmdCount < MAX_CMDS) {
            if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                cmdTable[cmdCount] = strdup(entry->d_name);
                cmdCount++;
            }
        }
        closedir(dir);
    } else {
        perror("No se pudo abrir el directorio /bin");
        exit(EXIT_FAILURE);
    }
}

// Verificar si existe el commando (input) en la tabla de comandos
int IsCommandInTable(const char *cmd) {
    for (int i = 0; i < cmdCount; i++) {
        if (strcmp(cmdTable[i], cmd) == 0) {
            return 1;
        }
    }
    return 0;
}

// Liberar memoria de la tabla comandos
void FreeCommandsMemory() {
    for (int i = 0; i < cmdCount; i++) {
        free(cmdTable[i]);
    }
}

// Separa en Tokens el comando (input)
void TokenizeUserInput(char *command, char *tokens[]) {
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL; 
}

// Imprime los tokens del comando (input)
void PrintTokens(char *tokens[]) {
    for (int i = 0; tokens[i] != NULL; i++) {
        printf("Token %d: %s\n", i, tokens[i]);
    }
}

// Calculo de Distancia Hamimg
void DistanciaHamming(const char *str1, const char *str2, char *recommendations[], int *recommendationCount) {
    
    if (strlen(str1) == strlen(str2)) { // Valida longitud igual
        int distancia = 0;
        
        for (int i = 0; i < strlen(str1); i++) {
            if (str1[i] != str2[i]) { // Revisar char por char si son diferentes
                distancia++;
            }
        }

        // Agregar a recomendaciones
        if (strlen(str1) <= 10) {
            if (distancia <= (int)(strlen(str1) * 0.5)) {
                // Guarda el comando si la distancia es menor o igual a la mitad de la longitud
                recommendations[*recommendationCount] = strdup(str2);
                (*recommendationCount)++;
            }
        } else if (strlen(str1) > 10) {
            if (distancia <= (int)(strlen(str1) * 0.6)) {
                // Guarda el comando si la distancia es menor o igual al 60% de la longitud
                recommendations[*recommendationCount] = strdup(str2);
                (*recommendationCount)++;
            }
        }
    }
}

// Comparar dos caracteres en quick sort
int Comparador(const void *a, const void *b) {
    return (*(char *)a - *(char *)b);
}

// Calculo de Anagramas
void SonAnagramas(const char *str1, const char *str2, char *recommendations[], int *recommendationCount) {

    if (strlen(str1) == strlen(str2)) { // Valida longitud igual

        char *copia1 = strdup(str1);
        char *copia2 = strdup(str2);

        // Ordenar los caracteres de los strings
        qsort(copia1, strlen(copia1), sizeof(char), Comparador);
        qsort(copia2, strlen(copia2), sizeof(char), Comparador);

        // Revisar igualdad de strings ordenados
        if (strcmp(copia1, copia2) == 0) {
            recommendations[*recommendationCount] = strdup(str2);
            (*recommendationCount)++;
        }

        free(copia1);
        free(copia2);
    }
}

// Unir los tokens de la recomendacion a un string
void JoinUserRecommendation(char *recommendation, char *tokens[], char *newCommand) {
    strcpy(newCommand, recommendation);
    strcat(newCommand, " ");

    int i = 1;
    
    while (tokens[i] != NULL) {
        strcat(newCommand, tokens[i]);
        strcat(newCommand, " ");
        i++;
    }

    newCommand[strlen(newCommand) - 1] = '\0';
}

// Imprimir la tabla de comandos
void ListCommandsTable() {
    for (int i = 0; i < cmdCount; i++) {
        printf("%s\n", cmdTable[i]);
    }
}

// Eliminar recomendaciones duplicadas
void DeleteDuplicatedReccomendations(char *recommendations[], int *recommendationCount) {
    for (int i = 0; i < *recommendationCount; i++) {
        for (int j = i + 1; j < *recommendationCount; j++) {
            if (strcmp(recommendations[i], recommendations[j]) == 0) {
                free(recommendations[j]);

                for (int k = j; k < *recommendationCount - 1; k++) {
                    recommendations[k] = recommendations[k + 1];
                }
                (*recommendationCount)--;
                j--;
            }
        }
    }
}

int main() {
    printf("Bienvenido a dwimsh - Escrito por Daniel Juarez\n");
    
    LoadCommands(); // Cargar comandos a memoria

    char command[256];
    char *tokens[256];
    char *recommendations[MAX_CMDS];
    int recommendationCount  = 0;
    pid_t pid;
    int status;

    do {
        printf("dwimsh> ");
        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;  

        TokenizeUserInput(command, tokens);  // Tokenizar el comando

        if (tokens[0] == NULL || strlen(tokens[0]) == 0) { // Si se ingresa un comando vacio
            printf("No se ingreso ningun comando\n");
            continue;
        }

        if (strcmp(tokens[0], "exit") == 0) { // Salir del programa
            break;
        } else {
            if (IsCommandInTable(tokens[0])) { // Verificar si el comando está en la tabla de comandos
                pid = fork();
                if (pid < 0) {
                    perror("Error en fork");
                } else if (pid == 0) {
                    execvp(tokens[0], tokens);
                    perror("Fallo en la ejecucion");
                    exit(EXIT_FAILURE);
                } else {
                    wait(&status);
                }
            } else {
                printf("Comando no encontrado en /usr/bin\n");

                // Buscar recomendaciones
                for (int i = 0; i < cmdCount; i++) {
                    SonAnagramas(tokens[0], cmdTable[i], recommendations, &recommendationCount);
                    DistanciaHamming(tokens[0], cmdTable[i], recommendations, &recommendationCount);
                }

                if (recommendationCount == 0){
                    printf("No se encontraron comandos similares. Intente de nuevo.\n");
                } else {
                    DeleteDuplicatedReccomendations(recommendations, &recommendationCount); // Eliminar recomendaciones duplicadas
                    printf("Recomendaciones Totales: %d\n", recommendationCount);

                    char userInput[3];
                    for (int i = 0; i < recommendationCount; i++) { // Imprimir recomendaciones
                        printf("Quiere decir: \"%s", recommendations[i]);
                        
                        for (int j = 1; tokens[j] != NULL; j++) {
                            printf(" %s", tokens[j]);
                        }

                        printf("\"? [s/n] ");
                        fgets(userInput, sizeof(userInput), stdin);  // Leer confirmacion de recomendacion

                        userInput[strcspn(userInput, "\n")] = 0;

                        if (userInput[0] == 's' || userInput[0] == 'S') {
                            char newCommand[256];
                            JoinUserRecommendation(recommendations[i], tokens, newCommand); // Hacer string la recomendacion aceptada
                            char *newCommandTokens[256];
                            TokenizeUserInput(newCommand, newCommandTokens); // Tokenizar el nuevo comando

                            pid_t pid = fork();
                            
                            if (pid < 0) {
                                perror("Error en fork");
                            } else if (pid == 0) {
                                execvp(newCommandTokens[0], newCommandTokens);
                                perror("Error al ejecutar el comando");
                                exit(EXIT_FAILURE);
                            } else {
                                int status;
                                wait(&status);
                            }
                            break;
                        } else if (userInput[0] == 'n' || userInput[0] == 'N') { // Mostrar siguiente recomendacion
                            continue;
                        } else {
                            printf("Ingrese una Opcion Valida.\n");
                            i--;  // Repetir la recomendacion
                        }
                    }
                }
                
                // Vaciar recomendaciones
                for (int i = 0; i < recommendationCount; i++) {
                    free(recommendations[i]);
                }
                            
                recommendationCount = 0;
            }
        }
    } while (1);

    // Liberar memoria 
    FreeCommandsMemory();

    return 0;
}
