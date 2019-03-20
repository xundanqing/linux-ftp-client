#include "ftp.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/vfs.h> 
/*****************************************************************************
 * Function      : ConnectServer
 * Description   : �������ӷ�����
 * Input         : const  char *acHostiP       
                unsigned  short  iHostPort  
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static   int  ConnectServer(const char *acHostiP,  short iHostPort,int *piCilentFd)
{
    int iRet;
    int iFd;
    int iFlag = 1;

    TSoLinger tSoLinger;
    TTimeVal  tTimeVal;
    struct    sockaddr_in  TSockAddrIn;
    
    iFd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(iFd, IPPROTO_TCP, TCP_NODELAY, (char *)&iFlag, sizeof(iFlag));

    tTimeVal.tv_sec    = 2;
    tTimeVal.tv_usec   = 0;
    setsockopt(iFd, SOL_SOCKET, SO_SNDTIMEO, (char *)(&tTimeVal), sizeof(tTimeVal));

    tSoLinger.l_onoff  = 1;
    tSoLinger.l_linger = 5;
    setsockopt(iFd, SOL_SOCKET, SO_LINGER, (char*)(&tSoLinger), sizeof(tSoLinger));

    TSockAddrIn.sin_family = AF_INET;
    TSockAddrIn.sin_port   = htons(iHostPort);
    TSockAddrIn.sin_addr.s_addr = inet_addr(acHostiP);

   
    iRet = connect(iFd, (TSockAddr*)(&TSockAddrIn), sizeof(TSockAddrIn));

    if(0 == iRet)
    {
        *piCilentFd  = iFd;
         return  FTP_SUCCESS;
    }
    else
    {   
        close(iFd);
        return   FTP_ERROR;
    } 
}
/*****************************************************************************
 * Function      : SocketRead
 * Description   : ��ûظ�����
 * Input         : int  ifd       
                char *aucdata  
                int  *ilen     
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static  int  SocketRead(int  ifd,char *aucdata, int  idestlen)
{
    int nLen;
    timeval stime;

    stime.tv_sec  = 5;
    stime.tv_usec = 0;

    fd_set  readfd;
    FD_ZERO(&readfd );
    FD_SET (ifd, &readfd );
    memset (aucdata,0, idestlen);

    if (select(ifd+1, &readfd, 0, 0, &stime) > 0)
    {
        if ((nLen = recv(ifd, aucdata, idestlen, 0)) > 0)
        {
            return nLen;
        }
        else
        {
            return  -2;
        }
    }
    return  0;
}
/*****************************************************************************
 * Function      : SocketSend
 * Description   : ������������
 * Input         : char *acRespon   
                int   iResonlen  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
int  SocketSend(int iFtpSocketfd , const  char *acCmd,int   iCmdlen)
{

    int  nlen;
    int  nSendLen;
    timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    fd_set  writefd;
    FD_ZERO(&writefd);
    FD_SET (iFtpSocketfd, &writefd);

    nlen =  iCmdlen;

    if(select(iFtpSocketfd + 1, 0, &writefd , 0 , &tv) > 0)
    {
        while (nlen >0)
        {
            nSendLen = send(iFtpSocketfd, acCmd , iCmdlen , 0);
            if(nSendLen == -1)
            {
               return  FTP_ERROR;
            }
            nlen -= nSendLen;
            acCmd+= nSendLen;
        }
    }

    return (nlen == 0)? FTP_SUCCESS : FTP_ERROR;
    
}
/*****************************************************************************
 * Function      : SocketClose
 * Description   : �Ͽ�FTP����
 * Input         : int  iFtpfd  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
int  SocketClose(int  iFtpfd)
{
     shutdown(iFtpfd, SHUT_RDWR);
     close(iFtpfd);
}
/*****************************************************************************
 * Function      : FtpManager
 * Description   : ftpmanager���Ĺ��캯��
 * Input          : None
 * Output        : None
 * Return        : FtpManager
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FtpManager ::  FtpManager(const  char* SeriP,short SerPort,const char *Sername,const char *SerPass)                                                   //FTP���캯��
{
   strcpy(acFtpiPaddr,SeriP);
   strcpy(acFtpUserName,Sername);
   strcpy(acFtpPassWord,SerPass);
   iFtpPort = SerPort;
   iFtpSocketfd = FTP_FALSE;                                
   iFtpTag      = FTP_FALSE;
}
/*****************************************************************************
 * Function      : ~FtpManager
 * Description   : ftp��������
 * Input          : None
 * Output        : None
 * Return        : FtpManager
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FtpManager :: ~FtpManager()
{
  
}
/*****************************************************************************
 * Function      : Ftp_Connect
 * Description   : ����FTP������
 * Input          : None
 * Output        : None
 * Return        : FtpManager
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API FtpManager :: ConnectFtpServer()
{
    int   iRet;
    int   iResponlen;
    char  aucRespon[FTP_BUFFER_LEN] = {0};

    iRet = ConnectServer(acFtpiPaddr,iFtpPort,&iFtpSocketfd);
    if(iRet == FTP_ERROR)
    {
        SocketClose(iFtpSocketfd) ;
        iFtpTag = FTP_FALSE;
        return    FTP_ERROR;
    }
    else
        iFtpTag = FTP_TRUE;


    iRet = SocketRead(iFtpSocketfd,aucRespon,FTP_BUFFER_LEN);
    if(iRet <= 0)
    {
        SocketClose(iFtpSocketfd);
        return  FTP_ERROR;
    }

    if(!strstr(aucRespon,"220"))
    {
       aucRespon[FTP_BUFFER_LEN -1] = 0;
       printf("%s\n",aucRespon);
       return   FTP_ERROR;
    }
    
    return FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : Ftp_InputUsername
 * Description   : ����FTP�û���
 * Input          : None
 * Output        : None
 * Return        : FtpManager
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API FtpManager ::   FtpUser()
{
   int     iRet;
   const   char   *Username  = "USER xun\r\n";
   char    aucData[FTP_BUFFER_LEN] = {0};
   
   iRet = SocketSend(iFtpSocketfd,Username, 12) ;
   if(iRet == FTP_ERROR)
      return  FTP_ERROR;

     
   iRet = SocketRead(iFtpSocketfd, aucData, FTP_BUFFER_LEN);
   if(iRet <= 0)
      return  FTP_ERROR;


   if(!strstr(aucData,"331"))
   {
       aucData[FTP_BUFFER_LEN -1] = 0;
       printf("%s\n",aucData);
       return  FTP_ERROR;
   }   
   return      FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpManager.FtpPass
 * Description   : Ftp ��������
 * Input          : None
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::   FtpPass ()
{
   int    iRet;
   const  char   *Password  = "PASS xun\r\n";
   char   aucData[FTP_BUFFER_LEN] =  {0};
   
   iRet = SocketSend(iFtpSocketfd,Password, 12) ;
   if(iRet == FTP_ERROR)
      return  FTP_ERROR;

     
   iRet = SocketRead(iFtpSocketfd, aucData, FTP_BUFFER_LEN);
   if(iRet < 0)
      return  FTP_ERROR;

  if(!strstr(aucData,"230"))
  {
      aucData[FTP_BUFFER_LEN -1] = 0;
      printf("%s \n",aucData);
      return  FTP_ERROR; 
  }    
  return    FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : Ftp_Login
 * Description   : ����FTP��������½
 * Input          : None
 * Output        : None
 * Return        : FtpManager
 * Others        : 
 * Record
 * 1.Date        : 20180821
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API FtpManager ::  FtpLogin()
{  
   if(ConnectFtpServer()== FTP_ERROR)
      return FTP_ERROR;
   
   if(FtpUser()== FTP_ERROR)
      return FTP_ERROR;

   if(FtpPass()==FTP_ERROR)
      return  FTP_ERROR;

      printf("Ftp  Login  OK\n");
   
   return     FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpGetFileLenth
 * Description   : ���Զ�˷������ļ�������Ϣ
 * Input         : int isocketfd         
                char *remotefilename  
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static  int  FtpGetDestFileLen(int isocketfd ,const  char *remotefilename)
{
   int   iret;
   int   ilen;
   int   ifilesize;
   const char   *Prifix =   "SIZE ";
   char  acDataCmd [FTP_BUFFER_LEN] = {0};
   char  acRespone [FTP_BUFFER_LEN] = {0};
   strcat(acDataCmd,Prifix);
   strcat(acDataCmd,remotefilename);
   strcat(acDataCmd,"\r\n");
   ilen = strlen(acDataCmd);
   
   iret = SocketSend(isocketfd,(const  char *)acDataCmd,ilen);
   if(iret == FTP_ERROR)
      return  FTP_ERROR;

   iret = SocketRead(isocketfd, acRespone, FTP_BUFFER_LEN);
   if(iret <= 0 )
      return  FTP_ERROR;
      
  if(!strstr(acRespone,"213"))
  {
     acRespone[FTP_BUFFER_LEN -1] = 0;
     printf("%s \n",acRespone);
     return  FTP_ERROR;
  }

  return    atol(&acRespone[3]);
}
/*****************************************************************************
 * Function      : ParaseTransiP
 * Description   : �����ظ����ӵ�����IP �Ͷ˿�
 * Input         : char *ServeriP  
                short  Port     
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static  int  ParaseTransiP(char *str ,char *ServeriP, short*  Port)
{   
    int   i =0;
    int   n =0;
    int   istrlen;
    char *pCurbuf;
    short Porthigh;
    short Portlow;
    
	if(!strstr(str , "(") ||!strstr(str , ")"))
	    return  FTP_ERROR;
	    
    pCurbuf = strstr(str,"(");
    pCurbuf ++;

    /*Parase  Server  iP str*/
    while(pCurbuf)
    {
       if(*pCurbuf == ',')
       {
          *pCurbuf =  '.';
          if((n++) == 3) 
          {
             pCurbuf++;
             break;
          }
       }
       ServeriP[i++] = *pCurbuf;
       pCurbuf++;
    }

   /*Parsse  Server  Port*/
   Porthigh= atoi(pCurbuf);
   pCurbuf = strstr(pCurbuf,",");
   pCurbuf ++;
   Portlow = atoi(pCurbuf);
  *Port =   Porthigh << 8 | Portlow;
   /*parase  Server  info ok*/
   
   return    FTP_SUCCESS;
   
}
/*****************************************************************************
 * Function      : FtpManager.FtpPassive
 * Description   : Ftp  ����ģʽ����
 * Input          : None
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  FtpPassive()
{
    int      ilen;
    int      iRet;
    short    iPort;
    char     acTransiP[FTP_HOST_LEN]= {0};
    char     acRespone[FTP_BUFFER_LEN] = {0};
	  const char *acPassmodecmd  = "PASV \r\n";

    iRet = SocketSend(iFtpSocketfd,acPassmodecmd,strlen(acPassmodecmd));
    if(iRet == FTP_ERROR)
        return FTP_ERROR;

    ilen = SocketRead(iFtpSocketfd, acRespone, FTP_BUFFER_LEN);
    if(ilen < 0)
         return  FTP_ERROR;

    if(!strstr(acRespone,"227"))
         return  FTP_ERROR;
         
    if(ParaseTransiP(acRespone,acTransiP,&iPort)==FTP_ERROR)
         return  FTP_ERROR;

    iRet = ConnectServer(acTransiP, iPort, &iFtpTransfd);
    if(iRet == FTP_ERROR)
        return  FTP_ERROR;
    
    return    FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : SendfileBeginPos
 * Description   : �����ļ���ʼλ��
 * Input         : int  iSockfd   
                int  ifilepos  
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static  int  SendfileBeginPos(int  iSockfd,int  ifilepos)
{

	const char  *acPrifix = "REST ";
	char  strPos   [FTP_HOST_LEN]   = {0};
	char  acCmd    [FTP_BUFFER_LEN] = {0};
	char  acRespon [FTP_BUFFER_LEN] = {0};
	
    sprintf(strPos,  "%d", ifilepos);
    strcpy(acCmd,acPrifix);
    strcat(acCmd,strPos);
    strcat(acCmd,"\r\n");

    if(SocketSend(iSockfd,(const  char *) acCmd, strlen(acCmd)) == FTP_ERROR)
       return  FTP_ERROR;

    if(SocketRead(iSockfd, acRespon, FTP_BUFFER_LEN) <= 0)
       return  FTP_ERROR;
       
    if(!strstr(acRespon,"350"))
       return  FTP_ERROR;

    return  FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : SendBeginFrame
 * Description   : ���Ϳ�ʼ֡
 * Input         : int  iSocketfd  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
int  SendBeginFrame(int  iSocketfd ,const  char *filename)
{
   
   const  char*  Prifix = "RETR ";
   char   acCmd   [FTP_BUFFER_LEN] = {0};
   char   acRespon[FTP_BUFFER_LEN] = {0};

   strcpy(acCmd,Prifix);
   strcat(acCmd,filename);
   strcat(acCmd,"\r\n");

   if (SocketSend(iSocketfd, acCmd, strlen(acCmd))==FTP_ERROR)
      return  FTP_ERROR;

   if(SocketRead(iSocketfd, acRespon, FTP_BUFFER_LEN) <= 0)
      return  FTP_ERROR;

   if(!strstr(acRespon,"150"))
      return  FTP_ERROR;

   return   FTP_SUCCESS;
   
}
/*****************************************************************************
 * Function      : FtpTransFile
 * Description   : �����ļ���ʼ
 * Input         : int  iSocketfd         
                const  char *filename  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
int  FtpTransFile(int  iSocketfd ,const  char *filename ,int  ifilelen)
{
    int   ilen    = 0;
    int   iCurlen = 0;
	char  acfilebuffer[4*FTP_BUFFER_LEN] ={0};

    FILE *fd =  fopen(filename,"w+b");
    if(!fd)
       return FTP_ERROR;

    
	while((ilen = SocketRead(iSocketfd,acfilebuffer,4*FTP_BUFFER_LEN)) > 0)
	{ 
       fwrite(acfilebuffer, 1, ilen, fd);
       iCurlen += ilen;
       if(iCurlen == ifilelen)
       {
          break;
       }
       memset(acfilebuffer,0x00,4*FTP_BUFFER_LEN);
    }
    
    SocketClose(iSocketfd);
    fclose(fd);
    sync();
    
    return  (iCurlen >= ifilelen) ? FTP_SUCCESS : FTP_ERROR;
}
/*****************************************************************************
 * Function      : FtpManager.FtpCreat
 * Description   : ���������ļ�
 * Input         : const  char *filename  
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API    FtpManager ::  FtpCreat(const  char *filename)
{
    FILE*  fd;
   
    fd = fopen(filename, "w+b");
   
    if(!fd)
      return FTP_ERROR;
      
    fclose(fd);
    return   FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpManager.FtpDown
 * Description   : ������������
 * Input         : int    ifilesize            
                const    char *aclocalname  
                const char *remotename      
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  FtpDown (int    ifilesize,const    char *aclocalname,const char *remotename)
{
    int  iRet;
    int  CurfilePos = 0;


    if(FtpPassive()== FTP_ERROR)
        return  FTP_ERROR;

    if(SendfileBeginPos(iFtpSocketfd,0)== FTP_ERROR)
        return FTP_ERROR;

    if(SendBeginFrame(iFtpSocketfd,remotename) == FTP_ERROR)
        return FTP_ERROR;

    if(FtpCreat(remotename) == FTP_ERROR)
        return  FTP_ERROR;
        
    if(FtpTransFile(iFtpTransfd,remotename,ifilesize)==FTP_ERROR)
        return  FTP_ERROR;

    printf("Down load  file %s  ok\n",remotename);
    
    return  FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpManager.FtpGet
 * Description   : ����Զ���ļ�������
 * Input         : char *acfilename  
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180829
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  FtpGet  (const  char *acFilename  ,char *locltename)
{  
    int    iret;
    int    ifilen;
    struct statfs  diskInfo; 
    long   long    ulCurdisk = 0;

    iret = statfs(FTP_FILE_DISK_PATH,&diskInfo);
    if(iret ==  0)
       ulCurdisk  =   diskInfo.f_bsize*diskInfo.f_bfree;

    ifilen = FtpGetDestFileLen(iFtpSocketfd,acFilename);
    if(ifilen < 0)
        return  FTP_ERROR;

    if(ulCurdisk  <  ifilen)
    {
        printf("disk  left  %d ,change your disk\n",ulCurdisk);
        return  FTP_ERROR;
    }

    if(FtpDown(ifilen,acFilename,locltename)==FTP_ERROR)
    {
         printf("ftp  %s  error\n ",acFilename);
         return  FTP_ERROR;
    }

    printf("ftp  put  file  %s  ok",acFilename);
    
    return   FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : SendFtpPutFrame
 * Description   : �ϱ�Ftp�������ļ��ϴ����ݰ�
 * Input         : const   char *acRemoteFilename  
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180830
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static   int  SendFtpPutFrame(int  ifd,const   char *acRemoteFilename)
{
	const   char*  acCmd = "STOR ";
	char    acSendData[FTP_BUFFER_LEN] = {0};
    char    acResponse[FTP_BUFFER_LEN] = {0};
    
	strcat(acSendData,acCmd);
	strcat(acSendData,acRemoteFilename);
	strcat(acSendData,"\r\n");

	if(SocketSend(ifd,acSendData,strlen(acSendData)) == FTP_ERROR)
	   return  FTP_ERROR;

	if(SocketRead(ifd,acResponse, FTP_BUFFER_LEN) == FTP_ERROR)
	   return  FTP_ERROR;

    if(!strstr(acResponse,"150"))
    {
        printf("%s\n",acResponse);
        return  FTP_ERROR;
    }
    return  FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpPutProc
 * Description   : FTP�����ϴ���ʼ
 * Input         : int  ifd           
                char *aclocalname  
 * Output        : None
 * Return        : static
 * Others        : 
 * Record
 * 1.Date        : 20180830
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
static   int  FtpPutProc(int  ifd,char *aclocalname)
{
    int    ilen;
    FILE*  File;
    int    itotal;
    char   acData  [FTP_BUFFER_LEN] = {0};
    char   acRespon[FTP_BUFFER_LEN] = {0};
    
    File = fopen(aclocalname,"rb");
    if(!File)
       return  FTP_ERROR;

    fseek(File,0,SEEK_SET);

    printf("Put  file  %s start \n",aclocalname);
    
    while (1)
    {
        ilen = fread(acData, 1, FTP_BUFFER_LEN,File);
        if (ilen <= 0)
        {
            break;
        }
        
        if(SocketSend(ifd,acData,ilen) == FTP_ERROR)
        {
            fclose(File);
            SocketClose (ifd);
            return  FTP_ERROR;
        }

        
        memset(acData,0x00,FTP_BUFFER_LEN);
    }
    fclose(File);
    SocketClose(ifd);
    return  FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpManager.FtpPut
 * Description   : �ϴ�����
 * Input         : const  char *acFilename  
                char *locltename         
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180830
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  FtpPut  (const  char *acFilename  ,char *locltename)
{

	if(FtpPassive()== FTP_ERROR)
	    return  FTP_ERROR;

    if(SendFtpPutFrame(iFtpSocketfd,acFilename)==FTP_ERROR)
        return  FTP_ERROR;


    if(FtpPutProc(iFtpTransfd,locltename) == FTP_ERROR)
    {   
        printf("ftp  file put %s error\n",locltename);
        return  FTP_ERROR;
    }

    printf("Ftp  Put file %s ok\n",locltename);
    
    return  FTP_SUCCESS;
}
/*****************************************************************************
 * Function      : FtpManager.FtpPwd
 * Description   : ��õ�ǰ�Ĺ���Ŀ¼
 * Input          : None
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180830
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  FtpPwd  ()   
{

   const  char  *acPrfix ="PWD ";
   char   acCmd   [FTP_BUFFER_LEN] = {0};
   char   acRespon[FTP_BUFFER_LEN] = {0};

   strcat(acCmd,acPrfix);
   strcat(acCmd, "\r\n");

   if(SocketSend(iFtpSocketfd,acCmd, strlen(acCmd)) == FTP_ERROR)
      return  FTP_ERROR;

   if(SocketRead(iFtpSocketfd,acRespon,FTP_BUFFER_LEN)== FTP_ERROR)
      return  FTP_ERROR;

   if(!strstr(acRespon,"/") || !strstr(acRespon,"257"))
   {
      acRespon[FTP_BUFFER_LEN -1] = '0';
      printf("%s\n",acRespon);
      return  FTP_ERROR;
   }
   
   return     FTP_SUCCESS ;
}
/*****************************************************************************
 * Function      : FtpManager.FtpCd
 * Description   : Ftp �������������Ŀ¼
 * Input         : const char * acRemotePath  
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180830
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  FtpCd(const char * acRemotePath)
{
   const  char  *acPrfix ="CWD ";
   char   acCmd   [FTP_BUFFER_LEN] = {0};
   char   acRespon[FTP_BUFFER_LEN] = {0};

   strcat(acCmd,acPrfix);
   strcat(acCmd,acRemotePath);
   strcat(acCmd, "\r\n");

   if(SocketSend(iFtpSocketfd,acCmd, strlen(acCmd)) == FTP_ERROR)
      return  FTP_ERROR;

   if(SocketRead(iFtpSocketfd,acRespon,FTP_BUFFER_LEN)== FTP_ERROR)
      return  FTP_ERROR;

   
   if(!strstr(acRespon,"/") || !strstr(acRespon,"250"))
   {
     acRespon[FTP_BUFFER_LEN -1] = '0';
     printf("%s\n",acRespon);
     return  FTP_ERROR;
   }
   return     FTP_SUCCESS ;
}
/*****************************************************************************
 * Function      : FtpManager.FtpMkDir
 * Description   : ����Ftp  ������ָ����Ŀ¼
 * Input         : const char * acRemotePath  
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180830
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager :: FtpMkDir(const char * acRemotePath)
{

   const  char  *acPrfix ="MKD ";
   char   acCmd   [FTP_BUFFER_LEN] = {0};
   char   acRespon[FTP_BUFFER_LEN] = {0};

   strcat(acCmd,acPrfix);
   strcat(acCmd,acRemotePath);
   strcat(acCmd, "\r\n");

   if(SocketSend(iFtpSocketfd,acCmd, strlen(acCmd)) == FTP_ERROR)
      return  FTP_ERROR;

   if(SocketRead(iFtpSocketfd,acRespon,FTP_BUFFER_LEN)== FTP_ERROR)
      return  FTP_ERROR;

   
   if(!strstr(acRespon,acRemotePath) ||!strstr(acRespon,"257"))
   {
      printf("%s\n",acRespon);
      return  FTP_ERROR;
   }
   
   return     FTP_SUCCESS ;
}
/*****************************************************************************
 * Function      : FtpManager.Ftp_GetServerHost
 * Description   : ����豸�����ӷ�����Ϣ
 * Input          : None
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180828
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API FtpManager :: GetFtpHostaddr()
{
    
}
/*****************************************************************************
 * Function      : FtpManager.Ftp_GetLoginInfo
 * Description   : ���FTP��½����
 * Input          : None
 * Output        : None
 * Return        : FTP_API
 * Others        : 
 * Record
 * 1.Date        : 20180828
 *   Author      : xundanqing
 *   Modification: Created function

*****************************************************************************/
FTP_API  FtpManager ::  GetFtpUserPass()
{
  
    return FTP_TRUE;
}
