#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#define MAX_BOOKS 1000
#define MAX_MEDICINES 100
#define MAX_LOGS 1000
#define UP 72
#define DOWN 80
#define ENTER 13
#define ESC 27

// Structures and global variables
typedef struct {
    char title[50];      // Title of the book
    char author[30];     // Author of the book
    char department[20]; // Department the book belongs to (e.g., CSE)
    int semester;        // Semester the book is relevant for
    int available;       // Number of copies available (0 = not available)
    int shelfRow;        // Shelf row number where the book is located
    int shelfCol;        // Shelf column number where the book is located
    char studentID[20];  // Student ID of the person who reserved/lent the book
    char lentDate[20];   // Date the book was lent
    char dueDate[20];    // Due date for returning the book
    int timerActive;     // 1 = Timer active, 0 = Timer inactive
} Book;

typedef struct {
    char name[50];
    char company[30];
    int stock;
    char expiryDate[20];
    int shelfRow;
    int shelfCol;
    float price;
} Medicine;

typedef struct {
    char title[50];
    char studentID[20];
    char action[10];
    char date[20];
} BookLog;

Book books[MAX_BOOKS];
Medicine medicines[MAX_MEDICINES];
BookLog bookLogs[MAX_LOGS];

int numBooks = 0;
int numMedicines = 0;
int numLogs = 0;

// Function prototypes
int isExpired(const char expiryDate[]); // Added prototype
void clearScreen();
void gotoxy(int x, int y);
int levenshteinDistance(const char *s1, const char *s2);
int menuSelection(const char *options[], int numOptions);
void displayBookList(Book books[], int count);
void displayMedicineList(Medicine medicines[], int count);
void setDueDate(char dueDate[], int days);
void loadBooks();
void saveBooks();
void loadMedicines();
void saveMedicines();
void loadBookLogs();
void saveBookLogs();
void addLogEntry(const char *title, const char *studentID, const char *action);
void displayBookLogs();
void returnBook();
void verifyBookLocation();
void checkOverdueBooks();
void findBooks();
void addBook();
void removeBook();
void updateBook();
void findMedicine();
void verifyMedicineLocation();
void expiredMedicines();
void addMedicine();
void removeMedicine();
void updateMedicine();
void exitProgram();
void librarySystem();
void pharmacySystem();
void mainMenu();
int isExpired(const char expiryDate[]); // Added prototype

// Main function
int main() {
    loadBooks();
    loadMedicines();
    loadBookLogs();
    mainMenu();
    return 0;
}

// Function implementations
void clearScreen() {
    system("cls");
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int levenshteinDistance(const char *s1, const char *s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int dp[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) dp[i][0] = i;
    for (int j = 0; j <= len2; j++) dp[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = fmin(fmin(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + cost);
        }
    }

    return dp[len1][len2];
}

// Case-insensitive substring search (custom implementation for Windows)
const char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return haystack; // Empty needle matches everything

    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (tolower(*h) == tolower(*n))) {
            h++;
            n++;
        }

        if (!*n) return haystack; // Full match found
    }

    return NULL; // No match found
}


int menuSelection(const char *options[], int numOptions) {
    int selected = 0;
    int key;

    while (1) {
        clearScreen();
        printf("\n\t========================================\n");
        printf("\t|      Welcome to ShelfX Inventory     |\n");
        printf("\t========================================\n");

        for (int i = 0; i < numOptions; i++) {
            if (i == selected) {
                printf("\t\033[1;48;5;10;37m  %s  \033[0m\n", options[i]);
            } else {
                printf("\t  %s\n", options[i]);
            }
        }

        key = getch();
        if (key == UP) {
            selected = (selected == 0) ? numOptions - 1 : selected - 1;
        } else if (key == DOWN) {
            selected = (selected == numOptions - 1) ? 0 : selected + 1;
        } else if (key == ENTER) {
            return selected;
        } else if (key == ESC) {
            return -1;
        }
    }
}

void displayBookList(Book books[], int count) {
    printf("\n\t+----------------------------------------------------------------------------+\n");
    for (int i = 0; i < count; i++) {
        Book *book = &books[i];
        printf("\t| %-2d. %-40s                               |\n", i + 1, book->title);
        printf("\t|     Author: %-35s                            |\n", book->author);
        printf("\t|     Shelf: Row %-2d, Column %-2d                                               |\n", book->shelfRow, book->shelfCol);
        printf("\t|     Available: %-3d                                                         |\n", book->available);
        printf("\t+----------------------------------------------------------------------------+\n");
    }
}

void displayMedicineList(Medicine medicines[], int count) {
    printf("\n\t+--------------------------------------------------------+\n");
    for (int i = 0; i < count; i++) {
        Medicine *medicine = &medicines[i];
        printf("\t| %-2d. %-40s           |\n", i + 1, medicine->name);
        printf("\t|     Company: %-30s            |\n", medicine->company);
        printf("\t|     Stock: %-3d Expiry: %-10s                      |\n", medicine->stock, medicine->expiryDate);
        printf("\t|     Shelf: Row %-2d, Column %-2d                           |\n", medicine->shelfRow, medicine->shelfCol);
        printf("\t|     Price: $%-10.2f                                 |\n", medicine->price);
        printf("\t+--------------------------------------------------------+\n");
    }
}

void setDueDate(char dueDate[], int days) {
    time_t now = time(NULL);
    struct tm *due = localtime(&now);
    due->tm_mday += days;
    mktime(due);
    strftime(dueDate, 20, "%d/%m/%Y", due);
}

void loadBooks() {
    FILE *file = fopen("books.txt", "r");
    if (!file) {
        printf("Error: Could not open books.txt\n");
        return;
    }

    char line[256];
    numBooks = 0; // Reset the number of books
    while (fgets(line, sizeof(line), file)) {
        if (numBooks >= MAX_BOOKS) break; // Prevent overflow

        // Parse the line into book fields
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^|]|%[^|]|%[^|]|%d",
               books[numBooks].title, books[numBooks].author, books[numBooks].department, &books[numBooks].semester,
               &books[numBooks].available, &books[numBooks].shelfRow, &books[numBooks].shelfCol,
               books[numBooks].studentID, books[numBooks].lentDate, books[numBooks].dueDate, &books[numBooks].timerActive);
        numBooks++;
    }

    fclose(file);
}

void saveBooks() {
    FILE *file = fopen("books.txt", "w");
    if (!file) {
        printf("Error: Could not open books.txt for writing\n");
        return;
    }

    for (int i = 0; i < numBooks; i++) {
        fprintf(file, "%s|%s|%s|%d|%d|%d|%d|%s|%s|%s|%d\n",
                books[i].title, books[i].author, books[i].department, books[i].semester,
                books[i].available, books[i].shelfRow, books[i].shelfCol,
                books[i].studentID, books[i].lentDate, books[i].dueDate, books[i].timerActive);
    }

    fclose(file);
}

void loadMedicines() {
    FILE *file = fopen("medicines.txt", "r");
    if (!file) {
        printf("Error: Could not open medicines.txt\n");
        return;
    }

    char line[256];
    numMedicines = 0;
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline character
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines or lines starting with '#' (comments)
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        if (numMedicines >= MAX_MEDICINES) {
            printf("Warning: Maximum number of medicines reached. Some entries may not be loaded.\n");
            break;
        }

        // Parse the line and check if all fields were successfully read
        int parsed = sscanf(line, "%[^|]|%[^|]|%d|%[^|]|%d|%d|%f",
                            medicines[numMedicines].name, medicines[numMedicines].company, &medicines[numMedicines].stock,
                            medicines[numMedicines].expiryDate, &medicines[numMedicines].shelfRow, &medicines[numMedicines].shelfCol,
                            &medicines[numMedicines].price);

        // If all 7 fields were not parsed, skip the line
        if (parsed != 7) {
            printf("Warning: Invalid data format in medicines.txt at line %d. Skipping this entry.\n", numMedicines + 1);
            continue;
        }

        numMedicines++;
    }

    fclose(file);
}

void saveMedicines() {
    FILE *file = fopen("medicines.txt", "w");
    if (!file) {
        printf("Error: Could not open medicines.txt for writing\n");
        return;
    }

    for (int i = 0; i < numMedicines; i++) {
        fprintf(file, "%s|%s|%d|%s|%d|%d|%.2f\n",
                medicines[i].name, medicines[i].company, medicines[i].stock,
                medicines[i].expiryDate, medicines[i].shelfRow, medicines[i].shelfCol,
                medicines[i].price);
    }

    fclose(file);
}

void loadBookLogs() {
    FILE *file = fopen("booklogs.txt", "r");
    if (!file) {
        printf("Error: Could not open booklogs.txt\n");
        return;
    }

    char line[256];
    numLogs = 0;
    while (fgets(line, sizeof(line), file)) {
        if (numLogs >= MAX_LOGS) break;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]",
               bookLogs[numLogs].title, bookLogs[numLogs].studentID, bookLogs[numLogs].action, bookLogs[numLogs].date);
        numLogs++;
    }

    fclose(file);
}

void saveBookLogs() {
    FILE *file = fopen("booklogs.txt", "w");
    if (!file) {
        printf("Error: Could not open booklogs.txt for writing\n");
        return;
    }

    for (int i = 0; i < numLogs; i++) {
        fprintf(file, "%s|%s|%s|%s\n",
                bookLogs[i].title, bookLogs[i].studentID, bookLogs[i].action, bookLogs[i].date);
    }

    fclose(file);
}

void addLogEntry(const char *title, const char *studentID, const char *action) {
    if (numLogs >= MAX_LOGS) {
        printf("Error: Log storage full. Cannot add more logs.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    char date[20];
    strftime(date, 20, "%d/%m/%Y", current);

    strcpy(bookLogs[numLogs].title, title);
    strcpy(bookLogs[numLogs].studentID, studentID);
    strcpy(bookLogs[numLogs].action, action);
    strcpy(bookLogs[numLogs].date, date);

    numLogs++;
    saveBookLogs();
}

void displayBookLogs() {
    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          BOOK TRANSACTION LOGS           |\n");
    printf("\t============================================\n");

    if (numLogs == 0) {
        printf("\n\t\033[1;31mNo logs found.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    for (int i = 0; i < numLogs; i++) {
        printf("\n\tTitle: %s\n", bookLogs[i].title);
        printf("\tStudent ID: %s\n", bookLogs[i].studentID);
        printf("\tAction: %s\n", bookLogs[i].action);
        printf("\tDate: %s\n", bookLogs[i].date);
        printf("\t--------------------------------------------\n");
    }

    printf("\n\t1. Press 'C' to clear all logs\n");
    printf("\t2. Press any other key to go back...");

    char choice = _getch();

    if (choice == 'C' || choice == 'c') {
        numLogs = 0;
        printf("\n\t\033[1;32mAll logs have been cleared.\033[0m\n");
        printf("\tPress any key to go back...");
        _getch();
    }
}

void returnBook() {
    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          RETURN A BOOK                   |\n");
    printf("\t============================================\n");

    if (numBooks == 0) {
        printf("\n\t\033[1;31mNo books found to return.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    // Ask for the student's ID
    char studentID[20];
    printf("\n\tEnter your Student ID: ");
    scanf("%s", studentID);

    // Find all books lent to this student
    Book lentBooks[MAX_BOOKS];
    int count = 0;
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].studentID, studentID) == 0) {
            lentBooks[count++] = books[i];
        }
    }

    if (count == 0) {
        printf("\n\t\033[1;31mNo books found that are lent to this Student ID.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    // Display all books lent to the student
    printf("\n\tBooks Lent to Student ID: %s\n", studentID);
    printf("\n\t+----------------------------------------------------------------------------+\n");
    for (int i = 0; i < count; i++) {
        Book *book = &lentBooks[i]; // Get a pointer to the current book
        printf("\t| %-2d. %-40s                               |\n", i + 1, book->title); // Display book title
        printf("\t|     Author: %-35s                            |\n", book->author); // Display author
        printf("\t|     Lent Date: %-30s                              |\n", book->lentDate); // Display actual lent date
        printf("\t|     Due Date: %-30s                               |\n", book->dueDate); // Display due date
        printf("\t|     Department: %-30s                             |\n", book->department); // Display department
        printf("\t|     Semester: %-3d                                                          |\n", book->semester); // Display semester
        printf("\t+----------------------------------------------------------------------------+\n");
    }

    // Let the user select books to return
    int selectedBooks[MAX_BOOKS] = {0}; // Track which books are selected
    int numSelected = 0;

    while (1) {
        printf("\n\tSelect a book to return (1-%d) or 0 to finish selection: ", count);
        int choice;
        if (scanf("%d", &choice) == 1 && choice >= 0 && choice <= count) {
            if (choice == 0) break; // Finish selection
            if (selectedBooks[choice - 1] == 0) {
                selectedBooks[choice - 1] = 1; // Mark the book as selected
                numSelected++;
                printf("\tBook '%s' selected for return.\n", lentBooks[choice - 1].title);
            } else {
                printf("\tBook '%s' is already selected.\n", lentBooks[choice - 1].title);
            }
        } else {
            printf("\tInvalid choice! Please try again.\n");
            while (getchar() != '\n'); // Clear input buffer
        }
    }

    if (numSelected == 0) {
        printf("\n\tNo books selected for return. Going back...\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    // Return the selected books
    for (int i = 0; i < count; i++) {
        if (selectedBooks[i] == 1) {
            // Find the book in the original books array
            for (int j = 0; j < numBooks; j++) {
                if (strcmp(books[j].title, lentBooks[i].title) == 0) {
                    // Mark the book as returned
                    books[j].available++;
                    strcpy(books[j].studentID, "");
                    strcpy(books[j].lentDate, ""); // Clear lent date
                    strcpy(books[j].dueDate, "");
                    books[j].timerActive = 0;

                    // Add a log entry for the return
                    addLogEntry(books[j].title, studentID, "Return");

                    printf("\n\t\033[1;32mBook '%s' returned successfully!\033[0m\n", books[j].title);
                    break;
                }
            }
        }
    }

    // Save changes to file
    saveBooks();

    printf("\n\tPress any key to go back...");
    _getch();
}

void verifyBookLocation() {
    srand(time(NULL)); // Seed the random number generator
    int numBooksToVerify = 3; // Number of books to verify
    int verifiedBooks[MAX_BOOKS] = {0}; // Track which books have been verified
    int count = 0;

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|      VERIFY BOOK LOCATION                |\n");
    printf("\t============================================\n");

    while (count < numBooksToVerify) {
        int index = rand() % numBooks; // Randomly select a book
        if (verifiedBooks[index]) continue; // Skip if the book has already been verified

        verifiedBooks[index] = 1; // Mark the book as verified
        count++;

        printf("\n\tBook: %s\n", books[index].title);
        printf("\tExpected Location: Row %d, Column %d\n", books[index].shelfRow, books[index].shelfCol);

        // Updated prompt
        printf("\tIs the book in the correct location? (1 = YES, 0 = NO) or 'c' to cancel verification: ");

        char input[10]; // Buffer to store user input
        scanf("%s", input); // Read the input as a string

        // Check if the user wants to cancel
        if (input[0] == 'c' || input[0] == 'C') {
            printf("\n\tVerification canceled.\n");
            printf("\tPress any key to go back...");
            _getch();
            return; // Exit the function
        }

        // Convert the input to an integer
        int confirmation = atoi(input);

        if (confirmation == 1) {
            printf("\n\t\033[1;32mBook is in the correct location!\033[0m\n");
        } else if (confirmation == 0) {
            printf("\n\t\033[1;31mBook is misplaced! Expected: Row %d, Column %d\033[0m\n", books[index].shelfRow, books[index].shelfCol);
        } else {
            printf("\n\t\033[1;31mInvalid input! Please enter 1, 0, or 'c'.\033[0m\n");
            count--; // Decrement count to retry this book
        }
    }

    printf("\n\tVerification complete! Press any key to go back...");
    _getch();
}
void checkOverdueBooks() {
    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    char currentDate[20];
    strftime(currentDate, 20, "%d/%m/%Y", current);

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|      OVERDUE BOOK NOTIFICATIONS          |\n");
    printf("\t============================================\n");

    int overdueFound = 0;
    for (int i = 0; i < numBooks; i++) {
        if (books[i].timerActive && strcmp(books[i].dueDate, currentDate) <= 0) {
            overdueFound = 1;
            printf("\n\tBook: %s\n", books[i].title);
            printf("\tBorrowed by: %s\n", books[i].studentID);
            printf("\tDue Date: %s\n", books[i].dueDate);
            printf("\t\033[1;31mThis book is overdue!\033[0m\n");
        }
    }

    if (!overdueFound) {
        printf("\n\t\033[1;32mNo overdue books found.\033[0m\n");
    }

    printf("\n\tPress any key to go back...");
    _getch();
}

void findBooks() {
    char department[20];
    int semester, found = 0;
    Book matchingBooks[MAX_BOOKS];
    int count = 0;

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|      SELECT DEPARTMENT & SEMESTER        |\n");
    printf("\t============================================\n");

    printf("\n\tEnter Department (e.g., CSE, EEE, BBA) or 'BACK' to go back: ");
    scanf("%s", department);

    if (strcmp(department, "BACK") == 0) {
        printf("\tGoing back to the previous menu...\n");
        printf("\tPress any key to continue...");
        _getch();
        return;
    }

    for (int i = 0; department[i]; i++) {
        department[i] = toupper(department[i]);
    }

    while (1) {
        printf("\tEnter Semester (1-8) or '0' to go back: ");
        if (scanf("%d", &semester) == 1) {
            if (semester == 0) {
                printf("\tGoing back to the previous menu...\n");
                printf("\tPress any key to continue...");
                _getch();
                return;
            } else if (semester >= 1 && semester <= 8) {
                break;
            }
        }
        printf("\tInvalid input! Please enter a number between 1 and 8 or '0' to go back.\n");
        while (getchar() != '\n');
    }

    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].department, department) == 0 && books[i].semester == semester) {
            matchingBooks[count++] = books[i];
            found = 1;
        }
    }

    clearScreen();
    printf("\n\t===========================================\n");
    printf("\t|       Books for %s - Semester %d        |\n", department, semester);
    printf("\t===========================================\n");

    if (!found) {
        printf("\n\t\033[1;31mNo books found for the selected Department & Semester.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    displayBookList(matchingBooks, count);

    int choice;
    while (1) {
        printf("\n\tSelect a book (1-%d) or 0 to go back: ", count);
        if (scanf("%d", &choice) == 1 && choice >= 0 && choice <= count) {
            break;
        }
        printf("\tInvalid choice! Please try again.\n");
        while (getchar() != '\n');
    }

    if (choice == 0) return;

    Book *selected = NULL;
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].title, matchingBooks[choice - 1].title) == 0) {
            selected = &books[i];
            break;
        }
    }

    if (!selected) {
        printf("\n\t\033[1;31mError: Selected book not found in the database.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    if (selected->available == 0) {
        printf("\n\t\033[1;31mNo copies available for lending!\033[0m\n");
        printf("\tDo you want to reserve it?\n");
        printf("\t[1] Reserve  [0] Back\n");
        printf("\tSelect an option: ");
        int action;
        scanf("%d", &action);

        if (action == 1) {
            char studentID[20];
            printf("\n\tEnter Student ID: ");
            scanf("%s", studentID);

            strcpy(selected->studentID, studentID);
            printf("\n\t\033[1;32mBook Reserved Successfully!\033[0m\n");

            addLogEntry(selected->title, studentID, "Reserve");
        } else if (action == 0) {
            return;
        } else {
            printf("\n\t\033[1;31mInvalid Choice!\033[0m\n");
        }
    } else {
        printf("\n\t[1] Lend  [0] Back\n");
        printf("\tSelect an option: ");
        int action;
        scanf("%d", &action);

        if (action == 1) {
            char studentID[20];
            printf("\n\tEnter Student ID: ");
            scanf("%s", studentID);

            if (strcmp(selected->studentID, studentID) == 0) {
                printf("\n\t\033[1;31mYou have already lent this book.\033[0m\n");
                printf("\n\tPress any key to go back...");
                _getch();
                return;
            }

            int days;
            printf("\n\tEnter Number of Days to Lend: ");
            scanf("%d", &days);

            // Set the lent date
            time_t now = time(NULL);
            struct tm *current = localtime(&now);
            strftime(selected->lentDate, 20, "%d/%m/%Y", current); // Format as "DD/MM/YYYY"

            // Lend the book
            selected->available--;
            strcpy(selected->studentID, studentID);
            setDueDate(selected->dueDate, days);
            selected->timerActive = 1;

            printf("\n\t\033[1;32mBook Lent Successfully! Due Date: %s\033[0m\n", selected->dueDate);

            addLogEntry(selected->title, studentID, "Lend");

            saveBooks();
        } else if (action == 0) {
            return;
        } else {
            printf("\n\t\033[1;31mInvalid Choice!\033[0m\n");
        }
    }

    printf("\n\tPress any key to go back...");
    _getch();
}

void addBook() {
    if (numBooks >= MAX_BOOKS) {
        printf("\n\t\033[1;31mCannot add more books. Maximum limit reached.\033[0m\n");
        return;
    }

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          ADD A NEW BOOK                  |\n");
    printf("\t============================================\n");

    Book newBook;

    printf("\n\tEnter Book Title (enter 'c' to cancel): ");
    if (scanf(" %[^\n]", newBook.title) != 1) {
        return; // If input fails, just return
    }
    if (strcmp(newBook.title, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return; // Cancel operation if user enters 'c'
    }

    printf("\tEnter Author: ");
    if (scanf(" %[^\n]", newBook.author) != 1) {
        return;
    }

    printf("\tEnter Department (e.g., CSE, EEE, BBA): ");
    if (scanf("%s", newBook.department) != 1) {
        return;
    }
    for (int i = 0; newBook.department[i]; i++) {
        newBook.department[i] = toupper(newBook.department[i]);
    }

    printf("\tEnter Semester (1-8): ");
    if (scanf("%d", &newBook.semester) != 1) {
        return;
    }

    printf("\tEnter Available Copies: ");
    if (scanf("%d", &newBook.available) != 1) {
        return;
    }

    printf("\tEnter Shelf Row: ");
    if (scanf("%d", &newBook.shelfRow) != 1) {
        return;
    }

    printf("\tEnter Shelf Column: ");
    if (scanf("%d", &newBook.shelfCol) != 1) {
        return;
    }

    strcpy(newBook.studentID, "");
    strcpy(newBook.dueDate, "");
    newBook.timerActive = 0;

    books[numBooks] = newBook;
    numBooks++;

    saveBooks();

    printf("\n\t\033[1;32mBook added successfully!\033[0m\n");
    printf("\n\tPress any key to go back...");
    _getch();
}

void removeBook() {
    char department[20];
    int semester;

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          REMOVE A BOOK                   |\n");
    printf("\t============================================\n");

    printf("\n\tEnter Department (e.g., CSE, EEE, BBA) or '0' to go back: ");
    scanf("%s", department);
    if (strcmp(department, "0") == 0) return;

    printf("\tEnter Semester (1-8) or '0' to go back: ");
    scanf("%d", &semester);
    if (semester == 0) return;

    Book filteredBooks[MAX_BOOKS];
    int count = 0;
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].department, department) == 0 && books[i].semester == semester) {
            filteredBooks[count++] = books[i];
        }
    }

    if (count == 0) {
        printf("\n\t\033[1;31mNo books found for the selected Department & Semester.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    displayBookList(filteredBooks, count);

    int choice;
    while (1) {
        printf("\n\tSelect a book to remove (1-%d) or 0 to go back: ", count);
        if (scanf("%d", &choice) == 1 && choice >= 0 && choice <= count) {
            break;
        }
        printf("\tInvalid choice! Please try again.\n");
        while (getchar() != '\n');
    }

    if (choice == 0) return;

    printf("\n\tRemoving book: %s\n", filteredBooks[choice - 1].title);

    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].title, filteredBooks[choice - 1].title) == 0) {
            for (int j = i; j < numBooks - 1; j++) {
                books[j] = books[j + 1];
            }
            numBooks--;
            break;
        }
    }

    saveBooks();
    printf("\n\t\033[1;32mBook removed successfully!\033[0m\n");
    printf("\n\tPress any key to go back...");
    _getch();
}

void updateBook() {
    char department[20];
    int semester;

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          UPDATE BOOK DETAILS            |\n");
    printf("\t============================================\n");

    if (numBooks == 0) {
        printf("\n\t\033[1;31mNo books found to update.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    printf("\n\tEnter Department (e.g., CSE, EEE, BBA) or '0' to go back: ");
    scanf("%s", department);
    if (strcmp(department, "0") == 0) return;

    for (int i = 0; department[i]; i++) {
        department[i] = toupper(department[i]);
    }

    printf("\tEnter Semester (1-8) or '0' to go back: ");
    scanf("%d", &semester);
    if (semester == 0) return;

    Book filteredBooks[MAX_BOOKS];
    int count = 0;
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].department, department) == 0 && books[i].semester == semester) {
            filteredBooks[count++] = books[i];
        }
    }

    if (count == 0) {
        printf("\n\t\033[1;31mNo books found for the selected Department & Semester.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    displayBookList(filteredBooks, count);

    int choice;
    while (1) {
        printf("\n\tSelect a book to update (1-%d) or 0 to go back: ", count);
        if (scanf("%d", &choice) == 1 && choice >= 0 && choice <= count) {
            break;
        }
        printf("\tInvalid choice! Please try again.\n");
        while (getchar() != '\n');
    }

    if (choice == 0) return;

    Book *selected = NULL;
    for (int i = 0; i < numBooks; i++) {
        if (strcmp(books[i].title, filteredBooks[choice - 1].title) == 0) {
            selected = &books[i];
            break;
        }
    }

    if (!selected) {
        printf("\n\t\033[1;31mError: Selected book not found in the database.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          UPDATE BOOK DETAILS            |\n");
    printf("\t============================================\n");
    printf("\n\tCurrent Details of the Selected Book:\n");
    printf("\tTitle: %s\n", selected->title);
    printf("\tAuthor: %s\n", selected->author);
    printf("\tDepartment: %s\n", selected->department);
    printf("\tSemester: %d\n", selected->semester);
    printf("\tAvailable Copies: %d\n", selected->available);
    printf("\tShelf Location: Row %d, Column %d\n", selected->shelfRow, selected->shelfCol);

    printf("\n\tWhich fields would you like to update?\n");
    printf("\t1. Title\n");
    printf("\t2. Author\n");
    printf("\t3. Department\n");
    printf("\t4. Semester\n");
    printf("\t5. Available Copies\n");
    printf("\t6. Shelf Location\n");
    printf("\t7. Update All Fields\n");
    printf("\t0. Go Back\n");

    int updateChoice;
    while (1) {
        printf("\n\tEnter your choice (0-7): ");
        if (scanf("%d", &updateChoice) == 1 && updateChoice >= 0 && updateChoice <= 7) {
            break;
        }
        printf("\tInvalid choice! Please try again.\n");
        while (getchar() != '\n');
    }

    if (updateChoice == 0) return;

    if (updateChoice == 1 || updateChoice == 7) {
        printf("\n\tEnter New Title: ");
        scanf(" %[^\n]", selected->title);
    }
    if (updateChoice == 2 || updateChoice == 7) {
        printf("\tEnter New Author: ");
        scanf(" %[^\n]", selected->author);
    }
    if (updateChoice == 3 || updateChoice == 7) {
        printf("\tEnter New Department: ");
        scanf("%s", selected->department);
        for (int i = 0; selected->department[i]; i++) {
            selected->department[i] = toupper(selected->department[i]);
        }
    }
    if (updateChoice == 4 || updateChoice == 7) {
        printf("\tEnter New Semester: ");
        scanf("%d", &selected->semester);
    }
    if (updateChoice == 5 || updateChoice == 7) {
        printf("\tEnter New Available Copies: ");
        scanf("%d", &selected->available);
    }
    if (updateChoice == 6 || updateChoice == 7) {
        printf("\tEnter New Shelf Row: ");
        scanf("%d", &selected->shelfRow);
        printf("\tEnter New Shelf Column: ");
        scanf("%d", &selected->shelfCol);
    }

    saveBooks();

    printf("\n\t\033[1;32mBook details updated successfully!\033[0m\n");
    printf("\n\tPress any key to go back...");
    _getch();
}

void findMedicine() {
    char query[50];
    int found = 0;
    Medicine matchingMedicines[MAX_MEDICINES];
    int count = 0;

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|      FIND MEDICINE                       |\n");
    printf("\t============================================\n");

    printf("\n\tEnter Medicine Name or Company: ");
    scanf(" %[^\n]", query);

    for (int i = 0; i < numMedicines; i++) {
        if (strcasestr(medicines[i].name, query) || strcasestr(medicines[i].company, query)) {
            matchingMedicines[count++] = medicines[i];
            found = 1;
        }
    }

    clearScreen();
    printf("\n\t=================================================================\n");
    printf("\t|       Search Results for \"%s\"                             |\n", query);
    printf("\t=================================================================\n");

    if (!found) {
        printf("\n\t\033[1;31mNo medicines found for your query.\033[0m\n");
        printf("\n\tPress any key to go back...");
        _getch();
        return;
    }

    displayMedicineList(matchingMedicines, count);

    printf("\n\tPress any key to go back...");
    _getch();
}

void verifyMedicineLocation() {
    srand(time(NULL));
    int numMedicinesToVerify = 3;
    int verifiedMedicines[MAX_MEDICINES] = {0};
    int count = 0;

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|      VERIFY MEDICINE LOCATION           |\n");
    printf("\t============================================\n");

    while (count < numMedicinesToVerify) {
        int index = rand() % numMedicines;
        if (verifiedMedicines[index]) continue;

        verifiedMedicines[index] = 1;
        count++;

        printf("\n\tMedicine: %s\n", medicines[index].name);
        printf("\tExpected Location: Row %d, Column %d\n", medicines[index].shelfRow, medicines[index].shelfCol);

        printf("\tIs the medicine in the correct location? (1 = YES, 0 = NO) or '0' to go back: ");
        int confirmation;
        scanf("%d", &confirmation);

        if (confirmation == 0) return;

        if (confirmation == 1) {
            printf("\n\t\033[1;32mMedicine is in the correct location!\033[0m\n");
        } else {
            printf("\n\t\033[1;31mMedicine is misplaced! Expected: Row %d, Column %d\033[0m\n", medicines[index].shelfRow, medicines[index].shelfCol);
        }
    }

    printf("\n\tVerification complete! Press any key to go back...");
    _getch();
}

void expiredMedicines() {
    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|      EXPIRED MEDICINES                  |\n");
    printf("\t============================================\n");

    int expiredFound = 0;
    for (int i = 0; i < numMedicines; i++) {
        if (isExpired(medicines[i].expiryDate)) {
            expiredFound = 1;
            printf("\n\tMedicine: %s\n", medicines[i].name);
            printf("\tCompany: %s\n", medicines[i].company);
            printf("\tExpiry Date: %s\n", medicines[i].expiryDate);
            printf("\tShelf Location: Row %d, Column %d\n", medicines[i].shelfRow, medicines[i].shelfCol);
            printf("\t\033[1;31mThis medicine is expired!\033[0m\n");

            printf("\t\033[1;33mLighting up Red LED at Row %d, Column %d\033[0m\n", medicines[i].shelfRow, medicines[i].shelfCol);
        }
    }

    if (!expiredFound) {
        printf("\n\t\033[1;32mNo expired medicines found.\033[0m\n");
    }

    printf("\n\t1. Press 'C' to clear all expired medicines\n");
    printf("\t2. Press 'R' to refresh the list\n");
    printf("\t3. Press any other key to go back...");

    char choice = _getch();

    if (choice == 'C' || choice == 'c') {
        for (int i = 0; i < numMedicines; i++) {
            if (isExpired(medicines[i].expiryDate)) {
                for (int j = i; j < numMedicines - 1; j++) {
                    medicines[j] = medicines[j + 1];
                }
                numMedicines--;
                i--;
            }
        }
        saveMedicines();
        printf("\n\t\033[1;32mExpired medicines cleared successfully!\033[0m\n");
    } else if (choice == 'R' || choice == 'r') {
        expiredMedicines();
        return;
    }

    printf("\n\tPress any key to go back...");
    _getch();
}

void addMedicine() {
    if (numMedicines >= MAX_MEDICINES) {
        printf("\n\t\033[1;31mCannot add more medicines. Maximum limit reached.\033[0m\n");
        return;
    }

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          ADD A NEW MEDICINE              |\n");
    printf("\t============================================\n");

    Medicine newMedicine;

    printf("\n\tEnter Medicine Name (or 'c' to cancel): ");
    scanf(" %[^\n]", newMedicine.name);
    if (strcmp(newMedicine.name, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Company (or 'c' to cancel): ");
    scanf(" %[^\n]", newMedicine.company);
    if (strcmp(newMedicine.company, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Stock (or '0' to go back): ");
    if (scanf("%d", &newMedicine.stock) != 1 || newMedicine.stock == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Expiry Date (DD/MM/YYYY) (or 'c' to cancel): ");
    scanf("%s", newMedicine.expiryDate);
    if (strcmp(newMedicine.expiryDate, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Shelf Row (or '0' to go back): ");
    if (scanf("%d", &newMedicine.shelfRow) != 1 || newMedicine.shelfRow == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Shelf Column (or '0' to go back): ");
    if (scanf("%d", &newMedicine.shelfCol) != 1 || newMedicine.shelfCol == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Price (or '0' to go back): ");
    if (scanf("%f", &newMedicine.price) != 1 || newMedicine.price == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    medicines[numMedicines] = newMedicine;
    numMedicines++;

    saveMedicines();

    printf("\n\t\033[1;32mMedicine added successfully!\033[0m\n");
    printf("\n\tPress any key to go back...");
    _getch();
}

void removeMedicine() {
    char name[50], company[30];

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          REMOVE A MEDICINE               |\n");
    printf("\t============================================\n");

    printf("\n\tEnter Medicine Name (or 'c' to cancel): ");
    scanf(" %[^\n]", name);
    if (strcmp(name, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    printf("\tEnter Company (or 'c' to cancel): ");
    scanf(" %[^\n]", company);
    if (strcmp(company, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < numMedicines; i++) {
        if (strcmp(medicines[i].name, name) == 0 && strcmp(medicines[i].company, company) == 0) {
            printf("\n\tRemoving medicine: %s\n", medicines[i].name);

            for (int j = i; j < numMedicines - 1; j++) {
                medicines[j] = medicines[j + 1];
            }
            numMedicines--;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\n\t\033[1;31mMedicine not found.\033[0m\n");
    } else {
        saveMedicines();
        printf("\n\t\033[1;32mMedicine removed successfully!\033[0m\n");
    }

    printf("\n\tPress any key to go back...");
    _getch();
}

void updateMedicine() {
    char name[50], company[30];

    clearScreen();
    printf("\n\t============================================\n");
    printf("\t|          UPDATE MEDICINE                |\n");
    printf("\t============================================\n");

    printf("\n\tEnter Medicine Name (or Enter 'c' to Cancel): ");
    scanf(" %[^\n]", name);
    if (strcmp(name, "c") == 0) {
        printf("\n\t\033[1;31mOperation canceled.\033[0m\n");
        return;
    }
    printf("\tEnter Company: ");
    scanf(" %[^\n]", company);

    int found = 0;
    for (int i = 0; i < numMedicines; i++) {
        if (strcmp(medicines[i].name, name) == 0 && strcmp(medicines[i].company, company) == 0) {
            printf("\n\tCurrent Stock: %d, Price: $%.2f\n", medicines[i].stock, medicines[i].price);
            printf("\tEnter New Stock: ");
            scanf("%d", &medicines[i].stock);
            printf("\tEnter New Price: ");
            scanf("%f", &medicines[i].price);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\n\t\033[1;31mMedicine not found. Would you like to add it? (1 = Yes, 0 = No): \033[0m");
        int choice;
        scanf("%d", &choice);
        if (choice == 1) {
            addMedicine();
        }
    } else {
        saveMedicines();
        printf("\n\t\033[1;32mMedicine updated successfully!\033[0m\n");
    }

    printf("\n\tPress any key to go back...");
    _getch();
}


void librarySystem() {
    const char *options[] = {"Find Book", "Verify Book Location", "Check Overdue Books", "Return Book", "Add Book", "Remove Book", "Update Book Details", "View Logs", "Back to Main Menu"};
    int choice;
    while (1) {
        choice = menuSelection(options, 9);
        if (choice == 0) findBooks();
        else if (choice == 1) verifyBookLocation();
        else if (choice == 2) checkOverdueBooks();
        else if (choice == 3) returnBook();
        else if (choice == 4) addBook();
        else if (choice == 5) removeBook();
        else if (choice == 6) updateBook();
        else if (choice == 7) displayBookLogs();
        else if (choice == 8) return;
    }
}

void pharmacySystem() {
    const char *options[] = {"Find Medicine", "Verify Location", "Expired Medicines", "Add Medicine", "Remove Medicine", "Update Medicine", "Back to Main Menu"};
    int choice;
    while (1) {
        choice = menuSelection(options, 7);
        if (choice == 0) findMedicine();
        else if (choice == 1) verifyMedicineLocation();
        else if (choice == 2) expiredMedicines();
        else if (choice == 3) addMedicine();
        else if (choice == 4) removeMedicine();
        else if (choice == 5) updateMedicine();
        else if (choice == 6) return;
    }
}

void mainMenu() {
    const char *mainOptions[] = {"Library", "Pharmacy", "Grocery Shop", "Electronics Store", "Mobile Store", "Clothing Store", "Shoe Store", "Exit"};
    int choice;
    while (1) {
        choice = menuSelection(mainOptions, 8);
        if (choice == 0) librarySystem();
        else if (choice == 1) pharmacySystem();
        else if (choice == 7) exitProgram();
    }
}

int isExpired(const char expiryDate[]) {
    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    char currentDate[20];
    strftime(currentDate, 20, "%d/%m/%Y", current);

    if (strcmp(expiryDate, currentDate) < 0) {
        return 1; // Expired
    }
    return 0; // Not expired
}

void exitProgram() {
    saveBooks();      // Save books to file
    saveMedicines();  // Save medicines to file
    saveBookLogs();   // Save logs to file
    printf("Data saved successfully. Exiting...\n");
    exit(0);          // Exit the program
}
