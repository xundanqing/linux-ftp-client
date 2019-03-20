/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : .cpp
 * Author        : liGuangqi
 * Date          : 2018-09-05
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-09-05
 *   Author      : liGuangqi
 *   Modification: Created file

*************************************************************************************************************/



#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "Common.h"
#include "CFTPManager.h"
/*
FTP_API CFTPManager::OnInitFtp(PTFtpPacket *pFtpPacket)
{
	if(pFtpPacket == NULL)
	{
		return FAILED;
	}
	memcpy((char *)&m_ftpPacket,pFtpPacket,sizeof(PTFtpPacket));
	return SUCCESSFULL;
}
*/
FTP_API CFTPManager::LoginFtpServer(std::string &serverIP,std::string &serverPort,std::string &username,std::string &password)
{

	FTP_API ret = 0;
	m_strUserName = username;
	m_strPassWord = password;
	m_strServerIP = serverIP;
    m_nServerPort = atol(serverPort.c_str());
	ret = ConnectFtpServer();
	OutPutReturnValueStr(ret);
	if(ret != 220)
	{
		return CONNECT_FTPSERVER_FAILED;
	}
	ret = inputUserName(username);
	OutPutReturnValueStr(ret);
	if(ret != 331)
	{
		return FTP_USERNAME_ERROR;
	}
	ret = inputPassWord(password);
	OutPutReturnValueStr(ret);
	if(ret != 230)
	{
		return FTP_PASSWORD_ERROR;
	}
	return SUCCESSFULL;
}
FTP_API CFTPManager::GetFromFtpServer(std::string remotefullname,std::string localfullfilename)
{
	FTP_API ret = 0;
	ret = Get(remotefullname,localfullfilename);
	OutPutReturnValueStr(ret);
	if(ret < 0)
	{
		return FTP_GET_FROM_SERVER_FAILED;
	}
	return SUCCESSFULL;
}
FTP_API CFTPManager::PutToFtpServer(std::string remotefullname,std::string localfullfilename)
{
	FTP_API ret = 0;
	ret = Put(remotefullname,localfullfilename);
	OutPutReturnValueStr(ret);
	if(ret < 0)
	{
		return FTP_PUT_SERVER_FAILED;
	}
	return SUCCESSFULL;
}

static int SplitString( std::string strSrc, std::list<std::string> &strArray , std::string strFlag)
{
    int pos = 1;

    while((pos = (int)strSrc.find_first_of(strFlag.c_str())) > 0)
    {
        strArray.insert(strArray.end(), strSrc.substr(0 , pos));
        strSrc = strSrc.substr(pos + 1, strSrc.length() - pos - 1);
    }

    strArray.insert(strArray.end(), strSrc.substr(0, strSrc.length()));

    return 0;
}

CFTPManager::CFTPManager()
{
    m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);
	m_bLogin = false;
}

CFTPManager::~CFTPManager(void)
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");

    Send(m_cmdSocket, strCmdLine.c_str());
    close(m_cmdSocket);
    m_bLogin = false;
}
FTP_API CFTPManager::ConnectFtpServer()
{

	trace("IP: %s port: %d\n", m_strServerIP.c_str(), m_nServerPort);
	if (Connect(m_cmdSocket, m_strServerIP, m_nServerPort) < 0)
    {
		trace("Connect FTP Server failed\n");
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    printf("[%d]ConnectFtpServer() @@@@Response: %s\n", __LINE__,m_strResponse.c_str());

    return  parseResponse(m_strResponse);
}
FTP_API CFTPManager::login2Server(const std::string &serverIP)
{
    std::string strPort;
    int pos = serverIP.find_first_of(":");

    if (pos > 0)
    {
        strPort = serverIP.substr(pos + 1, serverIP.length() - pos);
    }
    else
    {
        pos = serverIP.length();
        strPort = FTP_DEFAULT_PORT;
    }

    m_strServerIP = serverIP.substr(0, pos);
    m_nServerPort = atol(strPort.c_str());

    trace("IP: %s port: %d\n", m_strServerIP.c_str(), m_nServerPort);

    if (Connect(m_cmdSocket, m_strServerIP, m_nServerPort) < 0)
    {

        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    printf("@@@@Response: %s\n", m_strResponse.c_str());

    return  parseResponse(m_strResponse);
}

FTP_API CFTPManager::inputUserName(const std::string &userName)
{
    std::string strCommandLine = parseCommand(FTP_COMMAND_USERNAME, userName);

    m_strUserName = userName;

    if (Send(m_cmdSocket, strCommandLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    printf("Response: %s\n", m_strResponse.c_str());

    return parseResponse(m_strResponse);
}

FTP_API CFTPManager::inputPassWord(const std::string &password)
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_PASSWORD, password);

    m_strPassWord = password;
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    else
    {
        m_bLogin = true;

        m_strResponse = serverResponse(m_cmdSocket);
        printf("Response: %s\n", m_strResponse.c_str());

        return parseResponse(m_strResponse);
    }
}

FTP_API CFTPManager::quitServer(void)
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    else
    {
        m_strResponse = serverResponse(m_cmdSocket);
        printf("Response: %s\n", m_strResponse.c_str());

        return parseResponse(m_strResponse);
    }

}

const std::string CFTPManager::PWD()
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_CURRENT_PATH, "");

    if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
    {
        return "";
    }
    else
    {
        return serverResponse(m_cmdSocket);
    }
}


FTP_API CFTPManager::setTransferMode(type mode)
{
    std::string strCmdLine;

    switch (mode)
    {
    case binary:
        strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "I");
        break;
    case ascii:
        strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "A");
        break;
    default:
        break;
    }

    if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
    {
        return FAILED;
    }
    else
    {
        m_strResponse  = serverResponse(m_cmdSocket);
        printf("@@@@Response: %s", m_strResponse.c_str());

        return parseResponse(m_strResponse);
    }
}


const std::string CFTPManager::Pasv()
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_PSAV_MODE, "");

    if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
    {
        return "";
    }
    else
    {
        m_strResponse = serverResponse(m_cmdSocket);

        return m_strResponse;
    }
}


const std::string CFTPManager::Dir(const std::string &path)
{
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (createDataLink(dataSocket) < 0)
    {
        return "";
    }
    // 鏁版嵁杩炴帴鎴愬姛
    std::string strCmdLine = parseCommand(FTP_COMMAND_DIR, path);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
        close(dataSocket);
        return "";
    }
    else
    {
        trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
        m_strResponse = serverResponse(dataSocket);

        trace("@@@@Response: \n%s\n", m_strResponse.c_str());
        close(dataSocket);

        return m_strResponse;
    }

}


FTP_API CFTPManager::CD(const std::string &path)
{
	if(m_cmdSocket == INVALID_SOCKET)
	{
		return FAILED;
	}
    std::string strCmdLine = parseCommand(FTP_COMMAND_CHANGE_DIRECTORY, path);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API CFTPManager::DeleteFile(const std::string &strRemoteFile)
{
	if(m_cmdSocket == INVALID_SOCKET)
	{
		return FAILED;
	}
    std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_FILE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    printf("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API CFTPManager::DeleteDirectory(const std::string &strRemoteDir)
{
	if(m_cmdSocket == INVALID_SOCKET)
	{
		return FAILED;
	}
    std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_DIRECTORY, strRemoteDir);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API CFTPManager::CreateDirectory(const std::string &strRemoteDir)
{
	if(m_cmdSocket == INVALID_SOCKET)
	{
		return FAILED;
	}
    std::string strCmdLine = parseCommand(FTP_COMMAND_CREATE_DIRECTORY, strRemoteDir);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API CFTPManager::Rename(const std::string &strRemoteFile, const std::string &strNewFile)
{
	if(m_cmdSocket == INVALID_SOCKET)
	{
		return FAILED;
	}
    std::string strCmdLine = parseCommand(FTP_COMMAND_RENAME_BEGIN, strRemoteFile);
    Send(m_cmdSocket, strCmdLine);
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    Send(m_cmdSocket, parseCommand(FTP_COMMAND_RENAME_END, strNewFile));

    m_strResponse = serverResponse(m_cmdSocket);
    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

long CFTPManager::getFileLength(const std::string &strRemoteFile)
{
	if(INVALID_SOCKET == m_cmdSocket)
	{
		return FAILED;
	}
    std::string strCmdLine = parseCommand(FTP_COMMAND_FILE_SIZE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());

    std::string strData = m_strResponse.substr(0, 3);
    unsigned long val = atol(strData.c_str());

    if (val == 213)
    {
        strData = m_strResponse.substr(4);
        trace("strData: %s\n", strData.c_str());
        val = atol(strData.c_str());

        return val;
    }

    return -1;
}


void CFTPManager::Close(int sock)
{
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = INVALID_SOCKET;
}

FTP_API CFTPManager::Get(const std::string &strRemoteFile, const std::string &strLocalFile)
{
	//从服务器获取文件大小
	 unsigned long nSize = getFileLength(strRemoteFile);
     return downLoad(strRemoteFile, strLocalFile,0,nSize);
}


FTP_API CFTPManager::Put(const std::string &strRemoteFile, const std::string &strLocalFile)
{
    std::string strCmdLine;
    const unsigned long dataLen = FTP_DEFAULT_BUFFER;
    char strBuf[dataLen] = {0};
    long nSize = 0;
    //long nSize = getFileLength(strRemoteFile);
    long nLen = 0;
//  struct stat sBuf;
//
//  assert(stat(strLocalFile.c_str(), &sBuf) == 0);
//  trace("size: %d\n", sBuf.st_size);

    FILE *pFile = fopen(strLocalFile.c_str(), "rb");  // 浠ュ彧璇绘柟寮忔墦寮�  涓旀枃浠跺繀椤诲瓨鍦?
	if(pFile == NULL)
	{
		return FAILED;
	}
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    //assert(data_fd != -1);
	if(data_fd == -1)
	{
		return FAILED;
	}
    if (createDataLink(data_fd) < 0)
    {
        return -1;
    }

    //if (nSize == -1)
    //{
        strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFile);
   // }
  //  else
   // {
   //   strCmdLine = parseCommand(FTP_COMMAND_APPEND_FILE, strRemoteFile);
   // }

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        Close(data_fd);
        return -1;
    }

    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    fseek(pFile, 0, SEEK_SET);
	
    while (!feof(pFile))
    {
        nLen = fread(strBuf, 1, dataLen, pFile);
        if (nLen < 0)
        {
            break;
        }

        if (Send(data_fd, strBuf) < 0)
        {
            Close(data_fd);
            return -1;
        }
    }

    trace("@@@@Response: %s\n", serverResponse(data_fd).c_str());

    Close(data_fd);
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
    fclose(pFile);

    return 0;
}

const std::string CFTPManager::parseCommand(const unsigned int command, const std::string &strParam)
{
    if (command < FTP_COMMAND_BASE || command > FTP_COMMAND_END)
    {
        return "";
    }

    std::string strCommandLine;

    m_nCurrentCommand = command;
    m_commandStr.clear();

    switch (command)
    {
    case FTP_COMMAND_USERNAME:
        strCommandLine = "USER ";
        break;
    case FTP_COMMAND_PASSWORD:
        strCommandLine = "PASS ";
        break;
    case FTP_COMMAND_QUIT:
        strCommandLine = "QUIT ";
        break;
    case FTP_COMMAND_CURRENT_PATH:
        strCommandLine = "PWD ";
        break;
    case FTP_COMMAND_TYPE_MODE:
        strCommandLine = "TYPE ";
        break;
    case FTP_COMMAND_PSAV_MODE:
        strCommandLine = "PASV ";
        break;
    case FTP_COMMAND_DIR:
        strCommandLine = "LIST ";
        break;
    case FTP_COMMAND_CHANGE_DIRECTORY:
        strCommandLine = "CWD ";
        break;
    case FTP_COMMAND_DELETE_FILE:
        strCommandLine = "DELE ";
        break;
    case FTP_COMMAND_DELETE_DIRECTORY:
        strCommandLine = "RMD ";
        break;
    case FTP_COMMAND_CREATE_DIRECTORY:
        strCommandLine = "MKD ";
        break;
    case FTP_COMMAND_RENAME_BEGIN:
        strCommandLine = "RNFR ";
        break;
    case FTP_COMMAND_RENAME_END:
        strCommandLine = "RNTO ";
        break;
    case FTP_COMMAND_FILE_SIZE:
        strCommandLine = "SIZE ";
        break;
    case FTP_COMMAND_DOWNLOAD_FILE:
        strCommandLine = "RETR ";
        break;
    case FTP_COMMAND_DOWNLOAD_POS:
        strCommandLine = "REST ";
        break;
    case FTP_COMMAND_UPLOAD_FILE:
        strCommandLine = "STOR ";
        break;
    case FTP_COMMAND_APPEND_FILE:
        strCommandLine = "APPE ";
        break;
    default :
        break;
    }

    strCommandLine += strParam;
    strCommandLine += "\r\n";

    m_commandStr = strCommandLine;
    trace("parseCommand: %s\n", m_commandStr.c_str());

    return m_commandStr;
}

FTP_API CFTPManager::Connect(int socketfd, const std::string &serverIP, unsigned int nPort)
{
    if (socketfd == INVALID_SOCKET)
    {
        return -1;
    }

    unsigned int argp = 1;
    int error = -1;
    int len = sizeof(int);
    struct sockaddr_in  addr;
    bool ret = false;
    timeval stime;
    fd_set  set;

    ioctl(socketfd, FIONBIO, &argp);  //璁剧疆涓洪潪闃诲妯″紡

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(nPort);
    addr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    bzero(&(addr.sin_zero), 8);

    trace("Address: %s %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    if (connect(socketfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1)   //鑻ョ洿鎺ヨ繑鍥?鍒欒鏄庢鍦ㄨ繘琛孴CP涓夋鎻℃墜
    {
        stime.tv_sec = 20;  //璁剧疆涓?绉掕秴鏃?
        stime.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(socketfd, &set);

        if (select(socketfd + 1, NULL, &set, NULL, &stime) > 0)   ///鍦ㄨ繖杈圭瓑寰?闃诲 杩斿洖鍙互璇荤殑鎻忚堪绗?鎴栬�呰秴鏃惰繑鍥?  鎴栬�呭嚭閿欒繑鍥?1
        {
            getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len);
            if (error == 0)
            {
                ret = true;
            }
            else
            {
                ret = false;
            }
        }
    }
    else
    {   trace("Connect Immediately!!!\n");
        ret = true;
    }

    argp = 0;
    ioctl(socketfd, FIONBIO, &argp);

    if (!ret)
    {
        close(socketfd);
        fprintf(stderr, "cannot connect server!!\n");
        return -1;
    }

    //fprintf(stdout, "Connect!!!\n");

    return 0;
}


const std::string CFTPManager::serverResponse(int sockfd)
{
    if (sockfd == INVALID_SOCKET)
    {
        return "";
    }

    int nRet = -1;
    char buf[MAX_PATH] = {0};

    m_strResponse.clear();

    while ((nRet = getData(sockfd, buf, MAX_PATH)) > 0)
    {
        buf[MAX_PATH - 1] = '\0';
        m_strResponse += buf;
    }

    return m_strResponse;
}

FTP_API CFTPManager::getData(int fd, char *strBuf, unsigned long length)
{
	if(NULL == strBuf)
	{
		return FAILED;
	}
    if (fd == INVALID_SOCKET)
    {
        return -1;
    }

    memset(strBuf, 0, length);
    timeval stime;
    int nLen;

    stime.tv_sec = 1;
    stime.tv_usec = 0;

    fd_set  readfd;
    FD_ZERO( &readfd );
    FD_SET(fd, &readfd );

    if (select(fd + 1, &readfd, 0, 0, &stime) > 0)
    {
        if ((nLen = recv(fd, strBuf, length, 0)) > 0)
        {
            return nLen;
        }
        else
        {
            return -2;
        }
    }
    return 0;
}

FTP_API CFTPManager::Send(int fd, const std::string &cmd)
{
    if (fd == INVALID_SOCKET)
    {
        return -1;
    }

    return Send(fd, cmd.c_str(), cmd.length());
}

FTP_API CFTPManager::Send(int fd, const char *cmd, const size_t len)
{
    if((FTP_COMMAND_USERNAME != m_nCurrentCommand)
        &&(FTP_COMMAND_PASSWORD != m_nCurrentCommand)
        &&(!m_bLogin))
    {
        return -1;
    }

    timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    fd_set  writefd;
    FD_ZERO(&writefd);
    FD_SET(fd, &writefd);

    if(select(fd + 1, 0, &writefd , 0 , &timeout) > 0)
    {
        size_t nlen  = len;
        int nSendLen = 0;
        while (nlen >0)
        {
            nSendLen = send(fd, cmd , (int)nlen , 0);

            if(nSendLen == -1)
                return -2;

            nlen = nlen - nSendLen;
            cmd +=  nSendLen;
        }
        return 0;
    }
    return -1;
}


FTP_API CFTPManager::createDataLink(int data_fd)
{
	if(INVALID_SOCKET == data_fd)
	{
		return FAILED;
	}
    std::string strData;
    unsigned long nPort = 0 ;
    std::string strServerIp ;
    std::list<std::string> strArray ;

    std::string parseStr = Pasv();

    if (parseStr.size() <= 0)
    {
        return -1;
    }

    //trace("parseInfo: %s\n", parseStr.c_str());

    size_t nBegin = parseStr.find_first_of("(");
    size_t nEnd   = parseStr.find_first_of(")");
    strData       = parseStr.substr(nBegin + 1, nEnd - nBegin - 1);

    //trace("ParseAfter: %s\n", strData.c_str());
    if( SplitString( strData , strArray , "," ) <0)
        return -1;

    if( ParseString( strArray , nPort , strServerIp) < 0)
        return -1;

    //trace("nPort: %ld IP: %s\n", nPort, strServerIp.c_str());

    if (Connect(data_fd, strServerIp, nPort) < 0)
    {
        return -1;
    }

    return 0;

}

FTP_API CFTPManager::ParseString(std::list<std::string> strArray, unsigned long & nPort ,std::string & strServerIp)
{
    if (strArray.size() < 6 )
        return -1 ;

    std::list<std::string>::iterator citor;
    citor = strArray.begin();
    strServerIp = *citor;
    strServerIp += ".";
    citor ++;
    strServerIp += *citor;
    strServerIp += ".";
    citor ++ ;
    strServerIp += *citor;
    strServerIp += ".";
    citor ++ ;
    strServerIp += *citor;
    citor = strArray.end();
    citor--;
    nPort = atol( (*citor).c_str());
    citor--;
    nPort += atol( (*(citor)).c_str()) * 256 ;
    return 0 ;
}

FILE *CFTPManager::createLocalFile(const std::string &strLocalFile)
{
    return fopen(strLocalFile.c_str(), "w+b");
}
//length即从服务器获取的文件长度
FTP_API CFTPManager::downLoad(const std::string &strRemoteFile, const std::string &strLocalFile, const int pos, const unsigned long length)
{
	if(length < 0)
	{
		return FAILED;
	}
    FILE *file = NULL;
    unsigned long nDataLen = FTP_DEFAULT_BUFFER;
    char strPos[MAX_PATH]  = {0};
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(-1 == data_fd)
	{
		return FAILED;
	}
    if ((length != 0) && (length < nDataLen))
    {
        nDataLen = length;
    }
    char *dataBuf = new char[nDataLen];
	if(NULL == dataBuf)
	{
		return FAILED;
	}
    sprintf(strPos, "%d", pos);

    if (createDataLink(data_fd) < 0)
    {
        trace("@@@@ Create Data Link error!!!\n");
        return -1;
    }
	//去掉断网续传

    std::string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

   strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    file = createLocalFile(std::string(FTP_DEFAULT_PATH + strLocalFile));
	if(NULL == file)
	{
		return FAILED;
	}
    int len = 0;
    unsigned long nReceiveLen = 0;
    while ((len = getData(data_fd, dataBuf, nDataLen)) > 0)
    {
        nReceiveLen += len;

        int num = fwrite(dataBuf, 1, len, file);
        memset(dataBuf, 0, nDataLen);

        //trace("%s", dataBuf);
        trace("Num:%d\n", num);
        if (nReceiveLen == length && length != 0)
        {
            break;
		}
        if ((nReceiveLen + nDataLen) > length  && length != 0)
        {
            delete []dataBuf;
            nDataLen = length - nReceiveLen;
            dataBuf = new char[nDataLen];
            memset(dataBuf,0,nDataLen);
        }
    }

    Close(data_fd);
    fclose(file);
    delete []dataBuf;
	if(nReceiveLen != length )
	{
		trace("file size error nReceiveLen!=length:%ld!=%ld\n",nReceiveLen,length);
		return -1;
	}
    return 0;
}

FTP_API CFTPManager::parseResponse(const std::string &str)
{
	if(str.empty())
	{
		return FAILED;
	}
    std::string strData = str.substr(0, 3);
    unsigned int val = atoi(strData.c_str());

    return val;
}

FTP_API CFTPManager::OutPutReturnValueStr(FTP_API value)
{
	switch(value)
	{
		case 110:
		{
			ftpprint("新文件指示器上的重启标记\n");
			break;
		}
		case 120:
		{
			ftpprint("服务器准备就绪的时间（分钟数）\n");
			break;
		}
		case 125:
		{
			ftpprint("打开数据连接，开始传输\n");
			break;
		}
		case 150:
		{
			ftpprint("打开连接\n");
			break;
		}
		case 200:
		{
			ftpprint("成功\n");
			break;
		}
		case 202:
		{
			ftpprint("命令没有执行\n");
			break;
		}
		case 211:
		{
			ftpprint("系统状态回复\n");
			break;
		}
		case 212:
		{
			ftpprint("目录状态回复\n");
			break;
		}
		case 213:
		{
			ftpprint("文件状态回复\n");
			break;
		}
		case 214:
		{
			ftpprint("帮助信息回复\n");
			break;
		}
		case 215:
		{
			ftpprint("系统类型回复\n");
			break;
		}
		case 220:
		{
			ftpprint("服务就绪\n");
			break;
		}
		case 221:
		{
			ftpprint("退出网络\n");
			break;
		}
		case 225:
		{
			ftpprint("打开数据连接\n");
			break;
		}
		case 226:
		{
			ftpprint("结束数据连接\n");
			break;
		}
		case 227:
		{
			ftpprint("进入被动模式\n");
			break;
		}
		case 230:
		{
			ftpprint("登录因特网\n");
			break;
		}
		case 250:
		{
			ftpprint("文件行为完成\n");
			break;
		}
		case 257:
		{
			ftpprint("路径名建立\n");
			break;
		}
		case 331:
		{
			ftpprint("要求密码\n");
			break;
		}
		case 332:
		{
			ftpprint("要求帐号\n");
			break;
		}
		case 350:
		{
			ftpprint("文件行为暂停\n");
			break;
		}
		case 421:
		{
			ftpprint("服务关闭\n");
			break;
		}
		case 425:
		{
			ftpprint("无法打开数据连接\n");
			break;
		}
		case 426:
		{
			ftpprint("结束连接\n");
			break;
		}
		case 450:
		{
			ftpprint("文件不可用\n");
			break;
		}
		case 451:
		{
			ftpprint("遇到本地错误\n");
			break;
		}
		case 452:
		{
			ftpprint("磁盘空间不足\n");
			break;
		}
		case 500:
		{
			ftpprint("无效命令\n");
			break;
		}
		case 501:
		{
			ftpprint("错误参数\n");
			break;
		}
		case 502:
		{
			ftpprint("命令没有执行\n");
			break;
		}
		case 503:
		{
			ftpprint("错误指令序列\n");
			break;
		}
		case 504:
		{
			ftpprint("无效命令参数\n");
			break;
		}
		case 530:
		{
			ftpprint("未登录网络\n");
			break;
		}
		case 532:
		{
			ftpprint("存储文件需要帐号\n");
			break;
		}
		case 550:
		{
			ftpprint("文件不可用\n");
			break;
		}
		case 551:
		{
			ftpprint("不知道的页类型\n");
			break;
		}
		case 552:
		{
			ftpprint("超过存储分配\n");
			break;
		}
		case 553:
		{
			ftpprint("文件名不允许\n");
			break;
		}
		default:
		{
			ftpprint("未找到返回码对应的字符串,value=%d\n",value);
		}

	}
	return 0;
}

