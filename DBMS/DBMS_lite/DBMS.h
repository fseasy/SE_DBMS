#ifndef DBMS_H_INCLUDED
#define DBMS_H_INCLUDED
enum SQLComand {CREATE,INSERTE,DELETE} ;
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <windows.h>
#include "header.cpp"
#include "TABLE.h"
using namespace std ;

class DBMS
{
public:
    DBMS() ;
    ~DBMS() ;
    /**
    print welcome info
    */
    void welcome() ;
    /**
    DBMS start receive the SQL and work!
    */
    /**
        Init
        it is mainly used to handle the file
        to read from the file 'model' to init the vector tables
    */
    bool Init() ;
    void Command() ;
    //----------create------------------
    /**
       SQLCreate_Analysize
       process 'create' sql , if it's ok , get the table name and relation mode
       else reture false , sql exist errors
        */
    bool SQLCreate_Analyze(const string sql,string & tName,string & relMode) ;
    /**
        AddTable
        add a table pointer to the tables and invoke the class Table's init to create a
        new table
        如果是在初始化时调用，不需向model文件写入数据
        若不是，则需要向model写入表的关系模式，默认为false
    */
    bool AddTable(string tName, string relMode,bool isInit = false ) ;
    //----------end create------------------------
    //----------Describe---------------------
    /**
        SQLDescribe_Analysize
        get the table name form the sql and judge if  the name is valid
        if so ,get the table pointer
        else return false
    */
    bool SQLDescribe_Analysize(const string sql,string & tName,Table * & table) ;
    /**
        DescribeTable
        just invoke the table's function
    */
    void DescribeTable(Table * table) ;
    //----------end describe-----------------
    //show tables
    bool SQLShow_Analysize(const string sql) ;
    void ShowTables() ;
    //Display
    bool SQLDisplay_Analysize(const string sql,Table * & table) ;
    void DisplayTable(Table * table) ;
    //----------Insert-----------------------

    bool SQLInsert_Analysize(const string sql,string & attrName,string & attrVal,Table *& table) ;
    bool InsertTable(string attrName,string attrVal,Table * table) ;
    //-----------end Insert-------------------
    //select
    /**
        select
        1.firstly,we should realize the basic requirement,that is :
            single table,include projection（投影）, where condition basic supported
            in where condition ,just sipport 'and',and '>','=','<';

    */
    bool SQLSelect_Analysize(const string sql,vector<Table *> & tabs,vector<Condition *> & con,vector<string>& projection) ;
    bool SelectTable(vector<Table *> & tabs,vector<Condition *> & con,vector<string> & projection) ;
    bool IsSupDataType(const string dataType) ;//is the supported data type
private:
    string dbName ;
    fstream model ; //model file stream
    vector<Table *> tables ; //tables
    string oriSQL ; //!!由于SQL分析时需要忽略大小写，而insert等操作时又需要不能忽略大小写
    //!!于是就保存一份原始SQL，用upperSQL分析，用oriSQL插入
    //!!这并不好，但是开始时没考虑到，现在也只能这么做了
};

DBMS::DBMS()
{
    dbName = "MEMEDA" ;
    if(!Init())
    {
        exit(0)  ;
    }
    welcome() ;
    ShowTables() ;
    Command() ;
}
DBMS::~DBMS()
{
    for(int i = 0 ; i < tables.size() ; i++)
    {
        delete tables.at(i) ;
    }
}

void DBMS::welcome()
{
    cout <<"\t\t\t欢迎使用 "+dbName+" 数据库管理系统\n" ;
}
bool DBMS::Init()
{
    model.open("model.txt",ios::app|ios::binary|ios::in) ;//ios::app to create file if not exist
    //restore data
    if(!model)
    {
        Error("Model File init error ;") ;
        return false ;
    }
    model.seekg(0,model.end) ; //move the pointer to the end of the file
    int fileLen = model.tellg() ;//get length
    model.seekg(0,model.beg) ;
    int bufferSize  = 20;
    char buffer[bufferSize] ;
    char * tName = new char[100] ;
    char * relMode = new char[500] ;
    bool readName = true ;
    bool readMode = false ;
    int namepos = 0 ;
    int modepos = 0 ;
    while(!model.eof())
    {
        model.read(buffer,bufferSize) ;
        int pos = 0 ;
        int extractedNum = model.gcount() ;// the extracted number of read
        for( ; pos < extractedNum ; pos++)
        {
            if(buffer[pos] == ':')
            {
                readName = false ;
                readMode = true ;
                //tName ok
                tName[namepos] = '\0' ;
            }
            else if(buffer[pos] == '\n')
            {
                relMode[modepos] = '\0' ;//!!
                readMode = false ;
                readName = true ;
                //relMode ok
                //add Table
                string tmpTName(tName) ;
                cout << tmpTName ;
                string tmpRelMode(relMode) ;
                cout << tmpRelMode << endl;
                AddTable(tmpTName,tmpRelMode,true) ;
                //-----------end--------------
                //next
                namepos = 0 ;
                modepos = 0 ;

            }
            else
            {
                if(readName)
                {
                    tName[namepos] = buffer[pos] ;
                    namepos++ ;
                }
                else if(readMode)
                {
                    relMode[modepos] = buffer[pos] ;
                    modepos++ ;
                }

            }
        }
    }
    delete [] tName ;
    delete [] relMode ;
    model.close() ;
    return true ;
}
void DBMS::Command()
{
    string sql="" ;
    while(cout <<"\nMEMEDA> ")
    {
        getline(std::cin,sql) ;//getline 由string接收，cin.getline由char* 接收
        //to know has sql input over ;
        //if is the last char the ';' , not over
        Trim(sql) ;
        if(sql == "")
        {
            Error("Please input sql") ;
            continue ;
        }
        while(sql.at(sql.length()-1) != ';')
        {
            cout <<"      >" ;
            string tmp="" ;
            getline(cin,tmp) ;
            Trim(tmp) ;
            tmp= " "+tmp ;//add a space to avoid 连在一起
            sql+=tmp ;
        }
        //sql has been input over
        //upper str
        //!!保存一份原始SQL
        oriSQL = sql ;
        transform(sql.begin(),sql.end(),sql.begin(),::toupper) ;//toupper 前加::
        //get first word
        char cCommand[100] = "" ;
        GetFirWord(cCommand,sql) ; //GetFirWord(char * word,const string str)
        string command(cCommand) ; //貌似将string传入GetFirWord可能会崩溃，因为分配空间不够（由于函数中采用的是直接赋值，有问题）
        //judge
        if("CREATE" == command)
        {
            string tName ;
            string relMode ;
            if(SQLCreate_Analyze(sql,tName,relMode))
            {
                AddTable(tName,relMode,false) ;
            }
            else
            {
                Error("Create failed !") ;
            }
        }
        else if("DESCRIBE" == command)
        {
            string tName ;
            Table * table = NULL ;
            if(SQLDescribe_Analysize(sql,tName,table))
            {
                DescribeTable(table) ;
            }
            else
            {
                Error("No table which name is '"+tName+"'") ;
            }
        }
        else if("SHOW" == command)
        {
            if(SQLShow_Analysize(sql))
            {
                ShowTables() ;
            }
            else
            {
                Error("Show tables failed !") ;
            }
        }
        else if("DISPLAY" == command)
        {
            Table * table ;
            if(SQLDisplay_Analysize(sql,table))
            {
                DisplayTable(table) ;
            }
            else
            {
                Error("Display table faild") ;
            }
        }
        else if("INSERT" == command)
        {
            string attrName ;
            string attrVal ;
            Table * table = NULL ;
            if(SQLInsert_Analysize(sql,attrName,attrVal,table))
            {
                InsertTable(attrName,attrVal,table) ;
            }
            else
            {
                Error("Insert failed !") ;
            }
        }
        else if("SELECT" == command)
        {
            vector<string> projection ;
            vector<Condition *> con ;
            vector<Table *> tabs ;
            if(SQLSelect_Analysize(sql,tabs,con,projection))
            {
                SelectTable(tabs,con,projection) ;
            }
            else
            {
                Error("Select faild") ;
            }
            /*
            for(int i =0 ; i < projection.size() ; i++)
            {
                cout << projection.at(i) <<"\t" ;
            }
            cout << endl ;
            for(int i = 0 ; i < con.size() ; i++)
            {
                cout <<con.at(i)->mulope <<"\t" <<con.at(i)->attrName <<"\t"\
                     <<con.at(i)->cmpope <<"\t" <<con.at(i)->val <<endl ;
            }
            */
            for(int i = 0 ; i < con.size() ; i++)
            {
                delete con.at(i) ;
            }
        }
        else
        {
            Error("' "+command+" ' , no such commands !") ;
        }


    } ;
}
bool DBMS::SQLCreate_Analyze(const string sql,string & tName , string & relMode)
{
    vector<string> words ;
    char * x = GetAllWords(sql,words) ;
    //is correct
    //key word 'table' is correct
    if(words.at(1) != "TABLE")
    {
        return false ;
    }
    //bracket is matched
    int lqNum = 0 ; //左括号数目
    for(int i = 0 ; i < sql.length() ; i++)
    {
        if(sql.at(i) == '(')
        {
            lqNum++ ;
        }
        else if(sql.at(i) == ')')
        {
            lqNum-- ;
        }
    }
    if(lqNum != 0)
    {
        Error("Not matched bracket") ;
        return false ;
    }
    //words has stored all the words of sql
    //get table name
    tName = words.at(2) ;//the 3rd word
    //is the name ok
    Trim(tName) ;
    for(int i = 0 ; i < tables.size() ; i++)
    {
        if(tName == tables.at(i)->GetTableName())
        {
            Error("The Table '"+tName+"' has already existed !") ;
            return false ;
        }
    }
    //get table relMode
    int hpos,rpos ;
    hpos = sql.find('(') ;
    rpos = sql.rfind(')') ;
    relMode = sql.substr(hpos+1,rpos-hpos -1) ;
    delete [] x ;
    //is relMode correct
    words.clear() ;
    x = Split(relMode,",",words) ;
    for(int i = 0 ; i < words.size() ; i++)
    {
        string fieldTmp = words.at(i) ;
        vector<string> attr ;
        char * xx = GetAllWords(fieldTmp,attr) ;
        string type = attr.at(1) ;//dataType
        Trim(type) ;
        if(!IsSupDataType(type))
        {
            Error("Not supported data type :\""+type+"\"") ;
            return false ;
        }
        delete xx ;
    }
    delete [] x ;
    //ok
    return true ;
}
bool DBMS::AddTable(string tName,string relMode, bool isInit)
{
    Trim(tName) ;
    Trim(relMode) ;
    Table * table = new Table(tName,relMode) ;
    tables.push_back(table) ;
    if(!isInit)
    {
        model.open("model.txt",ios::out|ios::app|ios::binary) ;
        string mode = tName+":"+relMode+"\n" ;
        char buffer[200] = "" ;
        strcpy(buffer,mode.c_str()) ;
        model.write(buffer,mode.length()) ;
        model.close() ;
    }
    return true ;
}
bool DBMS::SQLDescribe_Analysize(const string sql , string & tName,Table * & table)
{
    vector<string> words ;
    char * x = GetAllWords(sql,words) ;
    tName = words.at(1) ;
    Trim(tName) ;
    for(int i = 0 ; i < tables.size() ; i++)
    {
        if(tName == tables.at(i)->GetTableName())
        {
            table = tables.at(i) ;
            return true ;
        }
    }
    delete [] x ;
    return false ;
}
void DBMS::DescribeTable(Table* table)
{
    table->Describe() ;
}
bool DBMS::SQLInsert_Analysize(const string sql,string & attrName,string & attrVal,Table * & table)
{
    table = NULL ;
    string tName ;
    vector<string> words ;
    char * x = GetAllWords(sql,words) ;
    try
    {
        if(words.at(1) != "INTO")
        {
            Error("key word 'INTO' not found ") ;
            delete [] x ;
            return false ;
        }
        tName = words.at(2) ;
        Trim(tName) ;
        for(int i = 0 ; i < tables.size() ; i++)
        {
            if(tName == tables.at(i)->GetTableName())
            {
                table = tables.at(i) ;
                break ;
            }
        }
        if(table == NULL)
        {
            Error("No such table named '"+tName+"'") ;
            delete [] x ;
            return false ;
        }
        //get attrName string
        int name_hpos = sql.find("(") ;
        int values_pos = sql.find("VALUES") ;
        if(values_pos == string::npos)
        {
            Error("key word 'VALUES' not found !") ;
            delete [] x ;
            return false ;
        }
        if(name_hpos > values_pos)//no attrName
        {
            attrName = "*" ;
        }
        else
        {
            int name_rpos ;
            for(name_rpos = name_hpos ; name_rpos < sql.length() ; name_rpos++)
            {
                if(sql[name_rpos] == ')')
                    break ;
            }
            if(name_rpos >values_pos)
            {
                Error("SQL brackets error") ;
                delete [] x ;
                return false ;
            }
            attrName = sql.substr(name_hpos+1,name_rpos-name_hpos-1) ;
        }
        //get attrVal
        //!!now ,wo should use the oriSQL
        //!!first , 再复制一份，全部装换为大写，用于获取位置，c++没有忽略大小写查找的函数
        string tmpSql = oriSQL ;
        transform(tmpSql.begin(),tmpSql.end(),tmpSql.begin(),::toupper) ;
        int val_hpos ;
        values_pos = tmpSql.find("VALUES") ;
        for(val_hpos = values_pos+6; val_hpos < sql.length() ; val_hpos++)
        {
            if(oriSQL[val_hpos] == '(')
            {
                break ;
            }
        }
        int val_rpos = oriSQL.rfind(')') ;
        attrVal = oriSQL.substr(val_hpos +1,val_rpos-val_hpos -1) ;
        delete [] x ;
    }
    catch(exception& x)
    {
        throw x ;
        return false ;
    }
    return true ;
}
bool DBMS::InsertTable(string attrName,string attrVal,Table * table)
{
    //define 2 vectors to store the attribute name and attribute value
    vector<string> names ;
    vector<string> values ;
    char * split = "," ;
    char * pNames = NULL ;
    if(attrName != "*")
    {
        pNames = Split(attrName,split,names) ;
    }
    char * pValues = Split(attrVal,split,values) ;
    if(pNames == NULL)//that is attrName == "*"
    {
        if(values.size() != table->getFieldNum())
        {
            string errorInfo = "Table '"+formatStr(table->GetTableName())+"' has been inserted with " ;
            char intbuf[10] ;
            itoa(values.size(),intbuf,10) ;
            errorInfo+=intbuf;
            errorInfo+=" column values, it requires exactly ";
            itoa(table->getFieldNum(),intbuf,10) ;
            errorInfo+=intbuf ;
            errorInfo+=" column values." ;
            Error(errorInfo) ;
            return false ;
        }
    }
    else
    {
        if(values.size() != names.size())
        {
            string errorInfo = "SQL wants to inserted with " ;
            char intbuf[10] ;
            itoa(names.size(),intbuf,10) ;
            errorInfo+=intbuf;
            errorInfo+=" column , but it gives " ;
            itoa(values.size(),intbuf,10) ;
            errorInfo+= intbuf ;
            errorInfo+=" values." ;
            Error(errorInfo) ;
            return false ;
        }
    }
    return(table->Insert(names,values)) ;
}
bool DBMS::IsSupDataType(const string dataType)
{
    int typeNum = 3 ;
    string types[] =
    {
        "INT",
        "CHAR",
        "VARCHAR"
    } ;
    for(int i = 0 ; i < typeNum ; i++)
    {
        if(dataType == types[i])
        {
            return true ;
        }
    }
    return false ;

}
bool DBMS::SQLShow_Analysize(const string sql)
{
    vector<string> words ;
    char * x = GetAllWords(sql,words) ;
    if(words.size() < 2)
    {
        Error("'Show' command error") ; ;
        delete [] x ;
        return false ;
    }
    string tmp = words.at(1) ;
    Trim(tmp) ;
    if(words.size() != 2 || tmp != "TABLES")
    {
        Error("'Show' command error") ; ;
        delete [] x ;
        return false ;
    }
    return true ;

}
bool DBMS::SQLDisplay_Analysize(const string sql,Table * & table)
{
    vector<string> words ;
    char * x = GetAllWords(sql,words) ;
    if(words.size() != 2)
    {
        Error("Display should just have a table name ") ;
        return false ;
    }
    else
    {
        string tableName = words.at(1) ;
        Trim(tableName) ;
        for(int i = 0 ; i < tables.size() ; i++)
        {
            if(tables.at(i)->GetTableName() == tableName)
            {
                table = tables.at(i) ;
                return true ;
            }
        }
        Error("No table named '"+tableName+"'") ;
        return false ;
    }
}
bool DBMS::SQLSelect_Analysize(const string sql,vector<Table *>& tabs,vector<Condition *>& con,vector<string>& projection)
{
    //con may be empty,projection may be empty
    if(sql.find("FROM") == string::npos)
    {
        Error("'From' key word not found") ;
        return false ;
    }
    vector <string> words ;
    char * x = GetAllWords(sql,words) ;
    bool getPro = true ;
    bool getTable = false ;
    bool getCon = false ;
    for(int i = 1 ; i < words.size() ; i++)
    {
        string tmp = words.at(i) ;
        Trim(tmp) ;
        if(tmp == "FROM")
        {
            getPro = false ;
            getTable = true ;
        }
        else if(tmp == "WHERE")
        {
            getTable = false ;
            getCon = true ;
        }
        else
        {
            if(getPro)
            {
                if(tmp != "*" )
                {
                    projection.push_back(tmp) ;
                }
            }
            else if(getTable)
            {
                bool isExist = false ;
                for(int j = 0 ; j < tables.size() ; j++)
                {
                    if(tables.at(j)->GetTableName() == tmp)
                    {
                        tabs.push_back(tables.at(j)) ;
                        isExist = true ;
                        break ;
                    }
                }
                //not found
                if(!isExist)
                {
                    Error("No such table named '"+tmp+"' ") ;
                    delete [] x ;
                    return false ;
                }

            }
            else if(getCon)
            {
                int hpos = sql.find("WHERE") + strlen("WHERE") ;
                //start get condition
                string buf = "" ;
                int len = sql.length() -1 ;//the last char is ';'
                bool theFirst = true ; //第一组的mulope默认为and
                for(int i = hpos ; i < len ; i++)
                {
                    if(oriSQL[i] == ' ')
                        continue ;
                    //每次读取一组
                    Condition * condition = new Condition ;
                    if(theFirst)
                    {
                        condition->mulope = "AND" ;
                        theFirst = false ;
                    }
                    else
                    {
                        for(; i < len ; i++)
                        {
                            if(oriSQL[i] != ' ')
                            {
                                buf+=sql[i] ;
                            }
                            else
                            {
                                condition->mulope = buf ;
                                buf = "" ;
                                break ;
                            }
                        }
                    }
                    //attrName
                    for( ; i < len && oriSQL[i] == ' '; i++) ;//ignore space

                    for(; i < len ; i++)
                    {
                        char tmp = oriSQL[i] ;
                        if(tmp != ' ' && tmp != '=' && tmp != '>' && tmp != '<')
                        {
                            buf+=sql[i] ;
                        }
                        else
                        {
                            condition->attrName = buf ;
                            buf = "" ;
                            break ;
                        }
                    }
                    //cmpope
                    for( ; i < len  && oriSQL[i] == ' ' ; i++ ) ; //ignore space

                    for( ; i < len ; i++)
                    {
                        if(oriSQL[i] == '='|| oriSQL[i] == '>' || oriSQL[i] == '<')
                        {
                            buf+=oriSQL[i] ;
                        }
                        else
                        {
                            condition->cmpope = buf ;
                            buf = "" ;
                            break ;
                        }
                    }
                    //val
                    for( ; i < len && oriSQL[i] == ' ' ; i++) ;
                    char tmp ;
                    tmp = oriSQL[i] ;
                    if(tmp == '"')
                    {
                        //it's str
                        hpos = i+1 ;
                        for(i++ ; i < len && oriSQL[i] != '"' ; i++) ;

                        int rpos = i ;
                        condition->val = oriSQL.substr(hpos,rpos-hpos) ;
                    }
                    else if(tmp == '\'')
                    {
                        hpos = i+1 ;
                        for( i++ ; i < len && oriSQL[i] != '\'' ; i++) ;

                        int rpos = i ;
                        condition->val = oriSQL.substr(hpos,rpos-hpos) ;
                    }
                    else
                    {
                        //it is not str
                        for( ; i < len ; i++)
                        {
                            if(oriSQL[i] != ' ')
                            {
                                buf+=oriSQL[i] ;
                            }
                            else
                            {
                                condition->val = buf ;
                                buf ="" ;
                                break ;
                            }
                        }
                        //可能是由于i = len而跳出循环，这时val值未保存
                        if(i == len)
                        {
                            condition->val = buf ;
                            buf ="" ;
                        }

                    }
                    con.push_back(condition) ;
                }
                break ;
            }
        }

    }
    delete [] x ;
    return true ;
}
bool DBMS::SelectTable(vector<Table*>& tabs,vector<Condition *>& con,vector<string>& projection)
{
    if(tabs.size() == 1)
    {
        return tabs.at(0)->Select(con,projection) ;
    }
    else
    return false ;
}
void DBMS::DisplayTable(Table* table)
{
    table->Display() ;
}
void DBMS::ShowTables()
{
    if(tables.size() == 0)
    {
        cout <<"\nEmpty set\n" ;
    }
    else
    {
        int len = 30 ;
        //like mysql
        cout << '+' ;
        for(int i = 1 ; i < len -1 ; i++)
            cout <<'-' ;
        cout <<'+' <<"\n" ;
        cout <<'|' ;
        int whiteLen = (len - dbName.length() -2 )/2 ;
        for(int i =0 ; i < whiteLen ; i++)
            cout <<' ' ;
        cout << dbName ;
        for(int i = 0 ; i < whiteLen ; i++)
            cout <<' ' ;
        cout <<'|' <<endl ;
        cout  <<"+" ;
        for(int i = 1 ; i < len -1 ; i++)
            cout <<'-' ;
        cout <<"+" <<endl ;
        for(int i = 0 ; i < tables.size() ; i++)
        {
            cout <<'|' <<"    " ;//4 brackets
            string tableName = tables.at(i)->GetTableName() ;
            cout <<left<<setw(len -2- 4)<<formatStr(tableName) ;
            cout <<"|\n" ;
        }
        cout  <<"+" ;
        for(int i = 1 ; i < len -1 ; i++)
            cout <<'-' ;
        cout <<"+" <<endl ;
        cout <<tables.size() <<" tables in DBMS\n" ;
    }
}
#endif // DBMS_H_INCLUDED
