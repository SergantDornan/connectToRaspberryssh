#include "uart.h"
#include "alias.h"
#include "Flags.h"
#include "algs.h"

// sshpass, openssh-server, openssh-client

void help(){
    std::cout << "Usage: " << std::endl;
    std::cout << "-wifi <1/0> - use existing wifi" << std::endl;
    std::cout << "-wn <wifi name> - wifi network" << std::endl;
    std::cout << "-wp <wifi password> - wifi password" << std::endl;
    std::cout << "-p <portname> - set port name, default port name is /dev/ttyUSB0" << std::endl;
    std::cout << "-b <baud rate> - set baud rate, default baud rate is 115200" << std::endl;
    std::cout << "-login <login> - set raspberry login" << std::endl;
    std::cout << "-password <password> - set raspberry password" << std::endl;
    std::cout << std::endl;
}

const std::string defaultLogin = "sergantdornan";
const std::string defaultPassword = "UbermenschenAmogus228";
const std::string name = "setssh_to_raspberry";
const std::string connect = "connect.sh";
const std::string hotspotSSID = "Malina";
const std::string hotstopPassword = "UberPass228";
const std::string wifi_config = "wifi_config";

int main(int argc, char* argv[]) {


    std::vector<std::string> args;
    for(int i = 1; i < argc; ++i) args.push_back(argv[i]);
    if(manage_alias(name, args) == 0) {
        auto self = getPathByAlias(name);
        if(self.size() == 0) return 0;
        auto belder = getPathByAlias("belder");
        if(belder.size() == 0) {
            std::cout << "================ ERROR ================" << std::endl;
            std::cout << "cannot find belder" << std::endl;
            std::cout << std::endl;
            return 1;
        }
        std::string cmd = belder[0] + " install.cpp -o install run -c " + getFolder(self[0]);
        system(cmd.c_str());
        return 0;
    }
    if(args.size() != 0 && args[0] == "help"){
        help();
        return 0;
    }

    auto parameters = getParameters(args, {"-b", "-p", "-login", "-password", "-wn", "-wp", "-wifi"});

    bool use_real_wifi = (parameters[6] == "1");
    int baud_rate = (parameters[0] == "-1") ? get_baudrate(115200) : get_baudrate(stoi(parameters[0]));
    std::string portname = (parameters[1] == "-1") ? "/dev/ttyUSB0" : parameters[1];
    std::string login = (parameters[2] == "-1") ?  defaultLogin : parameters[2];
    std::string password = (parameters[3] == "-1") ? defaultPassword : parameters[3]; 

    if(use_real_wifi){
        std::vector<std::string> lines;
        std::string line;
        std::ifstream in(wifi_config);
        while(std::getline(in, line)) lines.push_back(line);
        in.close();
        if((lines[0] == "-1" || lines[1] == "-1") && 
            (parameters[4] == "-1" || parameters[5] == "-1"))
        {
            std::cout << "Enter wifi name and password" << std::endl;
            std::cout << std::endl;
            return 1;
        }        

        if(parameters[4] != "-1") lines[0] = parameters[4];
        if(parameters[5] != "-1") lines[1] = parameters[5];

        if(parameters[4] != "-1" || parameters[5] != "-1"){
            std::ofstream out(wifi_config);
            out << lines[0] << std::endl;
            out << lines[1] << std::endl;
            out.close();
        }

        parameters[4] = lines[0];
        parameters[5] = lines[1];
    }

    if(!exists(portname)){
        std::cout << "Cannot find " << portname << std::endl;
        std::vector<std::string> options;
        auto dirs = getDirs("/dev");
        for(int i = 1; i < dirs.size(); ++i){
            if(dirs[i].find("ttyUSB") != std::string::npos) options.push_back(dirs[i]);
        }
        if(options.size() == 0){
            std::cout << "Connect UART-USB" << std::endl;
            std::cout << std::endl;
            return 1;
        }
        std::cout << "Here are the options: " << std::endl;
        std::cout << std::endl;
        for(int i = 0; i < options.size(); ++i) std::cout << options[i] << std::endl;
        std::cout << std::endl;
        
        std::cout << "Enter port name:" << std::endl;
        std::string newport;
        std::cin >> newport;
        std::cout << std::endl;
        if(newport.size() >= 10 && std::string(newport.begin(), newport.begin() + 10) == "dev/ttyUSB")
            newport = "/" + newport;
        else if(newport.size() >= 6 && std::string(newport.begin(), newport.begin() + 6) == "ttyUSB")
            newport = "/dev/" + newport;
        if(!exists(newport)){
            std::cout << "Cannot find " << newport << std::endl;
            std::cout << "Aborting" << std::endl;
            std::cout << std::endl;
            return 1;
        }
        portname = newport;
    }
    int fd = uart_init(portname, baud_rate);
    if(fd == -1) return 1;

    // login:
    sendCommand(fd, "ls");
    auto response = readResponse(fd, 0.2);

    if (response.size() == 2 && response[0] == "ls" && response[1] == "Password:"){
        std::cout << "Trying to login..." << std::endl; 
        sendCommand(fd, "");
        usleep(5000000);
        sendCommand(fd, login);
        usleep(5000000);
        response = readResponse(fd, 5);

        sendCommand(fd, password);
        usleep(5000000);
        response = readResponse(fd, 5);
        if(response.size() > 0 && response[0] == "Login incorrect"){
            std::cout << "Login incorrect" << std::endl;
            std::cout << std::endl;
            return 1;
        }
        std::cout << "Login success" << std::endl;
    }
    else std::cout << "Loged in" << std::endl;

    std::cout << "Turning wifi on..." << std::endl;
    std::string cmd = "sudo nmcli radio wifi on";
    sendCommand(fd, cmd);
    response = readResponse(fd, 0.2);

    bool needAuth = false;
    for(int i = 0; i < response.size(); ++i){
        if(response[i].find("[sudo] password for " + login + ":") != std::string::npos){
            needAuth = true;
            break;
        }
    }
    if(needAuth){
        sendCommand(fd, password);
        response = readResponse(fd, 0.2);
    }
    usleep(5000000);
    if(use_real_wifi){
        std::cout << "Conncecting to " << parameters[4] << " newtwork..." << std::endl;
        cmd = "sudo nmcli d wifi connect " + parameters[4] + " password " + parameters[5];
        sendCommand(fd, cmd);
        response = readResponse(fd, 0.2);

        for(int i = 0; i < response.size(); ++i){
            if(response[i].find("Error: No network with SSID") != std::string::npos){
                std::cout << "Error: No network with SSID " << parameters[4] << " found" << std::endl;
                std::cout << std::endl;
                return 1;
            }
        }
    }
    else{
        std::cout << "Activating hotspot..." << std::endl;
        cmd = "sudo nmcli device wifi hotspot ifname wlan0 ssid " + hotspotSSID + " password " + hotstopPassword;
        sendCommand(fd, cmd);
        response = readResponse(fd, 0.2);
        bool hotspotactive = false;
        for(int i = 0; i < response.size(); ++i){
            if(response[i].find("Device 'wlan0' successfully activated with")  != std::string::npos){
                hotspotactive = true;
                break;
            }
        }
        //if(hotspotactive) std::cout << "Hotspot active" << std::endl;
        //else {
        //    std::cout << "Error activating hotspot" << std::endl;
        //    return 1;
        // }
    }
    

    while (true){
        sendCommand(fd, "hostname -I");
        response = readResponse(fd, 0.2);
        bool b = false;
        for(int i = 0; i < response.size(); ++i){
            if(response[i].find("Device 'wlan0' successfully activated with") != std::string::npos){
                std::cout << "SUCCESS" << std::endl;
                b = true;
                break;
            }
        }
        if(b) break;
    }
    
    sendCommand(fd, "hostname -I");
    response = readResponse(fd, 0.2);

    std::string ip = (response.size() >= 3) ? split(response[2])[0] : "10.42.0.1";
    std::cout << "IP: " << ip << std::endl;

    std::cout << "Turning wifi on pc on..." << std::endl;
    int code = system("sudo nmcli radio wifi on");
    if(code != 0) return 1;
    usleep(5000000);
    std::cout << "Conncecting pc to " << ((use_real_wifi) ? parameters[4] : "raspberry") << " network..." << std::endl;
    cmd = "sudo nmcli d wifi connect \"" + ((use_real_wifi) ? parameters[4] : hotspotSSID) + "\" password \"" + ((use_real_wifi) ? parameters[5] : hotstopPassword) + "\"";
    code = system(cmd.c_str());
    if(code != 0) return 1;
    close(fd);
    cmd = "sshpass -p '" + password + "' ssh -tt -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null " + login + "@" + ip;
    std::ofstream out(connect);
    out << "#!/bin/bash\nset -e" << std::endl;
    out << cmd << std::endl;
    out.close();
    std::cout << std::endl;
    std::cout << "RUN THIS:" << std::endl;
    std::cout << connect << std::endl;
    return 0;
}
