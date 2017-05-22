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
 
    2.Do Porter2Stemmer process with each word and Drop the word in stopword list
 
    3.Store words in map, map the word to the frequence of this word in file
 
    4.Build the posting list, write in a temporary index file
 
    There are 2 temporary index files, for each file, it will read one which written by previous file,
    then write a updated file. Which means the posting list in index file will update each time.
 
    In temporary index, a single line form is "word file_number-frequence". For example:
    'app 2-2 3-4 5-1', means word 'app' occur in 2nd(2 times), 5th(4 times), 7th(1 time) files
    notice that, except first number, the file number need to be calculated.
    example: for 3-4, the file is 5th file, first number + current number (2+3=5)
 
    5.when reach the last file in directory, create index directory(xx.idx), and create 3 files "files", "index", "word"
 
    "files": contain every files' name, separate by "\n";
 
    "index": contain posting list, each line is a word. also include word frequence in each file. 
            similar with temporary index file, but no word at front like '2-2 3-4 5-1'
 
    "word" : contain words, form is location-word, the location is the corresponding posting list location in "index"
            like "4-app" means the word "app" posting list start location is 4th character in "index", words separated by
            space.
 
    6.rmove the temporary index files.
 */
void build_index(const char * argument1, const char * argument2) {
    
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
    //Open directory
    DIR *pDIR;
    struct dirent *entry;
    /* 
        file_in : read original file
        index_read : read temporary index
        index_write : wirte index (including temporary index and final index)
        total_word : wirte "word" in index directory
        write_filename : write "files" in index dirctory
     */
    FILE *file_in, *index_read, *index_write, *total_word, *write_filename;
    
    //get word in orignal file
    char * word;
    
    //for readline
    int k = 2003256;
    char readfile[k];
    
    //store the path of file since files is in directory
    char name[300];
    
    //store file name
    string file_name[2000];

    // i use in for loop
    // last_number for write the last file number in posting list
    int i, last_number;
    
    // location for geting substring
    // seek_location, store the start position of each word's posting list
    unsigned long location, seek_location;
    
    //temporary index name
    static const char* const index[] = {"temporary_index1", "temporary_index2"};
    
    // maping word and its frequence
    map<string, int> word_list;
    
    // map iterator for go through map
    typedef map<string, int>::const_iterator MapIterator;
    
    //count the number of file
    int file_number = 0;
    
    // index_word store the word after stem and stopword process
    // posting_list sotre the posting list
    string index_word, posting_list;
    
    // use non-alphabet characters as delimiter
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
                read each file except ‘.’ , ‘..’ and '.DS_Store'
             */
            cout<<entry->d_name<<endl;
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".DS_Store") != 0) {
                //open the file
                
                strcpy(name, argument1);
                strcat(name, "/");
                strcat(name, entry->d_name);
                file_in = fopen(name, "r");
                file_name[file_number] = entry->d_name;
                file_number++;
                //read each line and separate each word in line
                while(fgets(readfile, k, file_in) != NULL) {
                    word = strtok(readfile, delim);
                    
                    // change word to lower case, do Porter2Stemmer::stem process, and drop stopwords
                    // then store in map, count the frequence
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
            while(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".DS_Store") == 0) {
                entry = readdir(pDIR);
                cout<<entry->d_name<<endl;
            }
            /*
                if it is the last file, start build index file, and the file inside("files" "word" "index")
             */
            if (entry == NULL) {
                cout<<"last file"<<endl;
                //write file names in "files"
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
                    fclose(write_filename);
                }
                /*
                    if the total number of file greater than 1
                    read the temporary index written by previous file
                    update the posting list in each line and write them to final index file "index"
                    also write "files" and "word"
                 */
                else {
                    index_read = fopen(index[(file_number-1)%2], "r");
                    strcpy(name, argument2);
                    strcat(name, "/index");
                    index_write = fopen(name, "w");
                    strcpy(name, argument2);
                    strcat(name, "/word");
                    total_word = fopen(name, "w");
                    fgets(readfile, k, index_read);
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
                            fgets(readfile, k, index_read);
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
            // if it is the first file no need to update posting list, just write it into temporary index file with map
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
            /*
                if it is not the first file, need to read each line of last temporary index, update it by map,
                if new word in map, wirte a new line as posting list and write it into new temporary index file,
                if the word already in index file, update the posting list and write it into new temporary index file
             */
            else {
                index_read = fopen(index[(file_number-1)%2], "r");
                index_write = fopen(index[file_number%2], "w");
                fgets(readfile, k, index_read);
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
                        fgets(readfile, k, index_read);
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

/*
    Search the terms with index directory.
 
    1.find words in file "word"
    
    2.get the seek location of word
 
    3.get posting list of word in file "index" by seek location
 
    4.do conjunctive query for every terms (start with shortest posting list)
 
    5.get result file number, they should be sorted by frequence
 
    6.with the results, get file name from file "word", then output them
 
    7.if no results, output newline
 
 */
void search_terms(const char * argument2, string search_terms[], int number_of_term) {
    
    //for read line
    int k = 2003256;
    char read[k];
    
    //read 3 files in index directory("word" "index" "files")
    FILE *read_word, *read_index, *read_files;
    
    //store path of file
    char name[300];
    
    //get word
    char word[256];
    
    //posting list, value is file number
    int posting_list[5][2000] = {0};
    
    //corresponding to posting_list, value is frequence
    int occur_times[5][2000] = {0};
    
    //store length of posting list
    int len_of_pl[5] = {0};
    
    // seek_location : get posting list start location in "index"
    // i, j use in for loop
    // word_count file_count count the words and files
    int seek_location, i, j, word_count, file_count;
    
    // every single part in posting list, like file_number-frequence
    char * single_of_pl;
    
    // get_word get the word in "word"
    // index_file get file number
    string get_word, index_file;
    
    //open 3 files in index directory
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
    
    //get every search terms' posting list, store file number in posting_list, store frequence in occur_times
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
                    index_file = single_of_pl;
                    posting_list[word_count][file_count] = atoi(index_file.substr(0, index_file.find("-")).c_str());
                    occur_times[word_count][file_count] = atoi(index_file.substr(index_file.find("-") + 1, strlen(index_file.c_str()) - index_file.find("-")).c_str());
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
    
    //if number of posting list is smaller than number of search terms, means some word have no match
    //output newline,stop function.
    if (word_count < number_of_term) {
        cout<<endl;
        fclose(read_word);
        fclose(read_files);
        fclose(read_index);
        return;
    }
    
    //prepare for conjunctive query, result_pl and result_fre store the conjunctive result
    //current_pl and current_fre sotre current posting list for conjunctive query
    int result_pl[2000];
    int result_fre[2000];
    int current_pl[2000];
    int current_fre[2000];
    
    //this part is for sort the posting list by their length, store result in array "compare"
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
    
    //conjunctive query, start from shortest postings list to longest postings list
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
    
    //if no results, output newline and stop function
    if (result_len == 0) {
        cout<<endl;
        fclose(read_word);
        fclose(read_files);
        fclose(read_index);
        return;
    }
    
    //get file names from "files"
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
    
    //sort the file number by frequence, if same, sort in ascending order (lexicographically).
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
            else if (current_fre[j] == largest) {
                if((strcmp(f_name[j].c_str(), f_name[result_sequence[i]].c_str())) < 0) {
                    largest = current_fre[j];
                    result_sequence[i] = j;
                }
            }
        }
    }

    
    
    //output the result file name in order
    for (i = 0; i < result_len; i++) {
        cout<<f_name[result_sequence[i]]<<endl;
    }
    fclose(read_word);
    fclose(read_files);
    fclose(read_index);
    return;
}

//function for get length of number(number of digits)
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
