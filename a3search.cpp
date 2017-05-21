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
    if (argc < 4) {
        return 0;
    }
    int term_num;
    float cnum;
    int j;
    if (strncmp(argv[3], "-c", 2) == 0) {
        term_num = argc - 5;
        cnum = stof(argv[4]);
    }
    else {
        term_num = argc - 3;
    }
    string terms[term_num];
    for (j = 0; j < term_num; j++) {
        terms[j] = argv[argc - 1 - j];
        transform(terms[j].begin(), terms[j].end(), terms[j].begin(), ::tolower);
        Porter2Stemmer::stem(terms[j]);
    }
    DIR *indexfile;
    if(!(indexfile = opendir(argv[2]))) {
        build_index(argv[1], argv[2]);
    }
    search_terms(argv[2], terms, term_num);
    return 0;
}

/* 
    Build the index file
    Read file by file in directory
    1.Read a file and get each word in the file
    2.Do Porter2Stemmer process with each word
    3.Drop the word in stopword list
    3.Store words in map, map the word to the number of this word in file
    4.Build the posting list, write in a temporary index file
    5.Read

 */
void build_index(const char * argument1, const char * argument2) {
    int kcloud = 2003256;
    char readfile[kcloud];
    char name[300];
    //stop word list, already do Poster2Stemmer::stem process, reference: http://www.ranks.nl/stopwords
    const char* stopword[] =
    {   "a", "about", "abov", "after", "again", "against", "all","and", "ani", "are","aren",
        "be", "becaus", "been", "befor", "below", "between", "both", "but", "by", "can",
        "cannot", "could", "couldn", "did", "didn", "do", "doe", "doesn", "don", "down", "dure", "each",
        "few", "for", "from", "further", "had", "hadn", "has", "hasn", "have", "haven", "he", "her",
        "here", "herself", "him", "himself", "his", "how", "i", "if", "into", "is", "isn", "it",
        "itself", "me", "more", "most", "mustn", "my", "myself" ,"nor", "not", "off", "onc", "onli", "other",
        "ought", "our", "ourselv", "out", "over", "own", "same", "shan", "she", "should", "shouldn", "some",
        "such", "than", "that", "the", "their", "them", "themselv", "then", "there", "these", "they",
        "this", "those", "through", "too", "under", "until", "up", "veri", "was", "wasn","we", "were",
        "weren", "what", "when", "where", "which", "while", "who", "whom", "whi", "with", "won", "would", "wouldn"
        "you", "your", "yourself", "yourselv"};
    char * word;
    DIR *pDIR;
    FILE *file_in, *index_read, *index_write, *total_word, *write_filename;
    string file_name[2000];
    struct dirent *entry;
    int i, last_number;
    string temp;
    unsigned long location, seek_location;
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
                    seek_location = 0;
                    for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                        if (iter->second != 0) {
                            fprintf(total_word, "%lu-%s ", seek_location, iter->first.c_str());
                            fprintf(index_write, "%d-%d\n", file_number,iter->second);
                            seek_location += get_length(file_number) + get_length(iter->second) + 2;
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
                    seek_location = 0;
                    for (MapIterator iter = word_list.begin(); iter != word_list.end(); iter++) {
                        if (iter->first == index_word) {
                            fprintf(total_word, "%lu-%s ",seek_location, iter->first.c_str());
                            if(iter->second == 0) {
                                posting_list = readfile;
                                location = posting_list.find(" ") + 1;
                                posting_list = posting_list.substr( location, strlen(readfile) - location);
                                fprintf(index_write, "%s", posting_list.c_str());
                                seek_location += strlen(posting_list.c_str());
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
                                seek_location += strlen(posting_list.c_str()) + get_length(last_number) + get_length(iter->second) + 3;
                            }
                            fgets(readfile, kcloud, index_read);
                            index_word = readfile;
                            index_word = index_word.substr(0, index_word.find(" "));
                        }
                        else {
                            fprintf(total_word, "%lu-%s ",seek_location, iter->first.c_str());
                            fprintf(index_write, "%d-%d\n", file_number, iter->second);
                            seek_location += get_length(file_number) + get_length(iter->second) + 2;
                        }
                        word_list[iter->first] = 0;
                    }
                    fclose(index_write);
                    fclose(index_read);
                    fclose(total_word);
                    fclose(write_filename);
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

void search_terms(const char * argument2, string search_terms[], int number_of_term) {
    int k = 2003256;
    char read[k];
    FILE *read_word, *read_index, *read_files;
    char name[300];
    char word[256];
    int posting_list[5][2000] = {0};
    int occur_times[5][2000] = {0};
    int len_of_pl[5] = {0};
    int seek_location, i, j, word_count, file_count;
    char * single_of_pl;
    string get_word, index_word;
    strcpy(name, argument2);
    strcat(name, "/word");
    read_word = fopen(name, "r");
    strcpy(name, argument2);
    strcat(name, "/index");
    read_index = fopen(name, "r");
    strcpy(name, argument2);
    strcat(name, "/files");
    read_files = fopen(name, "r");
    word_count = 0;
    while(fscanf(read_word, "%s", word) != EOF) {
        get_word = word;
        seek_location = atoi(get_word.substr(0, get_word.find("-")).c_str());
        get_word = get_word.substr(get_word.find("-") + 1, strlen(word) - get_word.find("-")).c_str();
        for (i = 0; i < number_of_term; i++) {
            if (search_terms[i] == get_word.c_str()) {
                fseek(read_index, seek_location, SEEK_SET);
                fgets(read, k, read_index);
                single_of_pl = strtok(read, " ");
                file_count = 0;
                while (single_of_pl != NULL) {
                    index_word = single_of_pl;
                    posting_list[word_count][file_count] = atoi(index_word.substr(0, index_word.find("-")).c_str());
                    occur_times[word_count][file_count] = atoi(index_word.substr(index_word.find("-") + 1, strlen(index_word.c_str()) - index_word.find("-")).c_str());
                    if (file_count != 0) {
                        posting_list[word_count][file_count] += posting_list[word_count][0];
                    }
                    single_of_pl = strtok(NULL, " ");
                    file_count++;
                }
                len_of_pl[word_count] = file_count;
                word_count++;
            }
        }
    }
    if (word_count < number_of_term) {
        cout<<endl;
        fclose(read_word);
        fclose(read_files);
        fclose(read_index);
        return;
    }
    int result_pl[2000];
    int result_fre[2000];
    int current_pl[2000];
    int current_fre[2000];
    int compare[word_count];
    for (i = 0; i < word_count; i++) {
        compare[i] = -1;
    }
    int j2;
    int smallest;
    int flag;
    for (i = 0; i < word_count; i++) {
        smallest = -1;
        for (j = 0; j < word_count; j++) {
            flag = 0;
            for (j2 = 0; j2 < i; j2++) {
                if(compare[j2] == j) {
                    flag = 1;
                }
            }
            if (flag == 1) {
                continue;
            }
            if (smallest == -1) {
                smallest = len_of_pl[j];
                compare[i] = j;
            }
            else if (smallest > len_of_pl[j]) {
                smallest = len_of_pl[j];
                compare[i] = j;
            }
        }
    }
    int m, n;
    int len1, len2;
    int result_len;
    memcpy(&current_pl, &posting_list[compare[0]], len_of_pl[compare[0]]*sizeof(int));
    memcpy(&current_fre, &occur_times[compare[0]], len_of_pl[compare[0]]*sizeof(int));
    len1 = len_of_pl[compare[0]];
    result_len = len1;
    for (i = 1; i < word_count; i++) {
        len2 = len_of_pl[compare[i]];
        m = 0;
        n = 0;
        result_len = 0;
        
        while (m < len2 and n < len1) {
            if (posting_list[compare[i]][m] == current_pl[n]){
                result_pl[result_len] = posting_list[compare[i]][m];
                result_fre[result_len] = occur_times[compare[i]][m] + current_fre[n];
                result_len++;
                m++;
                n++;
            }
            else if (posting_list[compare[i]][m] < current_pl[n]) {
                m++;
            }
            else if (posting_list[compare[i]][m] > current_pl[n]) {
                n++;
            }
        }
        memcpy(&current_pl, &result_pl, result_len*sizeof(int));
        memcpy(&current_fre, &result_fre, result_len*sizeof(int));
        len1 = result_len;
    }
    if (result_len == 0) {
        cout<<endl;
        fclose(read_word);
        fclose(read_files);
        fclose(read_index);
        return;
    }
    int result_sequence[result_len];
    int largest;
    for (i = 0; i < result_len; i++) {
        largest = - 1;
        for (j = 0; j < result_len; j++) {
            flag = 1;
            for (j2 = 0; j2 < i; j2++) {
                if (result_sequence[j2] == j) {
                    flag = 0;
                }
            }
            if (flag == 0) {
                continue;
            }
            if(current_fre[j] > largest) {
                largest = current_fre[j];
                result_sequence[i] = j;
            }
        }
    }
    string f_name[result_len];
    
    int c = 0;
    i = 0;
    while (i < result_len) {
        fscanf(read_files, "%s", name);
        c++;
        if (c == current_pl[i]) {
            f_name[i] = name;
            i++;
        }
    }
    for (i = 0; i < result_len; i++) {
        cout<<f_name[result_sequence[i]]<<endl;
    }
    fclose(read_word);
    fclose(read_files);
    fclose(read_index);
}

int get_length(int x)
{
    int leng=0;
    while(x)
    {
        x/=10;
        leng++;
    }
    return leng;
}
