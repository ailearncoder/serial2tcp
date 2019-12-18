#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pwd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#define MAX_SOCK_FD             FD_SETSIZE
#define FD_SET_SIZE             2
#define MAX_QUE_CONN_NM         2

static int atFd = -1;
static int socketFd = -1;
static int client_fd[FD_SET_SIZE];

/****************************************
    Function:     // SetSerialBaud
    Description:  // 设置串口波特率
    Input:        // fd: 串口的文件描述符
                     speed: 波特率
    Output:       // NULL
    Return:       // 
****************************************/
int SetSerialBaud(int fd, int speed)
{
    struct termios opt;
    tcgetattr(fd, &opt);
    cfsetispeed(&opt, speed);
    cfsetospeed(&opt, speed);
    return tcsetattr(fd, TCSANOW, &opt);
}

int GetBaud(int baud)
{
    if (baud == 0)
        return B0;
    if (baud == 50)
        return B50;
    if (baud == 75)
        return B75;
    if (baud == 110)
        return B110;
    if (baud == 134)
        return B134;
    if (baud == 150)
        return B150;
    if (baud == 200)
        return B200;
    if (baud == 300)
        return B300;
    if (baud == 600)
        return B600;
    if (baud == 1200)
        return B1200;
    if (baud == 1800)
        return B1800;
    if (baud == 2400)
        return B2400;
    if (baud == 4800)
        return B4800;
    if (baud == 9600)
        return B9600;
    if (baud == 19200)
        return B19200;
    if (baud == 38400)
        return B38400;
    if (baud == 57600)
        return B57600;
    if (baud == 115200)
        return B115200;
    if (baud == 230400)
        return B230400;
    return -1;
}

//设置任意波特率，比如28800
int SetSerialBaud2(int fd,int baud)
{
    struct serial_struct ss,ss_set;   
    struct termios opt;
    tcgetattr(fd, &opt);

    cfsetispeed(&opt,B115200);
    cfsetospeed(&opt,B115200);

    tcflush(fd,TCIFLUSH);/*handle unrecevie char*/
    tcsetattr(fd,TCSANOW,&opt);

    if((ioctl(fd,TIOCGSERIAL,&ss))<0){
        printf("BAUD: error to get the serial_struct info:%s\n",strerror(errno));
        return -1;
    }

    ss.flags = ASYNC_SPD_CUST;
    ss.custom_divisor = ss.baud_base / baud;

    if((ioctl(fd,TIOCSSERIAL,&ss))<0){
        printf("BAUD: error to set serial_struct:%s\n",strerror(errno));
        return -2;
    }

    ioctl(fd,TIOCGSERIAL,&ss_set);
    printf("BAUD: success set baud to %d,custom_divisor=%d,baud_base=%d\n",
            baud,ss_set.custom_divisor,ss_set.baud_base);
    return 0;
}

/****************************************
    Function:     // SetSerialRawMode
    Description:  // 设置串口模式
    Input:        // fd: 串口的文件描述符
    Output:       // NULL
    Return:       // 
****************************************/
int SetSerialRawMode(int fd)
{
    struct termios opt;
    tcgetattr(fd, &opt);
    opt.c_lflag         &= ~(ICANON | ECHO | ECHOE | ISIG); /* input */
    opt.c_iflag         &= ~(IXON | IXOFF | ICRNL | INLCR | IGNCR); /* 不将 CR(0D) 映射成 NR(0A) */
    opt.c_oflag         &= ~OPOST;                  /* output */
    return tcsetattr(fd, TCSANOW, &opt);
}

static int OpenSerial(char * devName, char * baud)
{
    int atFd;
    int mbaud = -1;
    int mbaud2 = -1;
  
    /* 标准波特率 */
    if(baud[0] == 'B')
    {
        mbaud = atoi(baud + 1);
        mbaud = GetBaud(mbaud);
        if(mbaud <= 0)
        {
            printf("Not Support Baud:%s\n",baud);
            return -1;
        }
    }
    else
    {
        mbaud2 = atoi(baud);
        if(mbaud2 <= 0)
        {
            printf("Not Support Baud:%s\n",baud);
            return -1;
        }            
    }
    
    if ((atFd = open(devName, O_RDWR)) < 0)
    {
        printf("open dev %s failed\n", devName);
		perror("");
        return - 1;
    }
  
    if(mbaud > 0 && SetSerialBaud(atFd, mbaud)<0)
	{
		close(atFd);
		perror("SetSerialBaud Faild\n");
		return -1;
	}
    
    if(mbaud2 > 0 && SetSerialBaud2(atFd, mbaud2)<0)
	{
		close(atFd);
		perror("SetSerialBaud Faild\n");
		return -1;
	}
    
    if(SetSerialRawMode(atFd)<0)
	{
		perror("SetSerialRawMode Faild\n");
		close(atFd);
		return -1;
	}
    return atFd;
}

void close_fd()
{
    int i;
	if(atFd != -1)
		close(atFd);
	if(socketFd != -1)
		close(socketFd);
    for(i = 0; i < FD_SET_SIZE; i++)
    {
        if(client_fd[i] != -1)
            close(client_fd[i]);
    }
	atFd = -1;
	socketFd = -1;
}

/*自定义信号处理函数*/
void my_func(int sign_no)
{
    if (sign_no == SIGINT)
    {
        printf("I have get SIGINT\n");
		close_fd();
    }
    else if (sign_no == SIGQUIT)
    {
        printf("I have get SIGQUIT\n");
		close_fd();
    }
}

void init_sign()
{
	struct sigaction action;

    /* sigaction 结构初始化 */
    action.sa_handler = my_func;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    /* 发出相应的信号，并跳转到信号处理函数处 */
    sigaction(SIGINT, &action, 0);
    sigaction(SIGQUIT, &action, 0);
}

int init_socket(int port)
{
	int     sockfd;
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(& (server_sockaddr.sin_zero), 8);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("init_socket");
        return -1;
    }
	if (bind(sockfd, (struct sockaddr *) &server_sockaddr, 
        sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        return -1;
    }

    if (listen(sockfd, MAX_QUE_CONN_NM) == -1)
    {
        perror("listen");
        return -1;
    }
    printf("listening port %d....\n", port);
	return sockfd;
}

void serial2tcp(char * buf, int len)
{
	int i,count;
	for (i = 0; i < FD_SET_SIZE; i++)
	{
		if (client_fd[i] != -1)
		{
			count = send(client_fd[i], buf, len, 0);
			if(count != len)
			{
				printf("%d send faild %d\n", i, count);
				perror("");
			}
		}
	}
}

int main(int argc, char * argv[])
{
	#define BUFFER_SIZE 1
	
	int i;
	char debug = 0;
	struct passwd *passwd;
	
	struct sockaddr_in client_sockaddr[FD_SET_SIZE], c_sockaddr;
	int fd;
	socklen_t sin_size;
	char    IPdotdec[20]; //存放点分十进制IP地址
	char buf[BUFFER_SIZE];
	fd_set  inset, tmp_inset;
	unsigned count;
	char * devName = "/dev/ttyS1";
	int port = 10010;
    char * baud = "B115200";
    if(argc == 2 && strcmp("-h", argv[1]) == 0)
    {
        printf("-------------------------help-------------------------\n");
        printf("ser2tcp -d dev_name -p port -b baud\n");
        printf("\t Options:\n");
        printf("\t         -d dev_name 指定设备节点，默认：/dev/ttyS1\n");
        printf("\t         -p port     指定端口，    默认：10010\n");
        printf("\t         -b baud     指定波特率，  默认：B115200\n");
        printf("------------------------------------------------------\n");
        return 0;
    }
	for(i = 1;i < argc;i++)
	{
        if(strcmp("debug", argv[i]) == 0)
        {
            debug = 1;
            i++;
            if(i >= argc)
                break;
        }
        if(strcmp("-d", argv[i]) == 0)
        {
            devName = argv[i + 1];
            i++;
            if(i >= argc)
                break;
        }
        if(strcmp("-p", argv[i]) == 0)
        {
            port = atoi(argv[i + 1]);
            if(port == 0)
            {
                port = 10010;
            }
            i++;
            if(i >= argc)
                break;
        }
        if(strcmp("-b", argv[i]) == 0)
        {
            baud = argv[i + 1];
            if(strlen(baud) == 0)
            {
                baud = "B115200";
            }
            i++;
            if(i >= argc)
                break;
        }
	}
    printf("open device:%s port:%d baud:%s\n", devName, port, baud);
    if(debug)
    {
        printf("Debug Model Enter\n");
    }
    passwd=getpwuid(getuid());
	if(strcmp(passwd->pw_name, "root"))
	{
		printf("need root permission!\n");
		return -1;
	}
	
	atFd = OpenSerial(devName, baud);
	/*将调用 socket()函数的描述符作为文件描述符*/
	if(atFd < 0)
		return -1;
	socketFd = init_socket(port);
	if(socketFd < 0)
		return -1;
	printf("connected atFd %d\n", atFd);
	FD_ZERO(&inset);
	FD_SET(atFd, &inset);
	FD_SET(socketFd, &inset);
	
	for (i = 0; i < FD_SET_SIZE; i++)
    {
        client_fd[i] = -1;
    }
	
	init_sign();
	
	while(1)
	{
		tmp_inset = inset;
		sin_size = sizeof(struct sockaddr_in);
        /*调用 select()函数*/
        if (! (select(MAX_SOCK_FD, &tmp_inset, NULL, NULL, NULL) > 0))
        {
            perror("select");
			close_fd();
			return -1;
        }
		if (FD_ISSET(atFd, &tmp_inset) > 0)
        {
            if ((count = read(atFd, buf, BUFFER_SIZE)) > 0)
            {
				if(debug)
					printf("ser read %03d-%02x-%c\n",buf[0], buf[0], buf[0]);
				serial2tcp(buf, BUFFER_SIZE);
            }
            else 
            {
				FD_CLR(atFd, &inset);
				printf("atFd read error\n");
				perror("\n");
				close_fd();
				return -1;
            }
        }
		if (FD_ISSET(socketFd, &tmp_inset) > 0)
        {
			if ((fd = accept(socketFd, (struct sockaddr *) &c_sockaddr, &sin_size)) == -1)
            {
                perror("accept");
				continue;
			}
			
			for (i = 0; i < FD_SET_SIZE; i++)
            {
                if (client_fd[i] < 0)
                {
                    client_fd[i] = fd;
                    memcpy(&client_sockaddr[i], &c_sockaddr, sin_size);
                    break;
                }
            }

            if (i == FD_SET_SIZE)
            {
                printf("连接数大于%d\n", FD_SET_SIZE);
                close(fd);
                continue;
            }

            FD_SET(fd, &inset);
            inet_ntop(AF_INET, (void *) &c_sockaddr.sin_addr, IPdotdec, 16);
            printf("New connection %d %s:%d from %d(socket)\n", i, IPdotdec, c_sockaddr.sin_port, 
                fd);
        }
		for (i = 0; i < FD_SET_SIZE; i++)
		{
			if (client_fd[i] != -1 && FD_ISSET(client_fd[i], &tmp_inset) > 0)
			{
				if ((count = recv(client_fd[i], buf, BUFFER_SIZE, 0)) > 0)
				{
					if(debug)
						printf("tcp read %03d-%02x-%c\n",buf[0], buf[0], buf[0]);
					if ((count = write(atFd, buf, BUFFER_SIZE)) != BUFFER_SIZE)
					{
						printf("%d write faild %d\n", i, count);
					}
				}
				else 
				{
					close(client_fd[i]);
					FD_CLR(client_fd[i], &inset);
					c_sockaddr = client_sockaddr[i];
					inet_ntop(AF_INET, (void *) &c_sockaddr.sin_addr, IPdotdec, 16);
					printf("断开连接 %d %s:%d from %d(socket)\n", i, IPdotdec, c_sockaddr.sin_port, 
						client_fd[i]);
					perror("\n");
					client_fd[i] = -1;
				}
			}
		}
	}
    return 0;
}

