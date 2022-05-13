#define  _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <iostream>
#pragma execution_character_set("gb2312")
using namespace std;

int process_file(char *path);

int main()
{
    printf("current path: ");
    system("chdir");
    printf("\n\n");
    char dir[256] = {0};
    FILE *pp = _popen("dir /s /b *.c *.h", "r");
    while(fgets(dir, sizeof(dir), pp))
    {
        dir[strlen(dir) - 1] = 0;
        process_file(dir);
        memset(dir, 0, sizeof(dir));
    }

	printf("app end\n");
    _pclose(pp);
    cin.get();
    
    return 0;
}

//file: The absolute path to the file
int process_file(char * file)
{
    //open file
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
    {
        cout << "open: " << file;
        cout << " error\n";
        return 0;
    }

    //get file size
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(size == 0)
    {
        cout << "file size = 0\n";
        return 0;
    }

    cout << "open file: " << file << " success\n";
    cout << "file size: " << size << endl;

    char *f_data = (char *)malloc(size + 1);//存放整个文件
    char* l_data = (char *)malloc(size + 1);//存放一行的数据
    if(f_data == NULL || l_data == NULL)
    {
        cout << "Failed to create memory!!\n";
        return 0;
    }
    memset(f_data, 0, size + 1);
    memset(l_data, 0, size + 1);
    int len = 0;
    while(fgets(l_data, size, fp) != NULL)
    {
        len = strlen(l_data);
        if(len >= 1)
            l_data[len - 1] = 0;
        
        while(len --)
        {
            // clear tab、space
            if((l_data[len] != 0x20) && (l_data[len] != 0) && (l_data[len] != 0x9))
            {
                if(strlen(l_data) != 0)
                    strcat(f_data, l_data);
                break;
            }
            else
                l_data[len] = 0;
        }
        f_data[strlen(f_data)] = '\n';
    }
    fclose(fp);
    fp = fopen(file, "w");
    if(fp != NULL)
    {
        cout << "write file success\n\n";
        fwrite(f_data, strlen(f_data), 1, fp);
        fclose(fp);
    }
    else
    {
        cout << "open write file: ";
        cout << file << " error!\n";
        cout << strerror(errno) << "\n\n";
    }
    free(l_data);
    free(f_data);
    return 1;
}