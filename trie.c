#include "trie.h"


trie* init_trie_node(){
    trie *node = (trie*)malloc(sizeof(trie));
    if(node){
        node -> word = L'\0';
        node -> word_end = 0;
        node -> definition = NULL;
        for(int i = 0; i < 26; i++){
            node -> children[i] = NULL;
        }
    }
    return node;
}


void insert_to_file(char *data, char *definition) {
    char *filename = "dictionary.txt";
    FILE *file = fopen(filename, "a");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return;
    }

    fprintf(file, "%s:%s\n", data, definition);
    fclose(file);
}


void insert(trie *head, char *data, char *definition){
    trie *current = head;
    char *data_temp = data;
    while (*data_temp){
        if(current -> children[*data_temp - 'a'] == NULL){
            current -> children[*data_temp - 'a'] = (trie*)malloc(sizeof(trie));
            memset(current -> children[*data_temp - 'a'], 0, sizeof(trie));
            current -> children[*data_temp - 'a'] -> word = *data_temp;
        }
        current = current->children[*data_temp - 'a'];
        data_temp++;
    }
    current -> word_end = 1;
    current -> definition = (char *)malloc(strlen(definition) + 1);
    strcpy(current -> definition, definition);
}


char *search(trie *head, char *data){
    trie *current = head;
    while(*data){
        if(current -> children[*data - 'a'] == NULL){
            return NULL;
        }
        current = current -> children[*data - 'a'];
        data++;
    }
    return current -> word_end ? current -> definition : NULL;
}


void load_dictionary(trie *head, char *filename){
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return;
    }

    char line[2048];
    while (fgets(line, sizeof(line) / sizeof(wchar_t), file)){
        char *colon_pos = strchr(line, ':');
        if (colon_pos){
            *colon_pos = '\0';
            char *word = line;
            char *definition = colon_pos + 1;

            int len = strlen(definition);
            if (len > 0 && definition[len - 1] == L'\n'){
                definition[len - 1] = L'\0';
            }

            insert(head, word, definition);
        }
    }

    fclose(file);
}


wchar_t* convert_to_wchar_t(char *original){
    int length = strlen(original);
    wchar_t* wide_string = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    if (wide_string == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    int converted_length = mbstowcs(wide_string, original, length + 1);
    if (converted_length == (int)-1){
        fprintf(stderr, "Conversion failed\n");
        free(wide_string);
        return NULL;
    }

    return wide_string;
}


void set_utf8(){
    setlocale(LC_ALL, "en_US.UTF-8");
    _setmode(_fileno(stdout), _O_U16TEXT);
}


void set_normal(){
    setlocale(LC_ALL, NULL);
    _setmode(_fileno(stdout), _O_TEXT);
}


void print_definition(char *definition){
    if (definition){
        set_utf8();
        wchar_t *converted = convert_to_wchar_t(definition);
        if (converted){
            wprintf(L"%ls\n", converted);
            free(converted);
            set_normal();
        }else{
            set_normal();
            printf("Conversion failed\n");
        }
    } else{
        printf("Definition not found\n");
    }
}


int contain_child(trie *node){
    for (int i = 0; i < 26; i++) {
        if (node->children[i] != NULL) {
            return 1;
        }
    }
    return 0;
}


void delete_from_file(char *filename, char *data){
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file for reading");
        return;
    }

    // Temporary file to write the new content
    FILE *temp_file = fopen("temp.txt", "w");
    if (!temp_file) {
        perror("Could not open temporary file for writing");
        fclose(file);
        return;
    }

    char line[2048];
    int word_length = strlen(data);
    int found = 0;

    while (fgets(line, sizeof(line), file)){
        // Check if the line starts with the word followed by a colon
        if (strncmp(line, data, word_length) == 0 && line[word_length] == ':') {
            found = 1;
            continue; // Skip writing this line to the temporary file
        }
        fputs(line, temp_file); // Write the line to the temporary file
    }

    fclose(file);
    fclose(temp_file);

    if (found){
        remove(filename);
        rename("temp.txt", filename);
    } else {
        remove("temp.txt");
    }
}


void delete(trie *head, char *data){
    delete_from_file("dictionary.txt", data);
    trie *current = head;
    char *data_tep = data;
    int data_len = strlen(data_tep), level = 0;
    trie **stack = (trie **)malloc((data_len + 1) * sizeof(trie *));
    int *indices = (int *)malloc((data_len + 1) * sizeof(int));

    while(*data_tep){
        if (current->children[*data_tep - 'a'] == NULL) {
            printf("\"%s\" does not exist.\n", data);
            free(stack);
            free(indices);
            return;
        }

        stack[level] = current;
        indices[level] = *data_tep - 'a';
        current = current -> children[*data_tep - 'a'];
        data_tep++;
        level++;
    }
    if (current -> word_end == 1){
        current -> word_end = 0;
    } else{
        printf("\"%s\" does not exist.\n", data);
        free(stack);
        free(indices);
        return;
    }

    while (level > 0 && !contain_child(current) && current -> word_end == 0){
        free(current);
        level--;
        stack[level] -> children[indices[level]] = NULL;
        current = stack[level];
    }
    free(stack);
    free(indices);
}


void suggest_words(trie *node, char *prefix){
    if (node == NULL) return;
    if (node -> word_end) {
        suggestions[suggestion_count] = (char *)malloc((strlen(prefix) + 1) * sizeof(char));
        strcpy(suggestions[suggestion_count], prefix);
        suggestion_count++;
        if (suggestion_count >= MAX_SUGGESTIONS) return;
    }

    for (int i = 0; i < 26; i++){
        if (node -> children[i]){
            char next_char = 'a' + i;
            char new_prefix[MAX_WORD_LENGTH];
            snprintf(new_prefix, sizeof(new_prefix), "%s%c", prefix, next_char);
            suggest_words(node->children[i], new_prefix);
        }
    }
}

void autocomplete(trie *root, char *prefix){
    trie *current = root;
    for (int i = 0; prefix[i] != '\0'; i++){
        int index = prefix[i] - 'a';
        if (!current -> children[index]) {
            return;
        }
        current = current -> children[index];
    }
    suggestion_count = 0;
    suggest_words(current, prefix);
}

char **get_suggestions(){
    suggestions[suggestion_count] = NULL; // Terminate suggestions array with NULL
    return suggestions;
}

int get_suggestion_count(){
    return suggestion_count;
}
