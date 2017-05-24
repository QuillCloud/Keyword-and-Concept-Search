//
//  a3search.h
//  a3search
//
//  Created by 张云翮 on 06/05/2017.
//  Copyright © 2017 Zhang Yunhe. All rights reserved.
//

#ifndef a3search_h
#define a3search_h


#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <dirent.h>
#include <regex.h>
#include <cstring>
#include <algorithm>
#include <iterator>

using namespace std;
void build_index(const char * argument1, const char * argument2);
void n_search_terms(const char * argument2, string search_terms[], int number_of_term);
void c_search_terms(const char * argument2, string search_terms[], int number_of_term, float cnum);
int get_length(int x);

#endif /* a3search_h */
