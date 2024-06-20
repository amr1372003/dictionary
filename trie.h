#ifndef _TRIE_H_
#define _TRIE_H_

    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <wchar.h>
    #include <locale.h>
    #include <io.h>
    #include <fcntl.h>

    #define MAX_SUGGESTIONS  6
    #define MAX_WORD_LENGTH  100

    char *suggestions[MAX_SUGGESTIONS];
    int suggestion_count = 0;

    typedef struct Trie{
        char word;
        char word_end;
        char *definition;
        struct Trie *children[26];
    }trie;
    
    trie* init_trie_node();
    void insert(trie *head, char *data, char *definition);
    char *search(trie *head, char *data);
    void load_dictionary(trie *head, char *filename);
    wchar_t* convert_to_wchar_t(char *original);
    int contain_child(trie *node);
    void delete(trie *head, char *data);
    void delete_from_file(char *filename, char *data);
    void set_utf8();
    void set_normal();
    void print_definition(char *definition);
    void suggest_words(trie *node, char *prefix);
    void autocomplete(trie *root, char *prefix);
    void insert_to_file(char *data, char *definition);
    char **get_suggestions();
    int get_suggestion_count();
#endif