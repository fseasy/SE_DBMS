;">#include <iostream>
#include <cstring>
using namespace std;

int main()
{
    char buffer[20] ;
    char name[16]="This" ;
    for(int i = 4 ; i < sizeof(name) ; i++)
    {
        name[i] = '\0' ;
    }
    cout <<name ;
    int x = 589089 ;
    memcpy(buffer,name,sizeof(name)) ;
    memcpy(buffer+16,&x,sizeof(int)) ;
    for(int i = 0 ; i < sizeof(name) ; i++)
    {
        cout << name[i] ;
    }
    cout << endl ;
    for(int i = 0 ; i < sizeof(buffer) ; i++)
    {
        cout << buffer[i] ;
    }
    cout << endl ;
    int t ;
    memcpy(&t,buffer+sizeof(name),sizeof(int)) ;
    cout << t ;
    return 0;
}
