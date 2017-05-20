//
//  test.cpp
//  a3search
//
//  Created by 张云翮 on 11/05/2017.
//  Copyright © 2017 Zhang Yunhe. All rights reserved.
//

/* strtok example */
#include <stdio.h>
#include "wn.h"

int main() {
    
    char str[] = "investment";
    
    SynsetPtr synsets;
    
    synsets = findtheinfo_ds(str, NOUN, HYPOPTR, ALLSENSES);
    
    return 0;
    
}
