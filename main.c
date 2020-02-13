#include "st7735.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAXBUFSIZE 1024
#define WAIT_MICRO_SECOND 1000

char info_str[1024];
char dis_buf[26];

typedef struct      
{  
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
} CPU_OCCUPY;

typedef struct PACKED         
{
    char name[20];
    long total;
    char name2[20];
    long free;
}MEM_OCCUPY;

float g_cpu_used;           
int cpu_num;                //����һ��ȫ�ֵ�int����cup_num  
void cal_occupy(CPU_OCCUPY *, CPU_OCCUPY *); 
void get_occupy(CPU_OCCUPY *); 
void get_mem_occupy(MEM_OCCUPY *) ;
float get_io_occupy();
void get_disk_occupy(char ** reused);
int get_netway_ip_using_res(char *str_ip);

CPU_OCCUPY ocpu,ncpu;
MEM_OCCUPY mem;
char ip[128];

int main(void)
{
    st7735_cfg.channel = SPI_CHANNEL;
    st7735_cfg.speed = SPI_SPEED;
    st7735_cfg.displayMode = LCD_HORIZONTAL;
    
    if(LCD_Init(&st7735_cfg) != 0)
    {
        printf("\n\nSYS ERROR\n\n");
        return -1;
    }
    LCD_ClearScreen(BLACK);
    while(1)
    {
        FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        fgets(info_str, 6, fp);
        sprintf(dis_buf, "cpu temp:%04.2f C", (float)atoi(info_str)/1000);
        LCD_ConvertASCIIToBuffer(0, (uint8_t*)dis_buf, GREEN, BLACK);
        LCD_DisplayBuffer();
        fclose(fp);

        get_occupy(&ocpu);
        delay(WAIT_MICRO_SECOND);
        get_occupy(&ncpu);
        cal_occupy(&ocpu, &ncpu);
        sprintf(dis_buf, "cpu used:%4.2f%%", g_cpu_used);
        LCD_ConvertASCIIToBuffer(1, (uint8_t*)dis_buf, GREEN, BLACK);
        
        get_mem_occupy(&mem);
        double using = ((double)(mem.total - mem.free)/mem.total)*100;
        sprintf(dis_buf, "mem used:%4.2f%%",using);
        LCD_ConvertASCIIToBuffer(2, (uint8_t*)dis_buf, GREEN, BLACK);
        
        char t[20]="";
        char *used = t;
        get_disk_occupy(&used);
        sprintf(dis_buf, "disk used:%s", used);
        LCD_ConvertASCIIToBuffer(3, (uint8_t*)dis_buf, GREEN, BLACK);
        
        if (get_netway_ip_using_res(ip) != -1)
        {
            sprintf(dis_buf, "IP:%s", ip);
            LCD_ConvertASCIIToBuffer(4, (uint8_t*)dis_buf, GREEN, BLACK);
        }
        else
        {
            LCD_ConvertASCIIToBuffer(4, (uint8_t*)"Failed to obtain IP address", GREEN, BLACK);
        }
    }
    return 0;
}


void  cal_occupy (CPU_OCCUPY *o, CPU_OCCUPY *n)
{  
    double od, nd;  
    double id, sd;  
    od = (double) (o->user + o->nice + o->system +o->idle);//��һ��(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od  
    nd = (double) (n->user + n->nice + n->system +n->idle);//�ڶ���(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od  
    id = (double) (n->user - o->user);    //�û���һ�κ͵ڶ��ε�ʱ��֮���ٸ���id  
    sd = (double) (n->system - o->system);//ϵͳ��һ�κ͵ڶ��ε�ʱ��֮���ٸ���sd  
    g_cpu_used = ((sd+id)*100.0)/(nd-od); //((�û�+ϵͳ)��100)��(��һ�κ͵ڶ��ε�ʱ���)�ٸ���g_cpu_used  
}  
void  get_occupy (CPU_OCCUPY *o)
{  
    FILE *fd;            
    char buff[MAXBUFSIZE];                                                                                               
    fd = fopen ("/proc/stat", "r"); //����ֻ��ȡstat�ļ��ĵ�һ�м�cpu����Ϣ�������ȡÿ��cpu��ʹ������������stat�ļ��Ľ��������С�
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u %u %u %u", o->name, &o->user, &o->nice,&o->system, &o->idle);  
    fclose(fd);     
}  
void get_mem_occupy(MEM_OCCUPY * mem)
{
    FILE * fd;
    char buff[MAXBUFSIZE];
    fd = fopen("/proc/meminfo","r");
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %ld", mem->name,&mem->total);  
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %ld", mem->name2,&mem->free); 
}
float get_io_occupy()
{
        char cmd[] ="iostat -d -x";
        char buffer[MAXBUFSIZE];  
        char a[20];   
        float arr[20];
        FILE* pipe = popen(cmd, "r");    
        if (!pipe)  return -1;    
        fgets(buffer, sizeof(buffer), pipe);
        fgets(buffer, sizeof(buffer), pipe);
        fgets(buffer, sizeof(buffer), pipe);
        fgets(buffer, sizeof(buffer), pipe);
        sscanf(buffer,"%s %f %f %f %f %f %f %f %f %f %f %f %f %f ",a,&arr[0],&arr[1],&arr[2],&arr[3],&arr[4],&arr[5],&arr[6],&arr[7],&arr[8],&arr[9],&arr[10],&arr[11],&arr[12]);
        //printf("%f\n",arr[12]);
        return arr[12];
        pclose(pipe);  
}
void get_disk_occupy(char** reused)
{
    char currentDirectoryPath[ MAXBUFSIZE ];
    getcwd(currentDirectoryPath, MAXBUFSIZE);
    //printf("��ǰĿ¼��%s\n",currentDirectoryPath);
    char cmd[50]="df ";
    strcat(cmd,currentDirectoryPath);
    //printf("%s\n",cmd);
    
    char buffer[MAXBUFSIZE];
    FILE* pipe = popen(cmd, "r");    
    char fileSys[20];
    char blocks[20];
    char used[20];
    char free[20];
    char percent[10];
    char moment[20];
    
    if (!pipe)  return ;  
    if(fgets(buffer, sizeof(buffer), pipe)!=NULL)
    {
        sscanf(buffer,"%s %s %s %s %s %s",fileSys,blocks,used,free,percent,moment);
    }
    if(fgets(buffer, sizeof(buffer), pipe)!=NULL)
    {
        sscanf(buffer,"%s %s %s %s %s %s",fileSys,blocks,used,free,percent,moment);
    }
    //printf("desk used:%s\n",percent);
    strcpy(*reused,percent);
    return ;
}

int get_netway_ip_using_res(char *str_ip)
{
    int sta = -1;
    char do_comment[] = "ifconfig | grep 'inet addr' | awk '{print $2}' | sed 's/.*://g'"; 
    //�������Ǵ�ifconfig����ȡ��Ӧ��IP
    FILE *fp = NULL;
    fp = popen(do_comment, "r");
    if(fp != NULL)
    {
        fgets(str_ip, 1024, fp);
        sta = 0;
        // if(strcmp("127.0.0.1", str_ip))
        // {
        //     break;
        // }
    }
    fclose(fp);
    return sta;
}
