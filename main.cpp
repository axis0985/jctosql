#include <stdio.h>  // for printf
#include <SQLAPI.h> // main SQLAPI++ header
#include <iostream>
#include <fstream>
#include <string.h>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;
int main(int argc, char* argv[])
{
    SAConnection con; // create connection object
    SACommand cmd;
    string insert_proto("insert into ");
    try
    {
        // connect to database
        // in this example it is Oracle,
        // but can also be Sybase, Informix, DB2
        // SQLServer, InterBase, SQLBase and ODBC
        ios_base::sync_with_stdio(false);
        fstream file;
        char db[50],uid[50],pw[64],table[64];
        char filename[128];
        string sqlcmd;

        if(argc>=2)
        {
            strcpy(filename,argv[1]);
            file.open(argv[1],ios::in);
        } else {
            char tmp[128];
            cin.getline(tmp,128);
            file.open(tmp,ios::in);
            strcpy(filename,tmp);
        }

        cout << "Enter database name: ";
        cin.getline(db,50);
        cout << "Table name: " ;
        cin.getline(table,64);
        // CSV
        if(file && !strcmp((filename + strlen(filename) - 4),".csv"))
        {
            string o_values;
            string column;
            getline(file,column);
            int i = 0;
            sqlcmd = insert_proto
                    + table
                    + " ("
                    + column
                    + ") values(";
            while(!file.eof()){
                getline(file,o_values);
                if(o_values.empty())
                    break;
                char *value ;
                value = strtok((char*)o_values.c_str(),",");
                while(value != NULL)
                {
                    sqlcmd += "\"" + string(value) + "\"";
                    value = strtok(NULL,",");
                    if(value != NULL)sqlcmd += ",";
                }
                if(!file.eof()&& o_values.empty()) sqlcmd += string("),(");
            }
            sqlcmd += ")" ;
            cout<< sqlcmd <<endl;
        } else if(file && !strcmp((filename + strlen(filename) - 4),"json"))
        { //JSON
            string jsonc;
            while(!file.eof()){
                string tmp;
                getline(file,tmp);
                jsonc += tmp;
            }
            json j = json::parse(jsonc);
            sqlcmd = insert_proto
                    + table
                    + " (";
            for (json::iterator it = j.begin(); it != j.end();) {
                 sqlcmd += it.key();
                 if(++it != j.end()) sqlcmd += ",";
            }
            sqlcmd += ") values(";
            for (json::iterator it = j.begin(); it != j.end();) {
                    string v = it.value();
                 sqlcmd += "\"" + v + "\"";
                 if(++it != j.end()) sqlcmd += ",";
            }
            sqlcmd += ")";
            cout << sqlcmd << endl;
        }
        cout << "Enter username: ";
        cin.getline(uid,50);
        cout << "Enter password for " << uid << "@" << db <<":";
        cin.getline(pw,64);
        con.Connect(
            db,     // database name
            uid,   // user name
            pw,   // password
            SA_MySQL_Client);    //client name
        cmd.setConnection(&con);
        cmd.setCommandText((char*)sqlcmd.c_str());
        cmd.Execute();
        while(cmd.FetchNext())
            printf("%s ",(const char*)cmd.Field(1).asString());
        // Disconnect is optional
        // autodisconnect will ocur in destructor if needed
        file.close();
        con.Disconnect();
        cout << "Imported " << filename << "to " << db << '.' << table;
    }
    catch(SAException &x)
    {
        // SAConnection::Rollback()
        // can also throw an exception
        // (if a network error for example),
        // we will be readyprintf("We are connected!\n");
        try
        {
            // on error rollback changes
            con.Rollback();
        }
        catch(SAException &)
        {
        }
        // print error message
        printf("%s\n", (const char*)x.ErrText());
    }
     return 0;
}
