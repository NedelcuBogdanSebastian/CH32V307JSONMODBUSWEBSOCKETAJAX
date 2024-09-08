/********************************** (C) COPYRIGHT *******************************
 * File Name          : HTTPS.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/05/31
 * Description        : HTTPS related functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*********************************************************************************
* Modifications made by: Nedelcu Bogdan Sebastian
* Date of modifications: 03-September-2024
*********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>    
#include "HTTPS.h"
#include "main.h"

#define HTML_LEN     1024*5                                 //Maximum size of a single web page

st_http_request http_request;

u8 *name;                                               //The name of the web page requested by HTTP
u8 socket;                                              //socket id
u8 httpweb[200];                                        //The array is used to store the HTTP response message
char HtmlBuffer[HTML_LEN];                              //Web page send buffer

extern u8 HTTPDataBuffer[RECE_BUF_LEN];//MAC address IP address Gateway IP address subnet mask

extern u16 PARAMETERSDataBuffer[NOofPARAMETERS];
extern const char *PARAMETERSNameBuffer[];

/*********************************************************************
 * @fn      ParseHttpRequest
 *
 * @brief   Analyze the request message, take out the method and assign it to request->method,
 *          and assign the URL to request->URL (with '/')
 *
 * @param   request - data buff,Used to save the content of each part of the post request
 *          buf - POST request
 *
 * @return  none
 */
void ParseHttpRequest(st_http_request *request, char *buf)
{
    char *strptr = buf;

    if (strstr(strptr, "GET") || strstr(strptr, "get")) {       /*browser 'get' request*/
        request->METHOD = METHOD_GET;
        strptr += strlen("GET") + 2;
        memset(buf, 1, strlen("GET"));                          /* clear the request method */
    }
    else {
        request->METHOD = METHOD_ERR;
        return;
    }
    request->URL[MAX_URL_SIZE - 1] = '0';
    memcpy(request->URL, strptr, MAX_URL_SIZE - 1);
    strptr = strtok(request->URL, " ");
    strcpy((char*) request->URL, strptr);
}

/*********************************************************************
 * @fn      ParseURLType
 *
 * @brief   Parse URL type
 *
 * @param   type - type.
 *          buf - data buff
 *
 * @return  URL type
 */
void ParseURLType(char *type, char * buf)
{
    if (strstr(buf, ".html") || strstr(name, "HTTP")) /* html type */
        *type = PTYPE_HTML;
    else
        *type = PTYPE_ERR;
}

/*********************************************************************
 * @fn      MakeHttpResponse
 *
 * @brief   Select the corresponding response message and assign
 *           it to buf, according to the type .
 *
 * @param   buf - data buff
 *          type - type
 *          len - data length
 *
 * @return  none
 */
void MakeHttpResponse(u8 *buf, char type, u32 len )
{
    char *head = 0;
    char string[8] = {0};

    memset(buf, 0, sizeof(buf));
    if (type == PTYPE_HTML)
        head = RES_HTMLHEAD_OK;
    strcpy(buf, head);
    snprintf(string, sizeof(string), "%d", len);
    strcat(buf, string);
    strcat(buf, RES_END);
}

/*********************************************************************
 * @fn      DataLocate
 *
 * @brief   Locate the location of "name"
 *
 * @param   buf - data buff
 *          name - name strings
 *
 * @return  pointer to the last digit of name
 */
char * DataLocate(char *buf, char *name)
{
    char *p;
    p = strstr(buf, name);
    if (p != NULL)
        p += strlen(name);
    return p;
}



/*********************************************************************
 * @fn      Refresh_Html
 *
 * @brief   Select an html file in flash, replace it with the
 *          parameter table, and save it to HtmlBuffer[].
 *
 * @param   html - HTML array constants
 *          buf - parameter structure
 *          paranum - parameters number
 *
 * @return  length of data
 */
uint16_t Refresh_Html(const char *html, Parameter *buf, u8 paranum)
{
    const char *lastptr = html;
    char *currptr;
    const char *keyword = "__A";
    uint8_t i;
    uint16_t datalen = 0, copylen = 0;

    while(1)
    {
        currptr = strstr(lastptr, keyword);
        if(currptr == NULL) break;
        copylen = currptr - lastptr;
        memcpy(&HtmlBuffer[datalen], lastptr, copylen);
        datalen += copylen;
        for(i = 0; i < paranum; i++)
        {
            if(memcmp(currptr, buf[i].para, strlen(buf[i].para)) == 0)
                break;
        }
        if(i == paranum)
            return 0 ;
        memcpy(&HtmlBuffer[datalen], buf[i].value, strlen(buf[i].value));
        datalen += strlen(buf[i].value);
        lastptr = currptr + strlen(buf[i].para);
    }
    if(strlen(lastptr) != 0)
    {
        memcpy(&HtmlBuffer[datalen], lastptr, strlen(lastptr));
        datalen += strlen(lastptr);
    }

    return datalen;
}

/*********************************************************************
 * @fn      copy_flash
 *
 * @brief   Select the html file in the flash and copy it directly
 *          to the HtmlBuffer (only for some web pages without variables)
 *
 * @param   html - HTML array constants
 *          len - data length
 *
 * @return  none
 */
void copy_flash(const char *html, u32 len)
{
    memset(HtmlBuffer, 0, HTML_LEN);
    memcpy(HtmlBuffer, html, len);
}

/*********************************************************************
 * @fn      WEB_ERASE
 *
 * @brief   erase Data-Flash block, minimal block is 256B.
 *
 * @param   Page_Address - the address of the page being erased
 *          Length - erasing length
 *
 * @return  none
 */
void WEB_ERASE(uint32_t Page_Address, u32 Length) {
    u32 NbrOfPage, EraseCounter;

    FLASH_Unlock_Fast();
    NbrOfPage = Length / FLASH_PAGE_SIZE;

    for (EraseCounter = 0; EraseCounter < NbrOfPage; EraseCounter++) {
        FLASH_ErasePage_Fast( Page_Address + (FLASH_PAGE_SIZE * EraseCounter)); //Erase 256B
    }
    FLASH_Lock_Fast();
}

/*********************************************************************
 * @fn      WEB_WRITE
 *
 * @brief   write Data-Flash data block.
 *
 * @param   StartAddr - the address of the page being written.
 *          Buffer - data buff
 *          Length - data length
 *
 * @return  FLASH_Status
 */
FLASH_Status WEB_WRITE(u32 StartAddr, u8 *Buffer, u32 Length) {
    u32 address = StartAddr;
    u32 *p_buff = (u32 *) Buffer;
    FLASH_Status FLASHStatus = FLASH_COMPLETE;

    FLASH_Unlock();
    while((address < (StartAddr + Length)) && (FLASHStatus == FLASH_COMPLETE))
    {
        FLASHStatus = FLASH_ProgramWord(address, *p_buff);
        address += 4;
        p_buff++;
    }
    FLASH_Lock();
    return FLASHStatus;
}

/*********************************************************************
 * @fn      WEB_READ
 *
 * @brief   read Data-Flash data block.
 *
 * @param   StartAddr - the address of the page being read.
 *          Buffer - data buff
 *          Length - data length
 *
 * @return  none
 */
void WEB_READ(u32 StartAddr, u8 *Buffer, u32 Length) {
    u32 address = StartAddr;
    u32 *p_buff = (u32 *) Buffer;

    while(address < (StartAddr + Length))
    {
        *p_buff = (*(u32 *)address);
        address += 4;
        p_buff++;
    }
}


/*********************************************************************
 * @fn      Data_Send
 *
 * @brief   Socket sends data.
 *
 * @return  none
 */
void Data_Send(u8 id, uint8_t *dataptr, uint32_t datalen)
{
    u32 len, totallen;
    u8 *p, timeout = 50;

    p = dataptr;
    totallen = datalen;
    while(1){
        len = totallen;
        WCHNET_SocketSend(id, p, &len);                         //Send the data
        totallen -= len;                                        //Subtract the sent length from the total length
        p += len;                                               //offset buffer pointer
        if(--timeout == 0) break;
        if(totallen) continue;                                  //If the data is not sent, continue to send
        break;                                                  //After sending, exit
    }
}


/*********************************************************************
 * @fn      strFind
 *
 * @brief   query for a specific string.
 *
 * @param   str  - source string.
 *          substr - String to be queried.
 *
 * @return  The number of data segments contained in the received data
 */
int strFind( char str[], char substr[] )
{
    int i, j, check ,count = 0;
    int len = strlen( str );
    int sublen = strlen( substr );
    for( i = 0; i < len; i++ )
    {
        check = 1;
        for( j = 0; j + i < len && j < sublen; j++ )
        {
            if( str[i + j] != substr[j] )
            {
                check = 0;
                break;
            }
        }
        if( check == 1 )
        {
            count++;
            i = i + sublen;
        }
    }
    return count;
}



#define BUFFER_SIZE 1024

// Function to add a header to the response
void add_header(char *response, const char *header_name, const char *header_value) {
    strcat(response, header_name);
    strcat(response, ": ");
    strcat(response, header_value);
    strcat(response, "\r\n");
}




/*********************************************************************
 * @fn      Web_Server
 *
 * @brief   web process function.
 *
 * @return  none
 */
void Web_Server(void)
{
    uint8_t reqnum = 0;
    u32 resplen = 0;
    u32 pagelen = 0;

    char Html_json[1000];

    // Create JSON buffer with parameters name and data
    // for the request for JSON data
    sprintf(Html_json,"{");
    for (u8 i = 0; i < NOofPARAMETERS; i++)
	    sprintf(Html_json + strlen(Html_json), PARAMETERSNameBuffer[i], PARAMETERSDataBuffer[i]);
    sprintf(Html_json + strlen(Html_json),"}\0");

    reqnum = strFind(HTTPDataBuffer,"GET") + strFind(HTTPDataBuffer,"get") + \
             strFind(HTTPDataBuffer,"POST") + strFind(HTTPDataBuffer,"post");

    while (reqnum) {
        reqnum--;
        ParseHttpRequest(&http_request, HTTPDataBuffer);

        switch (http_request.METHOD)
        {
            case METHOD_ERR:
                break;

            case METHOD_GET:                                        // GET method
                name = http_request.URL;
                ParseURLType(&http_request.TYPE, name);

                if(strstr(name, "json") != NULL) {                  // Request for JSON data
                	pagelen = strlen(Html_json);
                	copy_flash(Html_json, pagelen);
                    // Analyze the requested resource type and return the response
                    MakeHttpResponse(httpweb, http_request.TYPE, pagelen);
                    resplen = strlen(httpweb);
                    Data_Send(socket, httpweb, resplen);
                    Data_Send(socket, HtmlBuffer, pagelen);
                } else {                                            // AJAX request for data
                    char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\n";
                    // Add CORS headers
                    add_header(response, "Access-Control-Allow-Origin", "*");
                    add_header(response, "Access-Control-Allow-Methods", "GET, POST");
                    add_header(response, "Access-Control-Allow-Headers", "Content-Type");
                    add_header(response, "Content-Type", "application/json");
                    strcat(response, "\r\n"); // End of headers
                    // Add JSON body to the answer send to AJAX request
                    strcat(response, "{\"message\": \"This is a CORS correct AJAX JSON response.\"}");
                    resplen = strlen(response);
                    Data_Send(socket, response, resplen);
                }
                break;

            default:
                break;
        }
    }
}
