#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BOOKS 100
#define MAX_USERS 50
#define MAX_STR 50

// --- STRUCTURES ---

typedef struct {
    int day;
    int month;
    int year;
    int hour;
    int minute;
} Date;

typedef struct {
    int id;
    char title[MAX_STR];
    char author[MAX_STR];
    char category[MAX_STR];
    int isBorrowed;          // 0: Non, 1: Oui
    char borrowerLogin[MAX_STR];
    Date dueDate;
} Book;

typedef struct {
    char login[MAX_STR];
    char password[MAX_STR];
    char role[MAX_STR];      // "student" ou "professor"
} User;

// --- VARIABLES GLOBALES ---

Book library[MAX_BOOKS];
int bookCount = 0;
User users[MAX_USERS];
int userCount = 0;
User *currentUser = NULL;

// --- OUTILS DE DATE ---

Date getCurrentDate() {
    time_t t = time(NULL);
    struct tm *info = localtime(&t);
    Date d;
    d.day = info->tm_mday;
    d.month = info->tm_mon + 1;
    d.year = info->tm_year + 1900;
    d.hour = info->tm_hour;
    d.minute = info->tm_min;
    return d;
}

// Retourne 1 si d1 est strictement AVANT d2
int isBefore(Date d1, Date d2) {
    if (d1.year != d2.year) return d1.year < d2.year;
    if (d1.month != d2.month) return d1.month < d2.month;
    if (d1.day != d2.day) return d1.day < d2.day;
    if (d1.hour != d2.hour) return d1.hour < d2.hour;
    return d1.minute < d2.minute;
}

// --- GESTION DES FICHIERS ---

void saveData() {
    FILE *fUsers = fopen("users.txt", "w");
    if (fUsers) {
        for (int i = 0; i < userCount; i++) {
            fprintf(fUsers, "%s %s %s\n", users[i].login, users[i].password, users[i].role);
        }
        fclose(fUsers);
    }

    FILE *fBooks = fopen("books3.txt", "w");
    if (fBooks) {
        for (int i = 0; i < bookCount; i++) {
            fprintf(fBooks, "%d; %[^;]; %[^;]; %[^;]; %d %[^;]; %[^;]; %[^;] %[^;]; %[^;]; %d\n",
                    library[i].id, library[i].title, library[i].author, library[i].category,
                    library[i].isBorrowed,
                    (strlen(library[i].borrowerLogin) > 0) ? library[i].borrowerLogin : "none",
                    library[i].dueDate.day, library[i].dueDate.month, library[i].dueDate.year,
                    library[i].dueDate.hour, library[i].dueDate.minute);
        }
        fclose(fBooks);
    }
}

void loadData() {

    FILE *fUsers = fopen("users.txt", "r");
    if (fUsers) {
        userCount = 0;
        while (fscanf(fUsers, "%s %s %s", users[userCount].login, users[userCount].password, users[userCount].role) == 3) {
            userCount++;
        }
        fclose(fUsers);
    }

    FILE *fBooks = fopen("books3.txt", "r");
    if (fBooks) {
        bookCount = 0;
        

        while (fscanf(fBooks, "%d;%[^;];%[^;];%[^;];%d;%[^;];%[^;];%[^;];%[^;];%[^;];%d",
                      &library[bookCount].id, library[bookCount].title, library[bookCount].author, library[bookCount].category,
                      &library[bookCount].isBorrowed, library[bookCount].borrowerLogin, &library[bookCount].dueDate.day, &library[bookCount].dueDate.month, &library[bookCount].dueDate.year, &library[bookCount].dueDate.hour, &library[bookCount].dueDate.minute)
                      == 11) {
        
            if (strcmp(library[bookCount].borrowerLogin, "none") == 0) {
                strcpy(library[bookCount].borrowerLogin, "");
            }
            bookCount++;
        }
        fclose(fBooks);
        
    }
}

// --- FONCTIONNALITÉS ---

void createAccount() {
    if (userCount >= MAX_USERS) return;
    printf("\n--- Création de compte ---\nLogin : ");
    scanf("%s", users[userCount].login);
    printf("Mot de passe : ");
    scanf("%s", users[userCount].password);
    printf("Rôle (student/professor) : ");
    scanf("%s", users[userCount].role);
    userCount++;
    saveData();
    printf("Compte créé avec succès !\n");
}

int loginUser() {
    char l[MAX_STR], p[MAX_STR];
    printf("\nConnexion : "); scanf("%s", l);
    printf("Mot de passe : "); scanf("%s", p);
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].login, l) == 0 && strcmp(users[i].password, p) == 0) {
            currentUser = &users[i];
            return 1;
        }
    }
    return 0;
}

int hasOverdueBooks() {
    Date now = getCurrentDate();
    int overdueCount = 0;
    for (int i = 0; i < bookCount; i++) {
        if (library[i].isBorrowed && strcmp(library[i].borrowerLogin, currentUser->login) == 0) {
            if (isBefore(library[i].dueDate, now)) {
                overdueCount++;
            }
        }
    }
    return overdueCount;
}

void searchBooks() {
printf("Debug: Nombre de livres en mémoire =%d\n", bookCount);
    char cat[MAX_STR];
    printf("Catégorie souhaitée (ou 'tous') : ");
    scanf("%s", cat);
    printf("\n--- Résultats ---\n");
    for (int i = 0; i < bookCount; i++) {
        if (strcmp(cat, "tous") == 0 || strcmp(library[i].category, cat) == 0) {
            printf("[%d] %s - %s | %s", library[i].id, library[i].title, library[i].author,
                   library[i].isBorrowed ? "EMPRUNTÉ" : "Disponible");
            if (library[i].isBorrowed) {
                printf(" (À rendre le : %02d/%02d à %02d:%02d)",
                       library[i].dueDate.day, library[i].dueDate.month,
                       library[i].dueDate.hour, library[i].dueDate.minute);
            }
            printf("\n");
        }
    }
}

void borrowBook() {
    if (hasOverdueBooks() > 0) {
        printf("Erreur : Vous avez des livres en retard ! Rendez-les d'abord.\n");
        return;
    }

    int id, count = 0;
    for(int i = 0; i < bookCount; i++) {
        if(library[i].isBorrowed && strcmp(library[i].borrowerLogin, currentUser->login) == 0)
            count++;
    }

    int maxLimit = (strcmp(currentUser->role, "professor") == 0) ? 5 : 3;
    if (count >= maxLimit) {
        printf("Limite de %d livres atteinte.\n", maxLimit);
        return;
    }

    printf("ID du livre : "); scanf("%d", &id);
    for (int i = 0; i < bookCount; i++) {
        if (library[i].id == id && !library[i].isBorrowed) {
            library[i].isBorrowed = 1;
            strcpy(library[i].borrowerLogin, currentUser->login);

            Date d = getCurrentDate();
            int delay = (strcmp(currentUser->role, "professor") == 0) ? 3 : 2; // Test en minutes
           
            d.minute += delay;
            if (d.minute >= 60) { d.minute -= 60; d.hour++; }
            if (d.hour >= 24)  { d.hour = 0; d.day++; }

            library[i].dueDate = d;
            saveData();
            printf("Emprunt réussi ! À rendre avant %02d:%02d\n", d.hour, d.minute);
            return;
        }
    }
    printf("Livre indisponible ou inexistant.\n");
}

void addBook() {
    if (strcmp(currentUser->role, "professor") != 0) {
        printf("Accès refusé : réservé aux professeurs.\n");
        return;
    }
    if (bookCount >= MAX_BOOKS) return;

    printf("Titre (utilisez_des_underscores) : "); scanf("%s", library[bookCount].title);
    printf("Auteur : "); scanf("%s", library[bookCount].author);
    printf("Catégorie : "); scanf("%s", library[bookCount].category);
    library[bookCount].id = 100 + bookCount;
    library[bookCount].isBorrowed = 0;
    bookCount++;
    saveData();
    printf("Livre ajouté avec l'ID %d\n", library[bookCount-1].id);
}

// --- MAIN ---

int main() {
    loadData();

    // Admin par défaut si les fichiers sont vides
    if (userCount == 0) {
        strcpy(users[0].login, "admin");
        strcpy(users[0].password, "123");
        strcpy(users[0].role, "professor");
        userCount = 1;
    }

    int choice;
    while (1) {
        if (!currentUser) {
            printf("\n--- ACCUEIL ---\n1. Connexion\n2. Créer un compte\n3. Quitter\nChoix : ");
            if (scanf("%d", &choice) != 1) break;
            if (choice == 1) {
                if (!loginUser()) printf("Échec de la connexion.\n");
            }
            else if (choice == 2) createAccount();
            else break;
        } else {
            printf("\n--- MENU (%s - %s) ---\n", currentUser->login, currentUser->role);
            printf("1. Rechercher/Voir livres\n2. Emprunter un livre\n3. Ajouter un livre (Prof)\n4. Déconnexion\nChoix : ");
            if (scanf("%d", &choice) != 1) break;
            if (choice == 1) searchBooks();
            else if (choice == 2) borrowBook();
            else if (choice == 3) addBook();
            else currentUser = NULL;
        }
    }
    return 0;
}


