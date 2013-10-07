#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
using namespace std ;

struct RowMode
{
    string fieldName ;
    string fieldType ;
    int    fieldSize ;
} ;
struct Condition
{
    string attrName ;
    string val ;
    string mulope ; //'and' or 'or' ,etc
    string cmpope ; //'>' or '<' '=' ,etc
    int offset ; // data offset
    int rowIndex ; //index of rowMode
} ;
/**
        去掉首尾空格/其他字符
*/
string Trim(string & str,char trim = ' ')
{
    string::iterator pos ;
    for(pos = str.begin() ; pos != str.end() ; pos++)
    {
        if(*pos != trim)
        {
            break ;
        }
    }
    str.erase(str.begin(),pos) ;//erase(iterator begin,iterator end) or erase(int begin,int end)
    for(pos = str.end() ; pos != str.begin() ; pos--)
    {
        if(*pos != trim)
        {
            break ;
        }
    }
    str.erase(pos,str.end()) ;
    return str ;
}
/**
    get the first word
*/
bool GetFirWord(char* word , const string str)
{
    unsigned int pos ;
    for(pos = 0 ; pos < str.length() ; pos++)
    {
        if(str.at(pos) != ' ')
        {
            word[pos] = str[pos] ;
        }
        else
        {
            word[pos] = '\0' ;
            return true ;
        }
    }
    return false ;
}
/**
    split,use vector

    stock:

     (1). 第一次调用 strtok 时, 第一个参数是 strToken, 以后再调用时, 第一个参数必须是 NULL;

     (2). 调用 strtok 后, 原字符串会被修改;

     (3). strtok 不是一个线程安全的函数.
*/
char * Split(const string str,char * split,vector<string> & res)
{
    char * cstr = new char[str.length()+1] ;// aa len = 2 memory = 3
    strcpy(cstr,str.c_str()) ;
    char * p ;
    p = strtok(cstr,split) ;
    while(p != NULL)
    {
        res.push_back(p) ;
        p = strtok(NULL,split) ;
    }
    return cstr ;//return for delete
}
/**
    get all words

*/
inline bool IsLegal(char c)
{
    if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_')
    return true ;
    else
    return false ;
}
char * GetAllWords(const string str,vector<string> & words)
{
    words.clear() ;//First Clear it!!
    int len = str.length() ;
    char * cstr = new char[len+1] ;
    strcpy(cstr,str.c_str()) ;
    for(int i = 0 ; i < len ; i++)
    {
        //将所有非字母字符变为\0
        if(!IsLegal(cstr[i]))
        {
            cstr[i] = '\0' ;
        }
    }
    //process
    bool isHead = true ;
    for(int i = 0 ; i < len ; i++)
    {
        if(isHead==true && cstr[i] != '\0')
        {
            string tmp(cstr+i) ;
            words.push_back(tmp) ;
            isHead = false ;
        }
        if(isHead == false &&cstr[i] =='\0'&&((i+1)<str.length()) &&cstr[i+1] != '\0' )
        {
            isHead = true ;
        }
    }
    return cstr ;
}
string formatStr(const string str)
{
    string res = str ;
    for(int i = 1 ; i < res.length() ; i++)
    {
        res[i] = tolower(res[i]) ;
    }
    return res ;
}
void Error(string errorInfo)
{
    cout <<"\nError at : " << errorInfo <<endl ;
}
