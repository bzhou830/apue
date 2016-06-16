#include <iostream>
#include <string>
using namespace std;

void swap(string::iterator &it1, string::iterator &it2)
{
    char tmp = 0;
    tmp = *it1;
    *it1 = *it2;
    *it2 = tmp;
}


void Deal(string & str)
{
    string::iterator it =  --str.end();
    string::iterator tail = --str.end();
    while(it != str.begin())
    {
        if(*it >= 'A' && *it <= 'Z')
        {
            swap(it, tail);    
            tail--;
        }
        it--;
    }
}


int main()
{
    string str;
    while(cin>>str)
    {
        Deal(str);
        cout<<str;
        str.clear();
    }
    return 0;
}
