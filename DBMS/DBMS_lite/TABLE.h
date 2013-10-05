#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <list>
#include <string>
#include "DBMS.h"

class Table
{
public:
    Table(string tName,string relMode) ;
    ~Table() ;
    //--insert
    bool Insert(vector<string> & names,vector<string> & values) ;
    bool writeBufOneField(string & val,int rmIndex,int & pos,char * & buffer) ;
    /**
        根据传入的names 向量，设置link数组的值，link每个下标对应rowMode的每个元素下标
        link的每个值对应该RowMode对应的names下标
        首先需要检查names是否正确，不正确返回false
    */
    bool setLink(vector<string> & names) ;
    //--describe
    void Describe() ;
    //--select
    bool Display() ;
    string GetTableName() ;
    int getFieldNum() ;

private:
    ifstream  data_in ;
    ofstream  data_out ;
    string tName ;
    vector<RowMode*> rowMode ;
    //fileinfo
    int fieldNum ; //字段数
    int rowSize ; //元组大小(Byte)
    int * link ;
} ;
Table::Table(string tName, string relMode)
{
    this->tName = tName ;
    string fileName = tName+".txt" ;
    data_in.open(fileName.c_str(),ios::binary|ios::in) ;
    data_out.open(fileName.c_str(),ios::binary|ios::out|ios::app) ;
    fieldNum = 0 ;
    rowSize = 0 ;
    rowSize+=sizeof(char) ;//标识位
    vector<string> fields ;
    vector<string> singleField ;
    char * pfields = Split(relMode,",",fields) ;
    fieldNum = fields.size() ;
    link = new int[fieldNum] ;
    for(int i = 0 ; i < fieldNum ; i++)
    {
        link[i] = -1 ;
    }
    //fields has stored all fields which split by ';'
    //let's store every field to the vector rowMode
    for(int i = 0 ; i < fields.size() ; i++)
    {
        string tmp = fields.at(i) ;
        Trim(tmp) ;
        char * psingleField = GetAllWords(tmp,singleField) ;
        RowMode * tmpRow  = new RowMode;
        tmpRow->fieldName = singleField.at(0) ;
        Trim(tmpRow->fieldName) ;
        tmpRow->fieldType = singleField.at(1) ;
        Trim(tmpRow->fieldType) ;
        if(singleField.size() == 3)
        {
            tmpRow->fieldSize = atoi(singleField.at(2).c_str()) ;
        }
        else
        {
            if(tmpRow->fieldType == "INT")
            {
                tmpRow->fieldSize = sizeof(int) ;
            }
        }
        //size
        rowSize+=(tmpRow->fieldSize) ;
        //cout << tmpRow.fieldName <<tmpRow.fieldType << tmpRow.fieldSize << endl ;
        rowMode.push_back(tmpRow) ;
        delete psingleField ;
    }
    delete pfields ;

}
Table::~Table()
{
    for(int i = 0 ; i < rowMode.size() ; i++)
    {
        delete rowMode.at(i) ;
    }
    delete link ;
    data_in.close() ;
    data_out.close() ;
}
void Table::Describe()
{
    cout << "\n" <<formatStr(tName) <<" : " ;
    for(int i = 0 ; i < rowMode.size() ; i++)
    {
        cout << formatStr(rowMode.at(i)->fieldName) <<" "
             << rowMode.at(i)->fieldType;
        if(rowMode.at(i)->fieldType == "CHAR" || rowMode.at(i)->fieldType == "VARCHAR")
        {
            cout <<"(" <<rowMode.at(i)->fieldSize <<")" ;
        }
        if(i != rowMode.size() -1)
        {
            cout <<", " ;
        }
    }
    cout << endl ;
}
bool Table::Insert(vector<string> & names,vector<string> & values)
{

    char * rowbuf = new char[rowSize] ;
    //!!first ,write flag
    char flag = '1' ;
    int pos = 0 ;
    memcpy(rowbuf+pos,&flag,sizeof(flag)) ;
    pos+=sizeof(char) ;
    //write data
    if(names.empty())
    {
        for(int i = 0 ; i < rowMode.size() ; i++)
        {
            string val = values.at(i) ;
            Trim(val) ;
            if(!writeBufOneField(val,i,pos,rowbuf))
            {
                return false ;
            }
        }

    }
    else
    {
        if(setLink(names))
        {
            for(int i = 0 ; i < fieldNum ; i++)
            {
                if(link[i] == -1)
                {
                    //写入0
                    int fsize = rowMode.at(i)->fieldSize ;
                    char * zero = new char[fsize];
                    for(int j = 0 ; j < fsize ; j++)
                    {
                        zero[j] =  '\0' ;
                    }
                    memcpy(rowbuf+pos,zero,fsize) ;
                    pos+=fsize ;
                    delete [] zero ;
                }
                else
                {
                    string val = values.at(link[i]) ;
                    Trim(val) ;
                    if(!writeBufOneField(val,i,pos,rowbuf))
                    {
                        return false ;
                    }
                }
            }
        }
        else
        {
            return false ;
        }


    }
    //rowbuff ok
    //write to file
    if(data_out)
    {
        data_out.write(rowbuf,rowSize) ;
        data_out.flush() ;
        return true ;
    }
    else
    {
        Error("Table '"+tName+"' open failed !") ;
        return false ;
    }

}
bool Table::writeBufOneField(string & val,int rmIndex ,int & pos,char * & buffer)//rmIndex for rowMode index
{
    if(rowMode.at(rmIndex)->fieldType == "CHAR" || rowMode.at(rmIndex)->fieldType =="CHAR")
    {
        int size = rowMode.at(rmIndex)->fieldSize ;
        char * str = new char[size] ;
        for(int i = 0 ; i < size ; i++)
        {
            str[i] = '\0' ;
        }
        Trim(val) ;
        if(val.length() < 1 || (val[0] != '\''&& val[0] != '"') || \
                (val[val.length()-1] != '\''&&val[val.length()-1]!='"'))
        {
            Error("Insert value error , at '"+val+"' ,it should be char(varchar)") ;
            delete [] str ;
            return false ;
        }
        //
        val = val.substr(1,val.length()-2) ;//clean the " / '
        strcpy(str,val.c_str()) ;
        memcpy(buffer+pos,str,size) ;
        pos+=size ;
        delete [] str ;
        return true ;
    }
    else if(rowMode.at(rmIndex)->fieldType == "INT")
    {
        Trim(val) ;
        int buf = atoi(val.c_str()) ;
        memcpy(buffer+pos,&buf,sizeof(int)) ;
        pos+=sizeof(int) ;
        return true ;
    }
    Error("Not supported data type") ;
    return false ;
}
bool Table::setLink(vector<string> & names)
{
    for(int i = 0 ; i < fieldNum ; i++)
    {
        link[i] = -1 ;
    }
    //check
    /**
    sample:
    mode : name , id ,password
            -1    0   -1
    attN : id ,psss
    有索引的有1个，而传入了2个值，故错误
    */
    int hasLink = 0 ;
    int size = names.size() ;
    for(int i =0 ; i < fieldNum ; i++)
    {
        string attrName = rowMode.at(i)->fieldName ;
        for(int j =0 ; j < size ; j++)
        {
            string tmp = names.at(j) ;
            Trim(tmp) ;
            if(tmp == attrName)
            {
                hasLink++ ;
                link[i] = j ;
                break ;
            }
        }
    }
    if(hasLink < size)//error
    {
        //ouput the error names
        for(int i = 0 ; i <  fieldNum ; i++)
        {
            if(link[i] != -1)
            {
                names.erase(names.begin()+link[i]) ;
            }
        }
        string errorInfo = "No such columns named " ;
        while(!names.empty())
        {
            errorInfo+="' " ;

            errorInfo+=names.back() ;
            names.pop_back() ;
            errorInfo+=" ' " ;
        }
        Error(errorInfo) ;
        return false ;
    }
    else
    {
        return true ;
    }
}
bool Table::Display()
{
    if(data_in)
    {
        data_in.seekg(0,data_in.beg) ;//to the begin
        int lines = 10 ;
        char * buffer = new char[lines*rowSize] ;
        while(!data_in.eof())
        {
            data_in.read(buffer,rowSize*lines) ;
            int extractNum = data_in.gcount() ;
            int pos = 0 ;
            while(pos< extractNum)
            {
                //check the flag
                char flag ;
                memcpy(&flag,buffer+pos,sizeof(char)) ;
                pos+=sizeof(char) ;
                if(flag == '1')
                {
                    for(int i = 0 ; i < rowMode.size() ; i++)
                    {
                        string type = rowMode.at(i)->fieldType ;
                        Trim(type) ;
                        if( type == "CHAR" || type == "VARCHAR")
                        {
                            // it can output directly
                            cout <<setw(rowMode.at(i)->fieldSize)<< left <<buffer+pos <<"    ";//4 spaces
                            pos+= rowMode.at(i)->fieldSize;
                        }
                        else if(type == "INT")
                        {
                            int x ;
                            memcpy(&x,buffer+pos,sizeof(int)) ;
                            cout <<setw(6) <<left << x <<"    " ;
                            pos+=sizeof(int) ;
                        }
                    }
                    cout << endl ;
                }
                else
                {
                    pos+=(rowSize-sizeof(char)) ;
                }

            }
        }
        return true ;
    }
    else
    {
        Error("Table '"+tName+"' open failed") ;
        return false ;
    }
}
string Table::GetTableName()
{
    return tName ;
}
int Table::getFieldNum()
{
    return fieldNum ;
}
#endif // TABLE_H_INCLUDED
