/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : ftp.h
 * Author        : xundanqing
 * Date          : 2018-08-21
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-08-21
 *   Author      : xundanqing
 *   Modification: Created file

*************************************************************************************************************/
/*socket attribute*/
typedef int                  FTP_API;     
typedef struct timeval      TTimeVal;
typedef struct linger       TSoLinger;
typedef struct sockaddr     TSockAddr;
typedef struct sockaddr_un  TSockAddrUn;
typedef struct sockaddr_in  TSockAddrIn;


/*FTP  RETURN  VALUE*/
#define  FTP_TRUE                     (  1 )
#define  FTP_FALSE                    (  0 )
#define  FTP_ERROR                    ( -1 )
#define  FTP_TIMEOUT                  (  0 )
#define  FTP_SUCCESS                  (  1 )
#define  FTP_HOST_LEN                 ( 16 )
#define  FTP_NAME_LEN                 ( 128)
#define  FTP_BUFFER_LEN               (1024)
#define  FTP_WAIT_TIME                ( 10 )
#define  FTP_FILE_DISK_PATH           "/opt/tm6158/data"             //目的磁盘挂载目录


/*FTP 类*/
class    FtpManager
{

  public:  
     FtpManager(const  char* SeriP,short SerPort,const char *Sername,const char *SerPass);                                                  
    ~FtpManager();                                                     //FTP析构函数
     FTP_API    FtpLogin  ();                                          //FTP登陆函数
     FTP_API    FtpUser   ();                                          //FTP输入用户名
     FTP_API    FtpPass   ();                                          //FTP输入密码
     FTP_API    FtpPassive();                                          //FTP被动模式
     FTP_API    ConnectFtpServer();                                    //FTP建立连接
     FTP_API    CloseFtpServer  ();                                    //FTP断开连接
     FTP_API    FtpCreat(const  char *filename);                       //创建本地文件
     FTP_API    FtpDown (int    ifilesize ,const char *aclocalname,const char *remotefile);

 public :
 
     FTP_API    FtpPwd  ();                                            //获得当前工作目录
     FTP_API    FtpGet  (const  char *acFilename  ,char *locltename);  //下载远端的文件  
     FTP_API    FtpPut  (const  char *aclocalname ,char *remotename);  //上传本地文件到服务器
     FTP_API    FtpCd   (const  char *acRemotePath);                   //进入FTP服务器目录
     FTP_API    FtpMkDir(const  char *acRemotePath);                   //在FTP服务器下创建新的目录
     FTP_API    FtpRmdir(const  char *acRemotePath);                   //在FTP下删除目录及文件
     
  private:
     char       acFtpUserName [FTP_NAME_LEN];                           //FTP用户名
     char       acFtpPassWord [FTP_NAME_LEN];                           //FTP密码
     char       acFtpiPaddr   [FTP_HOST_LEN];                           //FTPIP地址
     char       acCurFtpPath  [FTP_NAME_LEN];                           //FTP的当前工作目录
     char       acCurLocalPath[FTP_NAME_LEN];
     short      iFtpPort;                                               //FTP端口号
     int        iFtpSocketfd;                                           //FTPsocket
     int        iFtpTransfd;                                            //FTP传输文件句柄
     int        iFtpTag;                                                //FTP连接标示

};


                                
