#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <fstream>
#include <vector>
#include <unistd.h>

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

std::vector<std::string> split(const char *str, char c = ' '){
    std::vector<std::string> result;
    std::string tmp;
    do{
        const char *begin = str;
        while(*str != c && *str)
            str++;
        result.push_back(trim(std::string(begin, str)));
    }while(0 != *str++);
    return result;
}

void parse_command(std::string &command, std::string &name, std::string &argc)
{
    std::string::size_type pos = command.find_first_of(' ');
    if(pos != std::string::npos) {
        name = command.substr(0, pos);
        argc = command.substr(pos + 1, command.length() - pos - 1);
    } else {
        name = command;
        argc = "";
    }
}

int spawn_proc (int in, int out, std:: string &cmd){
    pid_t pid;

    if ((pid = fork ()) == 0){
        if (in != 0)
        {
            dup2 (in, 0);
            close (in);
        }

        if (out != 1)
        {
            dup2 (out, 1);
            close (out);
        }

        std::string name, args;
        parse_command(cmd, name, args);
        execlp(name.c_str(), name.c_str(), args.c_str(), NULL);
    }

  return pid;
}

std::string fork_pipes(std::string &cmd){
    std::vector<std::string> v;
    v = split(cmd.c_str(), '|');

    pid_t pid;
    int in, fpd[2];

    in = 0;

    for(int i = 0; i < v.size(); i++) {
        //std::cout << "cmd = " << v[i] << "\n";
        pipe (fpd);
        spawn_proc (in, fpd[1], v[i]);
        close (fpd [1]);
        in = fpd[0];
    }
    //std::cout << "end cmd" << "\n";
    std::string result;
    if(in != 0){
        char readbuffer[80];
        memset(&readbuffer, '\0', sizeof(readbuffer));
        while(read(in, readbuffer, sizeof(readbuffer))>0){
            result += readbuffer;
        }
    }
    return result;
}

int main(){
    std::string cmd;
    //std::getline(std::cin, cmd);
    //cmd = "echo 1";
    cmd = "ps aux | grep root | grep sbin";


    /* Read in a string from the pipe */
    std::string result = fork_pipes(cmd);;
    std::cout << "res = " << result << "n";

    std::ofstream out("result.out");
    out << result;
    out.close();
}
