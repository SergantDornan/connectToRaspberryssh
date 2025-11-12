#include "filework.h"
#include "alias.h"


const std::string name = "setssh_to_raspberry";

int main(int argc, char* argv[]){

    auto belder = getPathByAlias("belder");
    if(belder.size() == 0) {
            std::cout << "================ ERROR ================" << std::endl;
            std::cout << "cannot find belder" << std::endl;
            std::cout << std::endl;
            return 1;
        }
	const std::string oldline = "const std::string connect = \"connect.sh\";";
    const std::string newline = "const std::string connect = \"" + cwd() + "/connect.sh\";";
    const std::string mainFile = "main.cpp";
    rewriteLine(mainFile, oldline, newline);
    std::string cmd = belder[0] + " main.cpp -o " + name;
    system(cmd.c_str());
    rewriteLine(mainFile, newline, oldline);
    auto self = getPathByAlias(name);
    if(self.size() == 0) {
        manage_alias(name, {"add_alias"});
        self = getPathByAlias(name);
        if(self.size() == 0) {
            std::cout << "ERROR, cannot find self" << std::endl;
            return 1;
        }
    }
    cmd = belder[0] + " install.cpp -o install -c " + getFolder(self[0]);
    system(cmd.c_str());
    return 0;
}