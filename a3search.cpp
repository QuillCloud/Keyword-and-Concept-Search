//
//  a3search.cpp
//  a3search
//
//  Created by 张云翮 on 06/05/2017.
//  Copyright © 2017 Zhang Yunhe. All rights reserved.
//

#include "a3search.h"
#include "porter2_stemmer.h"



int main(int argc, const char * argv[]) {
    if (argc < 3) {
        return 0;
    }
    DIR *indexfile;
    if((indexfile = opendir(argv[2]))) {
        cout<<"indexfile"<<endl;
    }
    else {
        build_index(argv[1], argv[2]);
    }
    return 0;
}
void build_index(const char * argument1, const char * argument2) {
    int kcloud = 2003256;
    char readfile[kcloud];
    char name[300];
    const char* stopword[] =
    {   "about", "above", "after", "again", "against", "all", "and", "ani", "are", "becaus", "been", "befor", "below", "between", "both", "but", "can", "cannot", "could", "did", "doe", "down", "each", "few",
        "for", "from", "had", "has", "have", "him", "his", "how","her", "into", "most", "more", "not", "onli", "veri",
        "too", "they", "the", "such", "should", "some", "such", "than", "that", "were", "their", "then", "there",
        "those", "what", "which", "while", "whi", "where", "when", "who", "whom", "you", "would", "your", "with",
        "these", "she", "other", "our", "it", "go", "be", "do", "ad", "a", "in", "up", "aw"};
    char * word;
    DIR *pDIR;
    FILE *file_in, *index_read, *index_write, *total_word, *write_filename;
    string file_name[2000];
    struct dirent *entry;
    int i, last_number;
    unsigned long location;
    static const char* const index[] = {"index1", "index2"};
    map<string, int> word_list;
    typedef map<string, int>::const_iterator MapIterator;
    int file_number = 0;
    string index_word, posting_list, previous_word;
    char delim[] = "-; ,<>1234567890#*?.[]\\/$%^&()!@+=_~`{}|\"";
    /*
        read files in folder
     */
    if((pDIR = opendir(argument1))) {
        entry = readdir(pDIR);
        if (entry == NULL) {
            return;
        }
        while(1){
            /*
                read each file except . and ..
             */
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".DS_Store") != 0) {
                strcpy(name, argument1);
                strcat(name, "/");
                strcat(name, entry->d_name);
                file_in = fopen(name, "r");
                file_name[file_number] = entry->d_name;
                file_number++;
                while(fgets(readfile, kcloud, file_in) != NULL) {
                    word = strtok(readfile, delim);
                    while(word != NULL) {
                        if (strlen(word) < 3) {
                            word = strtok(NULL, delim);
                            continue;
                        }
                        index_word = word;
                        for (i = (int)strlen(index_word.c_str()); i >= 0; i--) {
                            if (index_word[i] >= 'A' and index_word[i] <= 'Z') {
                                index_word[i] = tolower(index_word[i]);
                            }
                            else if (index_word[i] < 'a' or index_word[i] > 'z') {
                                index_word.erase(i, 1);
                            }
                        }
                        if (strlen(index_word.c_str()) < 3) {
                            word = strtok(NULL, delim);
                            continue;
                        }
                        Porter2Stemmer::stem(index_word);
                        if ((find(begin(stopword), end(stopword), index_word) != end(stopword))) {
                            word = strtok(NULL, delim);
                            continue;
                        }
                        if (word_list.find(index_word) == word_list.end()) {
                            word_list[index_word] = 1;
                        }
                        else {
                            word_list[index_word]++;
                        }
                        word = strtok(NULL, delim);
                        
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
                strcpy(name, "mkdir ");
                strcat(name, argument2);
                system(name);
                strcpy(name, argument2);
                strcat(name, "/files");
                write_filename = fopen(name, "w");
                for (i = 0; i < file_number; i++) {
                    fprintf(write_filename, "%s\n", file_name[i].c_str());
                }
                /*
                    if it is also the first file(means only one file)
                 */
                if (file_number == 1) {
                    strcpy(name, argument2);
                    strcat(name, "/index");
                    index_write = fopen(name, "w");
                    strcpy(name, argument2);
                    strcat(name, "/word");
                    total_word = fopen(name, "w");
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
                    strcpy(name, argument2);
                    strcat(name, "/index");
                    index_write = fopen(name, "w");
                    strcpy(name, argument2);
                    strcat(name, "/word");
                    total_word = fopen(name, "w");
                    fgets(readfile, kcloud, index_read);
                    index_word = readfile;
                    index_word = index_word.substr(0, index_word.find(" "));
                    i = 1;
                    int j = 1;
                    for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                        if (iter->first == index_word) {
                            fprintf(total_word, "%d%s", i, index_word.c_str());
                            if(iter->second == 0) {
                                posting_list = readfile;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr( location, strlen(readfile) - location);
                                fprintf(index_write, "%s", posting_list.c_str());
                            }
                            else {
                                readfile[strlen(readfile) - 1] = '\0';
                                posting_list = readfile;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr(location, 5);
                                posting_list = posting_list.substr(0, posting_list.find("-"));
                                last_number = atoi(posting_list.c_str());
                                last_number = file_number - last_number;
                                posting_list = readfile;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr(location, strlen(readfile) - location);
                                fprintf(index_write, "%s %d-%d\n", posting_list.c_str(), last_number, iter->second);
                            }
                            fgets(readfile, kcloud, index_read);
                            index_word = readfile;
                            index_word = index_word.substr(0, index_word.find(" "));
                            j++;
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
                fgets(readfile, kcloud, index_read);
                index_word = readfile;
                index_word = index_word.substr(0, index_word.find(" "));
                for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                    if (iter->first == index_word) {
                        if(iter->second == 0) {
                            fprintf(index_write, "%s", readfile);
                        }
                        else {
                            readfile[strlen(readfile) - 1] = '\0';
                            posting_list = readfile;
                            location = posting_list.find(" ") + 1;
                            posting_list = posting_list.substr(location, 5);
                            posting_list = posting_list.substr(0, posting_list.find("-"));
                            last_number = atoi(posting_list.c_str());
                            last_number = file_number - last_number;
                            fprintf(index_write, "%s %d-%d\n", readfile, last_number,iter->second);
                        }
                        fgets(readfile, kcloud, index_read);
                        index_word = readfile;
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
        remove(index[0]);
        remove(index[1]);
        closedir(pDIR);
    }
}
