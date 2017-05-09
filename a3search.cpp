//
//  a3search.cpp
//  a3search
//
//  Created by 张云翮 on 06/05/2017.
//  Copyright © 2017 Zhang Yunhe. All rights reserved.
//

#include "a3search.h"
int k = 2003256;
char read[2003256];
int main(int argc, const char * argv[]) {
    build_index(argv[1]);
    return 0;
}

void build_index(const char * argument) {
    char name[300];
    char word[257];
    DIR *pDIR;
    FILE *file_in, *index_read, *index_write, *total_word;
    struct dirent *entry;
    int i, last_number;
    unsigned long location;
    static const char* const index[] = {"index1", "index2"};
    map<string, int> word_list;
    typedef map<string, int>::const_iterator MapIterator;
    int file_number = 0;
    string index_word, posting_list, previous_word;
    /*
        read files in folder
     */
    if((pDIR = opendir(argument))) {
        entry = readdir(pDIR);
        if (entry == NULL) {
            return;
        }
        while(1){
            /*
                read each file except . and ..
             */
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                strcpy(name, argument);
                strcat(name, "/");
                strcat(name, entry->d_name);
                file_in = fopen(name, "r");
                file_number++;
                while (fscanf(file_in, " %256s", word) == 1) {
                    
                    for (i = 0; i < strlen(word); i++) {
                        if (word[i] >= 'A' and word[i] <= 'Z') {
                            word[i] = tolower(word[i]);
                        }
                        else if (word[i] < 'a' or word[i] > 'z') {
                            word[i] = '\0';
                        }
                    }
                    if (strlen(word) < 3) continue;
                    if (word_list.find(word) == word_list.end()) {
                        word_list[word] = 1;
                    }
                    else {
                        word_list[word]++;
                    }
                }
                fclose(file_in);
            }
            else {
                entry = readdir(pDIR);
                if (entry == NULL) {
                    return;
                }
                continue;
            }
            entry = readdir(pDIR);
            /*
                if it is the last file
             */
            if (entry == NULL) {
                /*
                    if it is the first file(only one file)
                 */
                if (file_number == 1) {
                    index_write = fopen(index[file_number%2], "w");
                    total_word = fopen("word", "w");
                    i = 1;
                    for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                        if (iter->second != 0) {
                            fprintf(index_write, "%d-%d\n", file_number,iter->second);
                            fprintf(total_word, "%d%s", i, iter->first.c_str());
                            i++;
                            word_list[iter->first] = 0;
                        }
                    }
                    fclose(index_write);
                    fclose(total_word);
                }
                /*
                    at least two files
                 */
                else {
                    index_read = fopen(index[(file_number-1)%2], "r");
                    index_write = fopen(index[file_number%2], "w");
                    total_word = fopen("word", "w");
                    fgets(read, k, index_read);
                    index_word = read;
                    index_word = index_word.substr(0, index_word.find(" "));
                    i = 1;
                    for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                        if (iter->first == index_word) {
                            
                            fprintf(total_word, "%d%s", i, index_word.c_str());
                            if(iter->second == 0) {
                                posting_list = read;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr( location, strlen(read) - location);
                                fprintf(index_write, "%s", posting_list.c_str());
                            }
                            else {
                                read[strlen(read) - 1] = '\0';
                                posting_list = read;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr(location, 5);
                                posting_list = posting_list.substr(0, posting_list.find("-"));
                                last_number = atoi(posting_list.c_str());
                                last_number = file_number - last_number;
                                posting_list = read;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr(location, strlen(read) - location);
                                fprintf(index_write, "%s %d-%d\n", posting_list.c_str(), last_number, iter->second);
                            }
                            fgets(read, k, index_read);
                            index_word = read;
                            index_word = index_word.substr(0, index_word.find(" "));
                        }
                        else {
                            fprintf(total_word, "%d%s", i, iter->first.c_str());
                            fprintf(index_write, "%d-%d\n", file_number, iter->second);
                        }
                        word_list[iter->first] = 0;
                        i++;
                    }
                    fclose(index_write);
                    fclose(index_read);
                    fclose(total_word);
                }
                break;
            }
            /*
                if it is not the last file
             */
            
            // if it is the first file
            else if (file_number == 1) {
                index_write = fopen(index[file_number%2], "w");
                for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                    if (iter->second != 0) {
                        fprintf(index_write, "%s %d-%d\n", iter->first.c_str(), file_number,iter->second);
                        word_list[iter->first] = 0;
                    }
                }
                fclose(index_write);
            }
            
            // if it is not the first file
            else {
                index_read = fopen(index[(file_number-1)%2], "r");
                index_write = fopen(index[file_number%2], "w");
                fgets(read, k, index_read);
                index_word = read;
                index_word = index_word.substr(0, index_word.find(" "));
                for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                    if (iter->first == index_word) {
                        if(iter->second == 0) {
                            fprintf(index_write, "%s", read);
                        }
                        else {
                            read[strlen(read) - 1] = '\0';
                            posting_list = read;
                            location = posting_list.find(" ") + 1;
                            posting_list = posting_list.substr(location, 5);
                            posting_list = posting_list.substr(0, posting_list.find("-"));
                            last_number = atoi(posting_list.c_str());
                            last_number = file_number - last_number;
                            fprintf(index_write, "%s %d-%d\n", read, last_number,iter->second);
                        }
                        fgets(read, k, index_read);
                        index_word = read;
                        index_word = index_word.substr(0, index_word.find(" "));
                    }
                    else {
                        fprintf(index_write, "%s %d-%d\n", iter->first.c_str(), file_number,iter->second);
                    }
                    word_list[iter->first] = 0;
                }
                fclose(index_write);
                fclose(index_read);
            }
        }
        remove(index[(file_number - 1)%2]);
        closedir(pDIR);
    }
}
