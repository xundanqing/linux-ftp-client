#include "ftp.h"
#include <stdio.h>
#include <unistd.h>


int  main()
{

    FtpManager  Ftp("192.168.1.155",8888,"xun","xun");
    Ftp.FtpLogin();
	  Ftp.FtpPut("xun","xun");

    
    while(1)
    {
       sleep(5);
     }
  }
