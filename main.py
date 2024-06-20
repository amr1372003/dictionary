import ctypes
import requests
from tkinter import *
from tkinter import messagebox


class TrieNode(ctypes.Structure):
    pass
TrieNode._fields_ = [
    ('children', ctypes.POINTER(ctypes.POINTER(TrieNode)) * 26),
    ('word', ctypes.c_wchar),
    ('word_end', ctypes.c_int),
    ('definition', ctypes.c_char_p)
]

libtrie = ctypes.CDLL('./trie.dll')

# Function prototypes
init_trie_node = libtrie.init_trie_node
init_trie_node.restype = ctypes.POINTER(TrieNode)

insert = libtrie.insert
insert.argtypes = [ctypes.POINTER(TrieNode), ctypes.c_char_p, ctypes.c_char_p]

insert_to_file = libtrie.insert_to_file
insert_to_file.argtypes = [ctypes.c_char_p, ctypes.c_char_p]

search = libtrie.search
search.argtypes = [ctypes.POINTER(TrieNode), ctypes.c_char_p]
search.restype = ctypes.c_char_p

load_dictionary = libtrie.load_dictionary
load_dictionary.argtypes = [ctypes.POINTER(TrieNode), ctypes.c_char_p]

delete = libtrie.delete
delete.argtypes = [ctypes.POINTER(TrieNode), ctypes.c_char_p]

suggest_words = libtrie.suggest_words
suggest_words.argtypes = [ctypes.POINTER(TrieNode), ctypes.c_char_p]

autocomplete = libtrie.autocomplete
autocomplete.argtypes = [ctypes.POINTER(TrieNode), ctypes.c_char_p]

get_suggestions = libtrie.get_suggestions
get_suggestions.restype = ctypes.POINTER(ctypes.c_char_p)

get_suggestion_count = libtrie.get_suggestion_count
get_suggestion_count.restype = ctypes.c_int

print_definition = libtrie.print_definition
print_definition.argtypes = [ctypes.c_char_p]


root = init_trie_node()
load_dictionary(root, b"dictionary.txt")


def search_btn():
    word = search_box.get().strip().lower()
    word_encoded = word.encode('utf-8')
    definition = search(root, word_encoded)
    
    text_out = ""
    if definition:
        text_out = f"Definition for {word}: {definition.decode('utf-8')}"
    else:
        try:
            response = requests.get(f"https://api.dictionaryapi.dev/api/v2/entries/en/{word}")
            if response.status_code == 200:
                definitions = response.json()
                api_definition = definitions[0]['meanings'][0]['definitions'][0]['definition']
                text_out = f"Definition for {word}: {api_definition}"
                api_definition_encoded = api_definition.encode('utf-8')
                insert(root, word_encoded, api_definition_encoded)
                insert_to_file(word_encoded, api_definition_encoded)
            else:
                text_out = "Definition not found."
        except Exception as e:
            text_out = f"Error fetching definition from API: {e}"
    
    output_label.config(text = text_out)


def refresh_suggestions():
    word = search_box.get().strip().lower()
    if word:
        word_encoded = word.encode('utf-8')
        autocomplete(root, word_encoded)
        
        count = get_suggestion_count()
        suggestions = get_suggestions()
        
        search_list.delete(0, END)
        
        for i in range(count):
            suggestion = suggestions[i]
            if suggestion:
                search_list.insert(END, suggestion.decode('utf-8'))
            else:
                search_list.delete(0, END)
    else:
        search_list.delete(0, END)


def auto_complete(event):
    refresh_suggestions()


def fill_search_box(event):
    search_box.delete(0, END)
    search_box.insert(0, search_list.get(ACTIVE))
    search_btn()


def delete_word_from_trie_and_file(word_encoded):
    delete(root, word_encoded)
    refresh_suggestions()


def login():
    word = search_box.get().strip().lower()
    word_encoded = word.encode('utf-8')
    admin = "admin"
    admin_pass = "123"
    if admin_name.get() == admin and admin_password.get() == admin_pass:
        delete_word_from_trie_and_file(word_encoded)
        messagebox.showinfo('admin', 'Word deleted')
        del_win.destroy()
    else:
        messagebox.showinfo('admin', 'Wrong admin credentials')



def delete_window():
    global admin_name, admin_password, del_win

    del_win = Toplevel()
    del_win.title("Delete Window")

    frame = Frame(del_win)
    
    app_width = 300
    app_height = 300
    screen_width = del_win.winfo_screenwidth()
    screen_height = del_win.winfo_screenheight()
    x = (screen_width / 2) - (app_width / 2)
    y = (screen_height / 2) - (app_height / 2)
    del_win.geometry(f'{app_width}x{app_height}+{int(x)}+{int(y)}')

    admin_label = Label(frame, text = "Adim Name: ")
    admin_label.grid(row = 1, column = 0, padx = 12, pady = 15)

    pass_label = Label(frame, text = "Adim password: ")
    pass_label.grid(row = 2, column = 0, padx = 12, pady = 15)

    admin_name = Entry(frame)
    admin_name.grid(row = 1, column = 1, pady = 15)
    admin_password = Entry(frame, show = "*")
    admin_password.grid(row = 2, column = 1, pady = 15)

    apply_btn = Button(frame, text = "Delete", command = login)
    apply_btn.grid(row = 3, column = 0, columnspan = 2, pady = 15)

    frame.pack()



if __name__ == "__main__":
    # GUI
    window = Tk()
    window.title("Dictionary Search")
    frame = Frame(window)
    
    app_width = 500
    app_height = 300
    screen_width = window.winfo_screenwidth()
    screen_height = window.winfo_screenheight()
    x = (screen_width / 2) - (app_width / 2)
    y = (screen_height / 2) - (app_height / 2)
    window.geometry(f'{app_width}x{app_height}+{int(x)}+{int(y)}')

    search_box = Entry(frame, width = 50, borderwidth = 5)
    search_box.insert(0, "Enter a word: ")
    search_box.bind('<KeyRelease>', auto_complete)

    search_button = Button(frame, text = "Search", command = search_btn)

    search_list = Listbox(frame, width = 50, height = 5)
    search_list.bind("<<ListboxSelect>>", fill_search_box)

    output_label = Label(frame, text = "", wraplength = 400, justify = "left")

    del_button = Button(frame, text = "delete word", command = delete_window)

    search_box.grid(row = 1, column = 0, pady = 15, columnspan = 3, padx = 5)
    search_button.grid(row = 1, column = 3, pady = 15, padx = 5)
    search_list.grid(row = 2, column = 0, columnspan = 4, sticky = "ew")
    output_label.grid(row = 3, column = 0, columnspan = 4, pady = 15)
    del_button.grid(row = 4, column = 0, columnspan = 4, pady = 20)

    frame.pack(expand=True)
    
    window.mainloop()

