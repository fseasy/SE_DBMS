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
        ���ݴ����names ����������link�����ֵ��linkÿ���±��ӦrowMode��ÿ��Ԫ���±�
        link��ÿ��ֵ��Ӧ��RowMode��Ӧ��names�±�
        ������Ҫ���names�Ƿ���ȷ������ȷ����false
    */
    bool setLink(vector<string> & names) ;
    //--describe
    void Describe() ;
    //--display
    bool Display() ;
    //--select
    bool Select(vector<Condition *> & con,vector<string> & projection) ;
    void Projection(int pjOffset[] ,int pjIndex[] ,int colNum) ;
    string GetTableName() ;
    int getFieldNum() ;

private:
    ifstream  data_in ;
    ofstream  data_out ;
    fstream tmpFile ;
    string tName ;
    string tableFileName ;
    vector<RowMode*> rowMode ;
    //fileinfo
    int fieldNum ; //�ֶ���
    int rowSize ; //Ԫ���С(Byte)
    int * link ;
} ;
Table::Table(string tName, string relMode)
{
    this->tName = tName ;
    tableFileName = tName+".txt" ;
    fieldNum = 0 ;
    rowSize = 0 ;
    rowSize+=sizeof(char) ;//��ʶλ
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
    data_out.open(tableFileName.c_str(),ios::binary|ios::out|ios::app) ;
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
                    //д��0
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
        data_out.close() ;
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
    ����������1������������2��ֵ���ʴ���
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
    data_in.open(tableFileName.c_str(),ios::binary|ios::in) ;
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
        delete [] buffer ;
        data_in.close() ;
        return true ;
    }
    else
    {
        Error("Table '"+tName+"' open failed") ;
        return false ;
    }
}
bool Table::Select(vector<Condition *> & con,vector<string>& projection)
{
    int * pjOffset = NULL ;
    int * pjIndex = NULL ;
    int pjsize ;
    if(projection.size() != 0 )
    {
        pjOffset = new int[projection.size()] ;
        pjIndex = new int[projection.size()] ;
        pjsize = projection.size() ;
        for(int i = 0 ; i < projection.size() ; i++)
        {
            int offset = 1 ;
            bool isFound = false ;
            for(int j = 0 ; j < rowMode.size() ; j++)
            {
                if(rowMode.at(j)->fieldName == projection.at(i))
                {
                    pjOffset[i] = offset ;
                    pjIndex[i] = j ;
                    isFound = true ;
                    break ;
                }
                else
                {
                    offset += rowMode.at(j)->fieldSize ;
                }
            }
            if(!isFound)
            {
                Error("No such column named '"+projection.at(i)+"'") ;
                return false ;
            }
        }
    }
    else
    {
        //cout <<"pj empty\n" ;
        int offset = 1 ;
        pjOffset = new int[fieldNum] ;
        pjIndex = new int[fieldNum] ;
        pjsize = fieldNum ;
        for(int i = 0 ; i < rowMode.size() ; i++)
        {
            pjIndex[i] = i ;
            pjOffset[i] = offset ;
            offset+=rowMode.at(i)->fieldSize ;
        }
    }
    bool isMCs[con.size()] ;//is meet conditions
    for(int i = 0 ; i < con.size() ; i++)
    {
        isMCs[i] = true ;
    }
    //is con empty
    data_in.open(tableFileName.c_str(),ios::binary|ios::in) ;
    tmpFile.open("tmp.txt",ios::out|ios::binary) ;
    if(!tmpFile||!data_in)
    {
        Error("Temp file created failed ") ;
        return false ;
    }
    if(!con.empty())
    {

        //���� con
        for(int i = 0 ; i < con.size() ; i++)
        {
            int offset = 1 ;
            int rowIndex = 0 ;
            bool isFound = false ;
            for( ; rowIndex <rowMode.size() ; rowIndex++)
            {
                if(rowMode.at(rowIndex)->fieldName == con.at(i)->attrName)
                {
                    con.at(i)->rowIndex = rowIndex  ;
                    con.at(i)->offset = offset ;
                    isFound = true ;
                    break ;
                }
                else
                {
                    offset+=rowMode.at(rowIndex)->fieldSize ;
                }
            }
            if(!isFound)
            {
                Error("no such column named '"+con.at(i)->attrName+"'") ;
                return false ;
            }
            //start to read
            int rowNum = 10 ;
            char buffer[rowNum*rowSize] ;
            while(!data_in.eof())
            {
                int basepos = 0 ;
                data_in.read(buffer,sizeof(buffer)) ;
                int extractNum = data_in.gcount() ;
                for( ; basepos < extractNum ; basepos+=rowSize)
                {
                    int pos = 0 ;
                    char flag ;
                    memcpy(&flag,buffer+basepos+pos,sizeof(char)) ;
                    if(flag == '1')
                    {
                        //ok,judge
                        bool isMC = true ;
                        for(int i = 0 ; i < con.size() ; i++)
                        {
                            //first,get the type of data
                            string type = rowMode.at(con.at(i)->rowIndex)->fieldType ;
                            if(type == "VARCHAR" || type == "CHAR")
                            {
                                if(con.at(i)->cmpope == "=") //cmpope
                                {
                                    int offset = con.at(i)->offset ;
                                    if(strcmp(buffer+basepos+offset,(con.at(i)->val).c_str()) != 0)
                                    {
                                        isMCs[i] = false ;
                                    }
                                }
                                else
                                {
                                    Error("NOT suported for operator '"+con.at(i)->cmpope+"'") ;
                                    return false ;
                                }
                            }
                            else if(type == "INT")
                            {
                                int x ;
                                int offset = con.at(i)->offset ;
                                memcpy(&x,buffer+basepos+offset,sizeof(int)) ;
                                //cout <<"data is " << x <<"\n" ;
                                int conval ;
                                conval =  atoi((con.at(i)->val).c_str()) ;
                                // cout <<"conval is " <<conval <<"\n" ;
                                if(con.at(i)->cmpope == "=")
                                {
                                    if(x != conval)
                                    {
                                        isMCs[i] = false ;
                                    }
                                }
                                else if(con.at(i)->cmpope == ">")
                                {
                                    if(! (x> conval))
                                    {
                                        isMCs[i] = false ;
                                    }
                                }
                                else if(con.at(i)->cmpope == "<")
                                {
                                    if(! (x< conval))
                                    {
                                        isMCs[i] = false ;
                                    }
                                }
                                else if(con.at(i)->cmpope == "<=")
                                {
                                    if(! (x<= conval))
                                    {
                                        isMCs[i] = false ;
                                    }
                                }
                                else if(con.at(i)->cmpope == ">=")
                                {
                                    if(! (x>= conval))
                                    {
                                        isMCs[i] = false ;
                                    }
                                }
                                else
                                {
                                    Error("Not suported such operatio '"+con.at(i)->cmpope+"'") ;
                                    return false ;
                                }
                            }
                            if(con.at(i)->mulope == "AND")//mulope
                            {
                                isMC = isMCs[i]&isMC ;
                                isMCs[i] = true ;//!!��ԭ��������
                            }
                        }
                        if(isMC)
                        {
                            tmpFile.write(buffer+basepos,rowSize) ;
                        }

                    }

                }
            }
        }
    }
    else
    {
        int rowNum = 10 ;
        char buffer[rowNum*rowSize] ;
        while(!data_in.eof())
        {
            data_in.read(buffer,sizeof(buffer)) ;
            int extractNum = data_in.gcount() ;
            for(int basepos = 0 ; basepos < extractNum ; basepos+=rowSize)
            {
                char flag ;
                memcpy(&flag,buffer+basepos,sizeof(char)) ;
                if(flag == '1')
                {
                    tmpFile.write(buffer+basepos,rowSize) ;
                }
            }
        }
    }
    data_in.close() ;
    tmpFile.close() ;
    //projection
    Projection(pjOffset,pjIndex,pjsize) ;
}
void Table::Projection(int  pjOffset[],int  pjIndex[],int colNum)
{
    tmpFile.open("tmp.txt",ios::in|ios::binary) ;
    if(!tmpFile)
    {
        Error("Temp file open failed") ;
        return ;
    }
    //out put the title
    for(int i = 0 ; i < colNum ; i++)
    {
        int width = rowMode.at(pjIndex[i])->fieldSize ;
        cout <<left << setw(width+4) <<rowMode.at(pjIndex[i])->fieldName ;
    }
    cout << endl ;
    int rowNum = 10 ;
    char buffer[rowNum*rowSize] ;
    while(!tmpFile.eof())
    {
        int basepos = 0 ;
        tmpFile.read(buffer,sizeof(buffer)) ;
        int extractNum = tmpFile.gcount() ;
        for(; basepos < extractNum ; basepos+=rowSize)
        {
            for(int i = 0 ; i < colNum ; i++)
            {
                string type = rowMode.at(pjIndex[i])->fieldType ;
                int width = rowMode.at(pjIndex[i])->fieldSize ;
                if( type == "CHAR" || type == "VARCHAR")
                {
                    // it can output directly
                    cout <<setw(width+4)<< left <<buffer+basepos+pjOffset[i] ;
                }
                else if(type == "INT")
                {
                    int x ;
                    memcpy(&x,buffer+basepos+pjOffset[i],sizeof(int)) ;
                    cout <<setw(width+4) <<left << x ;
                }
            }
            cout << endl ;
        }
    }
    tmpFile.close() ;

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
