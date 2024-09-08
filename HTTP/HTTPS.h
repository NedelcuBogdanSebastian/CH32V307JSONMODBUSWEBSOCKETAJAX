/********************************** (C) COPYRIGHT *******************************
 * File Name          : HTTPS.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/05/31
 * Description        : HTTP related parameters.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef	__HTTPS_H__
#define	__HTTPS_H__
#include "debug.h"
#include "wchnet.h"

/*Address where configuration
 * information is stored*/
#define PAGE_WRITE_START_ADDR     ((uint32_t)0x0803FC00) /* Start from 255K */
#define PAGE_WRITE_END_ADDR       ((uint32_t)0x08040000) /* End at 256K */
#define FLASH_PAGE_SIZE           256

#define MAX_URL_SIZE              32

/* HTTP request method*/
#define	METHOD_ERR		          0
#define	METHOD_GET		          1
#define	METHOD_HEAD		          2
#define	METHOD_POST		          3

/* HTTP request URL */
#define	PTYPE_ERR		          0
#define	PTYPE_HTML	              1

/*WCHNET communication Mode*/
#define MODE_TCPSERVER            0
#define MODE_TCPCLIENT            1

/* HTML Doc. for ERROR */
#define RES_HTMLHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:"

#define RES_END "\r\n\r\n"

typedef struct _st_http_request                 //Browser request information
{
	char	METHOD;					
	char	TYPE;					
	char	URL[MAX_URL_SIZE];
}st_http_request;

typedef struct Para_Tab                         //Configuration information parameter table
{
	char *para;                                 //Configuration item name
	char value[30];                             //Configuration item value
}Parameter;

extern st_http_request http_request;

extern u8 httpweb[200] ;

extern u8 HTTPDataBuffer[];

extern u8 socket;

extern void ParseHttpRequest(st_http_request *, char *);	

extern void ParseURLType(char *, char *);

void MakeHttpResponse(u8 *buf, char type, u32 len );

extern char *GetURLName(char* url);

extern char *DataLocate(char *buf,char *name);

extern void copy_flash(const char *html, u32 len);

extern void Init_Para_Tab(void) ;

extern void Web_Server(void);

extern void WEB_ERASE(u32 Page_Address, u32 Length );

extern FLASH_Status WEB_WRITE( u32 StartAddr, u8 *Buffer, u32 Length );

extern void WEB_READ( u32 StartAddr, u8 *Buffer, u32 Length );

#endif	
