External Library File: porter2_stemmer.cpp porter2_stemmer.h

Program have two parts: Build index and Search with terms

1. Build index:

(1) Read original files in directory

(2) Read each file one by one

(3) For each file, get every words, ignore the non-alphabet characters. Also store file name in array ‘file_name’

(4) Transform word to lower case. Use Porter2Stemmer::stem(external library function) to get the words’ stem.

(5) There is a stopword list, contain 121 stop words(already do Porter2Stemmer::stem process), if the words in this list, drop these words.
    reference: http://www.ranks.nl/stopwords (Default English stopwords list)

(6) Use a map to store the information of current file, map the words to frequency of words in current file.
    The words in other files which already read but not in current file are also stored in this map, but the frequence is 0.

(7) If current file is not the last file, a temporary index file will be created.
    Temporary index contain the posting list up to current file, two temporary index files at most
    For every files except the first file, read temporary index created by the previous file and create a new temporary index file based on information in map.
    First file just create temporary index directly.


    Temporary index form: each line is a single word’s posting list, posting list contain file number and the word frequency, For example: 

    app 2-4 1-3 2-1

    This line means word ‘app’ occur in 2nd, 3rd, 4th files and frequency is 4, 3, 1. Notice that, except first file number, the other file
    numbers need to be calculated by ‘first file number’ + ‘current file number’ = ‘true current file number’ (2+1=3 2+2=4)


    To create new temporary index file, read temporary index created by previous file.
    The map is sorted, since the temporary index is written base on map, it is also sorted.
    Go through old temporary index line by line, also go through map
    Suppose get word ‘A’ from index and ‘B’ from map every time, there are 3 situations.
    
    1. ‘A’ is same as ‘B’ but ‘B’ mapping 0 in map.
	Means this word not in current file, but has a posting list in index. So just write this posting list into new temporary index.
	Then get next ‘A’ and ‘B’.

    2. ‘A’ is same as ‘B’ and ‘B’ mapping none zero value in map.
	Means current file has this word, and there is already a posting list for this word in index.
	So expand posting list based on map, and write expanded posting list into new temporary index.
	Then get next ‘A’ and ‘B’.

    3. ‘A’ is not same as ‘B’.
	Means word ‘A’ is a new word for index file, so create a new posting list and write it into new temporary index.
	Then get next ‘A’ but ‘B’ remain the same.

    So the Runtime would be number of words in map.

(8) If current file is the last file, create index directory and create 3 files inside called “files”, “word”, “index”.
	
    “files” : Files’ name are stored in array ‘file_name’, write each file’s name into “files” in order, separated by ‘\n’. 

    “index” : Similar with temporary posting list, but there is no word at front, a single line is look like ‘2-4 1-3 2-1’.

    “word” : Store each word and corresponding posting list’s start location in “index”, each word separated by space.
	     Like ‘4-app’, means the posting list of ‘app’, start location is 4th character in “index”.
             In search part, seek this location and get the whole line, then will get the corresponding posting list.

    Process is quite similar as (7), read old temporary index file.
    But this time, write front word(as well as the start location of posting list) into “word” and write the rest part into “index”.
    If only one file in total, write this 3 files in directory directly.

(9) remove the temporary index files if they exist.

An very simple example for build index:

File1: 
	App.

File2: 
	app, banana.

File1 create temporary index look like:

	app 1-1

File 2 create index directory and 3 files inside

“files” look like:

	File1
	File2

“word” look like:

	0-app 8-banana

“index” look like:

	1-1 1-1
	2-1


2. Search with terms:

(1) Get each term from arguments, transform them to lower case. Use Porter2Stemmer::stem(external library function) to get the terms’ stem.

(2) Find each term in file “word”, get the corresponding start location of posting list.

(3) Use start location of posting list to find posting list in file “index”, use ‘fseek’ to find postion and ‘fgets’ to get the whole line. 

(4) Get each posting list, sorted them based on their length, store file name and word frequency in different arrays. Two arrays have one-to-one corresponding relations.

(5) Do conjunctive query for each term’s posting list, start from shortest to longest.

    Go through two posting lists, suppose get file number ‘F_A’ from ‘P_A’ and ‘F_B’ from ‘P_B’, 3 situations.

    1. If ‘F_A’ and ‘F_B’ are equal.
    Store this file number in new posting list array. Add their frequencies and store in new corresponding array.
    Then get next ‘F_A’ and ‘F_B’

    2. If ‘F_A’ smaller than ‘F_B’
    Get next ‘F_A’

    3. If ‘F_A’ larger than ‘F_B’
    Get next ‘F_B’

    So the Runtime would be length of ‘P_A’ + length of ‘P_B’.

    I do not use skip pointers since number of file will not greater than 2000, if there are millions or billions files, I will definitely use them.

(6) After conjunctive query, get a list of file numbers and their corresponding frequencies, sort them by frequencies, from largest to smallest.

(7) get file names in “files” by sorted file number list.

(8) output the file name in order, if no result, output a new line.



Addition:
About external library function Porter2Stemmer::stem, in most time it works fine, like ‘apples’ and ‘apple’, they are transform to ‘appl’,
then ‘apples’ could be treated as ‘apple’, but sometimes, if a word ‘appl’ in file, it will also be treated like ‘apple’. 
But in most case it is fine and I need this function to save space and time as well as the search accuracy(when search apple, apples should be matched).

Concept search not be included in this program, but still accept -c option.