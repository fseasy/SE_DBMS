#include <iostream>
#include <fstream>
using namespace std;

int main()
{
    fstream x("tmp.txt",ios::out|ios::binary|ios::app) ;
    ifstream xx("tmp.txt") ;
    char ori[10] = "this is" ;
    char rewrite[4] = {'m','e','m','e'} ;
    x.write(ori,sizeof(ori)) ;
    char buffer[10] ;
    xx.read(buffer,sizeof(buffer)) ;
    cout << buffer ;
    //将文件指针移动到需要修改的地方，再写入数值，即能覆盖掉原来的内容
    x.seekg(0,x.beg) ;
    x.write(rewrite,sizeof(rewrite)) ;
    x.flush() ;//记得及时flush，将缓冲区写入磁盘，否则只有在关闭后磁盘内容才可能更改，会导致出错
    xx.read(buffer,sizeof(buffer)) ;
    cout << buffer ;
    x.close() ;
    xx.close() ;
    return 0;
}
