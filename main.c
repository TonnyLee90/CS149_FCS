#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// ANSI escape codes for colors and formatting in the terminal
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[1;36m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define RED     "\033[1;31m"
#define BLUE    "\033[1;34m"

// Print the main menu
void print_menu(void)
{
    printf("\n");
    printf(CYAN "╔══════════════════════════════════════╗\n" RESET);
    printf(CYAN "║     " BOLD "FILE MANAGEMENT SYSTEM" RESET CYAN "          ║\n" RESET);
    printf(CYAN "╠══════════════════════════════════════╣\n" RESET);
    printf(CYAN "║" RESET "  1. Create a file                    " CYAN "║\n" RESET);
    printf(CYAN "║" RESET "  2. Open / Edit a file  (nano)       " CYAN "║\n" RESET);
    printf(CYAN "║" RESET "  3. Close / Return to menu           " CYAN "║\n" RESET);
    printf(CYAN "║" RESET "  4. Search for a file                " CYAN "║\n" RESET);
    printf(CYAN "║" RESET "  5. List all files                   " CYAN "║\n" RESET);
    printf(CYAN "║" RESET "  6. Delete a file                    " CYAN "║\n" RESET);
    printf(CYAN "║" RESET "  0. Exit                             " CYAN "║\n" RESET);
    printf(CYAN "╚══════════════════════════════════════╝\n" RESET);
    printf("Enter choice: ");
}

// Create a file
void create_file(void)
{
    char filename[256];
    printf(YELLOW "\n[CREATE FILE]\n" RESET);
    printf("Enter file name: ");
    scanf("%255s", filename); // allowing up to 255 characters
    getchar(); // reads that leftover \n and removes it from the buffer.

    // Warn if file already exists
    if (access(filename, F_OK) == 0) {
        printf(YELLOW "Warning: '%s' already exists. Overwrite? (y/n): " RESET, filename);
        char ans = getchar(); 
        getchar();
        if (ans != 'y' && ans != 'Y') {
            printf("Cancelled.\n");
            return;
        }
    }
    // opening in "w" mode will create the file if it doesn't exist, or truncate it if it does
    // this is a pointer to a FILE struct, which we can use to write to the file if we wanted to
    FILE *fp = fopen(filename, "w");
    // if open fails, it returns NULL, so we check for that
    if (!fp) {
        perror(RED "Error" RESET);
        return;
    }
    fclose(fp);
    printf(GREEN "File '%s' created successfully!\n" RESET, filename);
}

// 2. Open / Edit launch nano automatically
void open_file(void)
{
    char filename[256];
    printf(YELLOW "\n[OPEN / EDIT FILE]\n" RESET);
    printf("Enter file name to open: ");
    scanf("%255s", filename);
    getchar();

    /* nano creates the file if it doesn't exist, so no pre-check needed */
    char command[512];
    //
    snprintf(command, sizeof(command), "nano %s", filename);

    printf(GREEN "Opening '%s' in nano...\n" RESET, filename);
    // executes the command in the shell, which will open nano with the specified file
    system(command);
    // nano ends when the user presses Ctrl+X, so after system returns, we can print a message
    printf(GREEN "Returned from nano. File saved.\n" RESET);
}

// 3. Close: returns to the menu 
void close_file(void)
{
    printf(YELLOW "\n[CLOSE FILE]\n" RESET);
    printf("Files opened in nano are closed when you press Ctrl+X inside nano.\n");
    printf(GREEN "Returned to main menu.\n" RESET);
}

// 4. Search for a file in the current directory
void search_file(void)
{
    char filename[256];
    printf(YELLOW "\n[SEARCH FILE]\n" RESET);
    printf("Enter file name to search: ");
    scanf("%255s", filename);
    getchar();

    printf("Searching in current directory...\n\n");

    // open current directory for reading
    // return a pointer to a DIR struct, which we can use to read entries in the directory
    // returns NULL if it fails
    DIR *dir = opendir(".");
    if (!dir) { perror("opendir"); return; }
    // // directory entry. It represents one item inside a directory
    // like a file or subfolder
    struct dirent *entry;
    int found = 0;

    // Every time readdir(dir) is called, it automatically advances an internal position pointer inside the DIR structure. 
    while ((entry = readdir(dir)) != NULL) {
        // comparing each file name (entry->d_name) with the target filename
        if (strcmp(entry->d_name, filename) == 0) {
            // if we find a match, we can get more info about the file using stat
            struct stat st;
            // Get information about the file named entry->d_name, and stores it in the st 
            // returns 0 on success and -1 on failure
            if (stat(entry->d_name, &st) == 0) {
                if(S_ISREG(st.st_mode)) {
                    printf(GREEN "File '%s' found in current directory.\n" RESET, entry->d_name);
                    printf("  Size  : %lld bytes\n", (long long)st.st_size);
                    printf("  Modified: %s", ctime(&st.st_mtime));
                } else if (S_ISDIR(st.st_mode)) {
                    printf("Found directory (skipping): %s\n", entry->d_name);
                    continue;
                }
            }
            found = 1;
            break;
        }
    }
    closedir(dir);

    if (!found) {
        printf(RED "File '%s' not found in current directory.\n" RESET, filename);
    }
}

//5. List all files in the current directory
void list_files(void)
{
    printf(YELLOW "\n[LIST FILES — current directory]\n" RESET);
    printf("%-30s %10s  %s\n", "Name", "Size(B)", "Last Modified");
    printf("%-30s %10s  %s\n", "────────────────────────────", "────────", "──────────────────────");

    DIR *dir = opendir(".");
    // if opendir fails, it returns NULL
    // print an error message
    if(!dir){
        perror("opendir"); 
        return; 
    }

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        // skip hidden entries
        if (entry->d_name[0] == '.') continue;

        struct stat st;
        if (stat(entry->d_name, &st) == 0 && S_ISREG(st.st_mode)) {
            char timebuf[64];
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", localtime(&st.st_mtime));
            printf(BLUE "%-30s" RESET " %10lld  %s\n",
                   entry->d_name, (long long)st.st_size, timebuf);
            count++;
        }
    }
    closedir(dir);
    printf("\n%d file(s) found.\n", count);
}

// 6. Delete a file
void delete_file(void)
{
    char filename[256];
    printf(YELLOW "\n[DELETE FILE]\n" RESET);
    printf("Enter file name to delete: ");
    scanf("%255s", filename);
    getchar();

    // Check if file exists before trying to delete
    if (access(filename, F_OK) != 0) {
        printf(RED "Error: '%s' does not exist.\n" RESET, filename);
        return;
    }

    printf(RED "Are you sure you want to delete '%s'? (y/n): " RESET, filename);
    char ans = getchar(); 
    getchar();
    if (ans != 'y' && ans != 'Y') { printf("Cancelled.\n"); return; }

    if (remove(filename) == 0)
        printf(GREEN "File '%s' deleted successfully.\n" RESET, filename);
    else
        perror(RED "Error deleting file" RESET);
}

int main(void)
{
    int choice;
    printf(BOLD "\nWelcome to the File Management System\n" RESET);

    while(1) {
        print_menu();
        // scanf returns the number of items successfully read, so if it doesn't return 1
        // it means the input was invalid ('abc')
        if(scanf("%d", &choice) != 1){
            while (getchar() != '\n'); // clear the input buffer
            printf(RED "Invalid input. Please enter a number.\n" RESET);
            continue;
        }
        getchar();

        switch (choice) {
            case 1: create_file(); break;
            case 2: open_file();   break;
            case 3: close_file();  break;
            case 4: search_file(); break;
            case 5: list_files();  break;
            case 6: delete_file(); break;
            case 0:
                printf(GREEN "\nExiting File Management System. Goodbye!\n\n" RESET);
                return 0;
            default:
                printf(RED "Invalid choice. Try again.\n" RESET);
        }
    }
}
