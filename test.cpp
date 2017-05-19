//
//  test.cpp
//  a3search
//
//  Created by 张云翮 on 11/05/2017.
//  Copyright © 2017 Zhang Yunhe. All rights reserved.
//

/* strtok example */
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <dirent.h>
#include <regex.h>
#include <algorithm> // for std::find
#include <iterator> // for std::begin, std::end
#include "wn.h"
using namespace std;

int main ()
{
    int a[] = {3, 6, 8, 33};
    int x = 3;
    if ((find(std::begin(a), end(a), x) != end(a))) {
        cout<<"in"<<endl;
    }
    else {
        cout<<"not in"<<endl;
    }
    
}

