#ifndef __FTP_CLIENT_H__
#define __FTP_CLIENT_H__

/*****************************************************************************
 ����  :   : Class FTPManager
 ��������  : FTP������
 ��    ��   : 2017��12��30��
 ��    ��   : shenqingzheng

*****************************************************************************/
//FTP��Ӧ��

//��Ӧ����	����˵��
//110	//���ļ�ָʾ���ϵ��������
//120	//������׼��������ʱ�䣨��������
//125	//���������ӣ���ʼ����
//150	//������
//200	//�ɹ�
//202	//����û��ִ��
//211	//ϵͳ״̬�ظ�
//212	//Ŀ¼״̬�ظ�
//213	//�ļ�״̬�ظ�
//214	//������Ϣ�ظ�
//215	//ϵͳ���ͻظ�
//220	//�������
//221	//�˳�����
//225	//����������
//226	//������������
//227	//���뱻��ģʽ��IP ��ַ��ID �˿ڣ�
//230	//��¼������
//250	//�ļ���Ϊ���
//257	//·��������
//331	//Ҫ������
//332	//Ҫ���ʺ�
//350	//�ļ���Ϊ��ͣ
//421	//����ر�
//425	//�޷�����������
//426	//��������
//450	//�ļ�������
//451	//�������ش���
//452	//���̿ռ䲻��
//500	//��Ч����
//501	//�������
//502	//����û��ִ��
//503	//����ָ������
//504	//��Ч�������
//530	//δ��¼����
//532	//�洢�ļ���Ҫ�ʺ�
//550	//�ļ�������
//551	//��֪����ҳ����
//552	//�����洢����
//553	//�ļ���������
#include <string>
#include <list>
#define INVALID_SOCKET              -1
#define FTP_API                     int
#define MAX_PATH                    260
#define trace                       printf
#define SUCCESSFULL                 0
#define FAILED                      -2
#define FTP_PARAM_BASE
#define FTP_DEFAULT_PORT            "21"                            //FTP默认端口�?
#define FTP_DEFAULT_BUFFER          1024*4                          //FTP下载缓冲大小
#define FTP_DEFAULT_PATH            "/mnt/nand/"                     //FTP默认保存路径

#define ftpprint(fmt, args...) printf("[%s:][%s:%d] "fmt"\r\n","FTPManager", __FUNCTION__, __LINE__, ##args)


#define FTP_COMMAND_BASE            1000
#define FTP_COMMAND_END             FTP_COMMAND_BASE + 30
#define FTP_COMMAND_USERNAME        FTP_COMMAND_BASE + 1            //用户�?
#define FTP_COMMAND_PASSWORD        FTP_COMMAND_BASE + 2            //密码
#define FTP_COMMAND_QUIT            FTP_COMMAND_BASE + 3            //退�?
#define FTP_COMMAND_CURRENT_PATH    FTP_COMMAND_BASE + 4            // 获取文件路径
#define FTP_COMMAND_TYPE_MODE       FTP_COMMAND_BASE + 5            // 改变传输模式
#define FTP_COMMAND_PSAV_MODE       FTP_COMMAND_BASE + 6            // 被动端口模式
#define FTP_COMMAND_DIR             FTP_COMMAND_BASE + 7            // 获取文件列表
#define FTP_COMMAND_CHANGE_DIRECTORY FTP_COMMAND_BASE + 8           // 改变路径
#define FTP_COMMAND_DELETE_FILE     FTP_COMMAND_BASE + 9            // 删除文件
#define FTP_COMMAND_DELETE_DIRECTORY FTP_COMMAND_BASE + 10          // 删除目录/文件�?
#define FTP_COMMAND_CREATE_DIRECTORY FTP_COMMAND_BASE + 11          // 创建目录/文件�?
#define FTP_COMMAND_RENAME_BEGIN    FTP_COMMAND_BASE  +12           // 开始重命名
#define FTP_COMMAND_RENAME_END      FTP_COMMAND_BASE + 13           // 重命名结�?
#define FTP_COMMAND_FILE_SIZE       FTP_COMMAND_BASE + 14           // 获取文件大小
#define FTP_COMMAND_DOWNLOAD_POS    FTP_COMMAND_BASE + 15           // 下载文件从指定位置开�?
#define FTP_COMMAND_DOWNLOAD_FILE   FTP_COMMAND_BASE + 16           // 下载文件
#define FTP_COMMAND_UPLOAD_FILE     FTP_COMMAND_BASE + 17           // 上传文件
#define FTP_COMMAND_APPEND_FILE     FTP_COMMAND_BASE + 18           // 追加上载文件

/*        登陆步骤
        login2Server
            |
        inputUserName
            |
        inputPassWord
            |
          具体操作
            |
          quit
*/



class CFTPManager
{
private:
    //PTFtpPacket m_ftpPacket;
public :
    CFTPManager();
    enum type {
        binary = 0x31,
        ascii,
    };

    virtual ~CFTPManager(void);
    //FTP_API OnInitFtp(PTFtpPacket *pFtpPacket);
    FTP_API LoginFtpServer(std::string &serverIP,std::string &serverPort,std::string &username,std::string &password);
    FTP_API GetFromFtpServer(std::string remotefullname,std::string localfullfilename);
    FTP_API PutToFtpServer(std::string remotefullname,std::string localfullfilename);
    FTP_API ConnectFtpServer();
    FTP_API OutPutReturnValueStr(FTP_API value);
    // ! 登陆服务�?
    FTP_API login2Server(const std::string &serverIP);

    // !输入用户�?
    FTP_API inputUserName(const std::string &userName);

    // !输入密码
    FTP_API inputPassWord(const std::string &password);

    // !退出FTP
    FTP_API quitServer(void);

    // !命令�?PWD
    const std::string PWD();

    // !设置传输格式 2进制  还是ascii方式传输
    FTP_API setTransferMode(type mode);

    // !设置为被动模�?
    const std::string Pasv();

    // ! 命令�?DIR
    const std::string Dir(const std::string &path);

    // !命令 �?CD
    FTP_API CD(const std::string &path);

    // ！删除文�?
    FTP_API DeleteFile(const std::string &strRemoteFile);

    // ! 删除文件�?目录
    FTP_API DeleteDirectory(const std::string &strRemoteDir);

    // ! 创建目录/文件�?
    FTP_API CreateDirectory(const std::string &strRemoteDir);

    // !重命�?
    FTP_API Rename(const std::string &strRemoteFile, const std::string &strNewFile);

    // !获取文件大小
    long getFileLength(const std::string &strRemoteFile);

    // !关闭连接
    void Close(int sock);

    // 下载文件
    FTP_API Get(const std::string &strRemoteFile, const std::string &strLocalFile);

    // 上载文件  支持断电传送方�?
    FTP_API Put(const std::string &strRemoteFile, const std::string &strLocalFile);


private:
    // !合成发送到服务器的命令
    const std::string parseCommand(const unsigned int command, const std::string &strParam);

    // ! 建立连接
    FTP_API Connect(int socketfd, const std::string &serverIP, unsigned int nPort);

    // ! 返回服务器信�?
    const std::string serverResponse(int sockfd);

    // !获取服务器数�?
    FTP_API getData(int fd, char *strBuf, unsigned long length);

    // !发送命�?
    FTP_API Send(int fd, const std::string &cmd);

    // !发送命�?
    FTP_API Send(int fd, const char *cmd, const size_t len);

    // !建立数据连接
    FTP_API createDataLink(int data_fd);

    // !解析PASV模式返回的字符串获取FTP端口号和FTP服务器IP
    FTP_API ParseString(std::list<std::string> strArray, unsigned long & nPort ,std::string & strServerIp);

    // 打开本地文件
    FILE *createLocalFile(const std::string &strLocalFile);

    // 下载文件
    FTP_API downLoad(const std::string &strRemoteFile, const std::string &strLocalFile, const int pos = 0, const unsigned long length = 0);

    // 解析返回ftp命令的�?
    FTP_API parseResponse(const std::string &str);

private:
    //！控制连接套接字
    int     m_cmdSocket;

    // !当前用户�?
    std::string m_strUserName;

    // !当前用户密码
    std::string m_strPassWord;

    // !服务器的IP
    std::string m_strServerIP;

    // !服务器Port
    unsigned int m_nServerPort;

    // !服务器回应信息缓�?
    std::string m_strResponse;

    // !保存命令参数
    std::string m_commandStr;

    // ！当前使用的命令参数
    unsigned int m_nCurrentCommand;

    // !是否登陆标志�?
    bool    m_bLogin;
};

#endif
