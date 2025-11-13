#include "uart.h"


int openSerialPort(const std::string& portname) {
    int fd = open(portname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cerr << "Error opening " << portname << ": " << strerror(errno) << std::endl;
        return -1;
    }
    return fd;
}

bool configureSerialPort(int fd, int speed) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) { // Чтение из файлового дискриптора в структуру
        std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
        return false;
    }

    cfsetospeed(&tty, speed); // Установка скорости (выход) 
    cfsetispeed(&tty, speed); // Установка скорости (вход)

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // Установка размера символа в 8 бит (сначала & с маской, потом | с нужным значениям)
    tty.c_iflag &= ~IGNBRK; // Игнорирование синала прерывания
    tty.c_lflag = 0; // хз
    tty.c_oflag = 0; // хз
    
    // Важно: устанавливаем неблокирующее чтение
    tty.c_cc[VMIN]  = 0;    // Не ждём минимального количества байт
    tty.c_cc[VTIME] = 1;    // Таймаут 0.1 секунды между байтами

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) { // Запись настроек в файл , TCSANOW - записать немедленно
        std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Очистка буферов
    tcflush(fd, TCIOFLUSH);
    
    return true;
}


int sendCommand(int fd, const std::string& command) {
    std::string cmd = command + "\n";
    int written = write(fd, cmd.c_str(), cmd.length());
    tcdrain(fd);  // Ждём, пока данные отправятся
    return written;
}


std::vector<std::string> readResponse(int fd, double timeout_sec) {
    char buffer[256];
    std::string response = "";
    int n;
    int no_data_count = 0;
    const int max_no_data = timeout_sec * 10;  // 10 попыток в секунду
    
    // Даём команде время на выполнение
    usleep(500000);  // 500 мс задержка перед началом чтения
    
    while (no_data_count < max_no_data) {
        n = read(fd, buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            buffer[n] = '\0';
            response += buffer;
            no_data_count = 0;  // Сбрасываем счётчик, если получили данные
            
            // Продолжаем читать если данные всё ещё приходят
        } 
        else if (n == 0) {
            // Нет данных, увеличиваем счётчик
            no_data_count++;
            // Если уже что-то получили и данных больше нет 0.1 секунды подряд
            if (response != "" && no_data_count > 1) break;
        } 
        else {
            // Ошибка чтения
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                break;
            }
        }
        
        usleep(100000);  // 100 мс между попытками чтения
    }
    if(response.size() == 0) return {""};
    response = std::string(response.begin(), response.end()-1);
    return split(response, "\n");
}


int uart_init(const std::string& portname, int br){
    int fd = openSerialPort(portname);
    if (fd < 0) return -1;
    if (!configureSerialPort(fd, br)) {
        close(fd);
        return -1;
    }
    std::cout << "Connected to " << portname << std::endl;
    // Сначала отправляем Enter, чтобы "проснуться"
    //write(fd, "\n", 1);
    usleep(500000);
    // Очищаем буфер от старых данных
    tcflush(fd, TCIFLUSH);
    return fd;
}

int get_baudrate(int baudrate) {
    switch(baudrate) {
        case 0:      return B0;
        case 50:     return B50;
        case 75:     return B75;
        case 110:    return B110;
        case 134:    return B134;
        case 150:    return B150;
        case 200:    return B200;
        case 300:    return B300;
        case 600:    return B600;
        case 1200:   return B1200;
        case 1800:   return B1800;
        case 2400:   return B2400;
        case 4800:   return B4800;
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        default:
            return B115200; 
    }
}
