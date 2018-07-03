//
//  flash_mock.cpp
//  test_gtest
//
//  Created by huangfa on 2018/7/3.
//  Copyright © 2018年 huangfa. All rights reserved.
//

#include <map>
#include <string>
#include <utility>
#include <stdio.h>
#include <iostream>

using namespace std;

void test_map(void)
{

    map<int, string> mapStudent;
    map<int, string>::iterator iter;

    mapStudent.insert(pair<int, string>(1, "student_one"));

    mapStudent.insert(pair<int, string>(2, "student_two"));

    mapStudent.insert(pair<int, string>(3, "student_three"));

    mapStudent[4] = "student_four";

    mapStudent[10] = "student_ten";

    mapStudent[10] = "new student_ten";

    cout<<"size is " << mapStudent.size() << endl;

    //to find a key
    iter = mapStudent.find(10);
    if(iter != mapStudent.end())
        cout<<"Find, the value is "<<iter->second<<endl;
    else
        cout<<"Do not Find"<<endl;

    //erase the key
    mapStudent.erase(iter);
    cout<<"erase the key "<<iter->second << endl;

    iter = mapStudent.find(10);
    if (iter == mapStudent.end())
    {
        cout << "Do not find after erase"<<endl;
    }

    //erase a range of key [start, end)
    {
        map<int, string>::iterator iter_s = mapStudent.find(2);
        map<int, string>::iterator iter_e = mapStudent.find(4);

        cout<<"erase range from "<<iter_s->first<<" to key "<<iter_e->first<<endl;
        mapStudent.erase(iter_s, iter_e);

        iter = mapStudent.find(3);
        if (iter == mapStudent.end()){
            cout<<"after erase range, key 3 not exist"<<endl;
        }
    }
    cout<<"size is " << mapStudent.size() << endl;

    for(iter = mapStudent.begin(); iter != mapStudent.end(); iter++)

        cout<<iter->first<<' '<<iter->second<<endl;

}
