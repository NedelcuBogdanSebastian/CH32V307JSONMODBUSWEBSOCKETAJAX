/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : Nedelcu Bogdan Sebastian
 * Version            : V1.0.0
 * Date               : 03-September-2024
 * Description        : Server with support for WEBSOCKETS, JSON, AJAX, and MODBUS TCP.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*********************************************************************************/

/*
    This implementation has multiple methods over ethernet, for sending and receiving data.

    It implements WEBSOCKETS, JSON, AJAX, and MODBUS TCP.

    (The AJAX is only LIGHT IMPLEMENTED, only sends an answer packet that obeys CORS rules, NO DATA INTERPRETARION :D )

    To test you have 2 methods, by using the Python scripts or by webpages and RMMS
    utility.

    1. Using the Python scripts, you need python-3.8.10-amd64 and install the following
       libraries:
           * for      get_JSON.py script -> pip install requests         (json and time libraries are part of the Python standard library)

           * for get_WEBSOCKET.py script -> pip install websocket-client (time and threading are part of Python's standard library)

           * for get_MODBUSTCP.py script -> pip install pymodbus         (binascii, struct, pickle, os, time, socket, subprocess, and threading
                                                                          are part of Python's standard library)

    2. Using RMMS and webpages clients:
           * use RMMS as Modbus TCP client, to connect to 192.168.1.10, port 502

           * use the WEBSOCKETClient.html to test data transfer using websockets. After connection, the JS client from the
             web page sends a string once at one second. Also you can send a data string with CRC16 at the end,
             and the CH32V307 server will send it back and the web page will check the data CRC16.
             Use one of the following to compute CRC-16 (Modbus) variant:
                 - https://www.lammertbies.nl/comm/info/crc-calculation
		         - https://www.tahapaksu.com/crc/
		     Attention! For example when inserting the A32AF54FBBC677CDFEEA, the CRC16 (Modbus) result is: 0xA70
		                you need to append a 4 byte CRC16 => 0A70, not only the 0xA70!
		                So the resulted string is A32AF54FBBC677CDFEEA0A70.
		                If you send this data in the send field, and hit the SEND button, the message will go to server,
		                the server will send back the message, and the JS websocket client will check the CRC16 and the
		                CRC Status will become => CRC Status: CRC-OK

		   * to test the JSON you only need to call 192.168.1.10/json.html in Chrome, Firefox or Midori

		   * to test the AJAX you use the AJAXClient.html and there you can send some data and see the received string from the server


     The AJAXServer.py and the app.py are some extra work. Fell free to test! :)

 */


/*
    PB3 - input button, has internal pull-up
    Line 29 in net_config.h. WCHNET_NUM_TCP_LISTEN changed to 3 from 2
    In line 943 eth_driver_10M.c commented a Delay_ms !!!!!! - we use SysTick now !!!!
    Also in line 545 on ReInitMACReg we commented a Delay_Us
    Also in line 786 on ETH_Configuration we commented a Delay_Us

    IZOLATORI PENTRU FIRUL DE COMUNICATIE
    ISO1641BDR !!!
    ADUM1251ARZ-RL7
    ADUM1241ARZ-RL7


 */

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "eth_driver.h"
#include "main.h"
#include "HTTPS.h"
#include "CRC16.h"
#include "ModbusTCP.h"
#include "websocket.h"
#include "wshandshake.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Activate the ones you need to see debug information from
//#define DEBUG_DATA_TCP
//#define DEBUG_DATA_HTTP
//#define DEBUG_DATA_MODBUS
//#define DEBUG_DATA_WEBSOCKET

volatile uint32_t TimingDelay;
volatile uint32_t WEBSOCKETTimingDelay;

uint8_t coil[100]; // Coil
uint16_t mreg[100]; // Register

#define RED_LED_ON        GPIOA->BSHR = GPIO_Pin_15
#define RED_LED_OFF       GPIOA->BCR = GPIO_Pin_15
#define RED_LED_TOGGLE    GPIOA->OUTDR ^= GPIO_Pin_15
#define BLUE_LED_TOGGLE   GPIOB->OUTDR ^= GPIO_Pin_4

#define HTTP_SERVER_PORT            80
#define MODBUS_SERVER_PORT          502
#define WEBSOCKET_SERVER_PORT       8088

u8 MACAddr[6]; //MAC address
u8 IPAddr[4]; //IP address
u8 GWIPAddr[4]; //Gateway IP address
u8 IPMask[4]; //subnet mask

u8 SocketId, SocketIdForListen;


#define TRUE   1
#define FALSE  0
#define MAX_PAYLOAD_SIZE 1024  // Define a reasonable maximum payload size
#define BUF_LEN  512

u8 HTTPDataBuffer[RECE_BUF_LEN];
u8 MODBUSDataBuffer[RECE_BUF_LEN];
//u8 WEBSOCKETDataBuffer[RECE_BUF_LEN];

struct fds {
    int fd;
    uint8_t buffer[BUF_LEN];
    enum wsState state;
    struct ws_frame fr;
    uint32_t readedLength;
};

struct fds client_socket;


u8 SocketRecvBuf[WCHNET_MAX_SOCKET_NUM][RECE_BUF_LEN]; //socket receive buffer

//volatile uint32_t connection_lost_counter = 0;
u16 counter = 0;

u16 PARAMETERSDataBuffer[NOofPARAMETERS];

const char *PARAMETERSNameBuffer[] = {
	"\"Toil_var\": %d,",
	"\"Tsupxacr\": %d,",
	"\"ItrecerA\": %d,",
	"\"Respxacr\": %d,",
	"\"NivelCRS\": %d,",
	"\"CAPbushC\": %d,",
	"\"UAil_var\": %d,",
	"\"UAupxacr\": %d,",
	"\"UArecerA\": %d,",
	"\"UAspxacr\": %d,",
	"\"UAvelCRS\": %d,",
	"\"UAPbushC\": %d"
};

u8 IPAddr[4] = {192, 168, 1, 10}; //IP address
u8 IPMask[4] = {255, 255, 255, 0}; //subnet mask
u8 GWIPAddr[4] = {192, 168, 1, 1}; //Gateway IP address
u8 MACAddr[6] = {0, 0, 0, 0, 0, 0}; //MAC address


void USART2_Putch(unsigned char ch)
{
	USART_SendData(USART2, ch);
	// Wait until the end of transmision
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
}


void USART2_Print(char s[])
{
    uint8_t i=0;

    while( i < 64)
	{
	    if( s[i] == '\0') break;
        USART2_Putch( s[i++]);
    }

}

/*********************************************************************
 * @fn      SYSTICK_Init_Config
 *
 * @brief   SYSTICK_Init_Config.
 *
 * @return  none
 */
void SysTick_Config(u_int64_t ticks)
{
    SysTick->SR &= ~(1 << 0);//clear State flag
    SysTick->CMP = ticks;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF;

    NVIC_SetPriority(SysTicK_IRQn, 15);
    NVIC_EnableIRQ(SysTicK_IRQn);
}
/*********************************************************************
 * @fn      mStopIfError
 *
 * @brief   check if error.
 *
 * @param   iError - error constants.
 *
 * @return  none
 */
void mStopIfError(u8 iError)
{
    if (iError == WCHNET_ERR_SUCCESS)
        return;
#ifdef DEBUG_DATA_TCP
    printf("Error: %02X\r\n", (u16) iError);
#endif
}

/*********************************************************************
 * @fn      TIM2_Init
 *
 * @brief   Initializes TIM2.
 *
 * @return  none
 */
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = { 0 };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = SystemCoreClock / 1000000;
    TIM_TimeBaseStructure.TIM_Prescaler = WCHNETTIMERPERIOD * 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/*********************************************************************
 * @fn      WCHNET_CreateTcpSocketListen
 *
 * @brief   Create socket used for HTTP.
 *          WCHNET establishes the corresponding socket.
 *
 * @return  none
 */
void WCHNET_CreateHTTPSocket(void)
{
    u8 i;
    SOCK_INF TmpSocketInf;

    memset((void *) &TmpSocketInf, 0, sizeof(SOCK_INF));
    TmpSocketInf.SourPort = HTTP_SERVER_PORT;
    TmpSocketInf.ProtoType = PROTO_TYPE_TCP;
    i = WCHNET_SocketCreat(&SocketIdForListen, &TmpSocketInf);
#ifdef DEBUG_DATA_TCP
    printf("HTTP Socket Id %d\r\n", SocketIdForListen);
#endif
    mStopIfError(i);
    i = WCHNET_SocketListen(SocketIdForListen);
    mStopIfError(i);
}

/*********************************************************************
 * @fn      WCHNET_CreateMODBUSSocket
 *
 * @brief   Create socket used for ModbusTCP.
 *          WCHNET establishes the corresponding socket.
 *
 * @return  none
 */
void WCHNET_CreateMODBUSSocket(void)
{
    u8 i;
    SOCK_INF TmpSocketInf;

    memset((void *) &TmpSocketInf, 0, sizeof(SOCK_INF));
    TmpSocketInf.SourPort = MODBUS_SERVER_PORT;
    TmpSocketInf.ProtoType = PROTO_TYPE_TCP;
   i = WCHNET_SocketCreat(&SocketIdForListen, &TmpSocketInf);
#ifdef DEBUG_DATA_MODBUS
    printf("Modbus Socket Id %d\r\n", SocketIdForListen);
#endif
    mStopIfError(i);
    i = WCHNET_SocketListen(SocketIdForListen);
    mStopIfError(i);
}

/*********************************************************************
 * @fn      WCHNET_CreateTcpSocketListen
 *
 * @brief   Create socket used for HTTP.
 *          WCHNET establishes the corresponding socket.
 *
 * @return  none
 */
void WCHNET_CreateWEBSOCKETSocket(void)
{
    u8 i;
    SOCK_INF TmpSocketInf;

    memset((void *) &TmpSocketInf, 0, sizeof(SOCK_INF));
    TmpSocketInf.SourPort = WEBSOCKET_SERVER_PORT;
    TmpSocketInf.ProtoType = PROTO_TYPE_TCP;
    i = WCHNET_SocketCreat(&SocketIdForListen, &TmpSocketInf);
#ifdef DEBUG_DATA_WEBSOCKET
    printf("WEBSOCKET Socket Id %d\r\n", SocketIdForListen);
#endif
    mStopIfError(i);
    i = WCHNET_SocketListen(SocketIdForListen);
    mStopIfError(i);
    // Reset socket id for websocket
    client_socket.fd = -1;
}


void client_close(struct fds *client)
{
    WCHNET_SocketClose(client->fd, TCP_CLOSE_NORMAL);
    WCHNET_CreateWEBSOCKETSocket();
    client->fd = -1;
    client->state = CONNECTING;
    memset(client->buffer, 0, BUF_LEN);
    memset(client->fr.payload, 0, client->readedLength);
    client->readedLength = 0;
}


int send_buff(struct fds *client, uint32_t bufferSize)
{
	uint32_t sendBufSize = bufferSize;
	uint8_t written;

	written = WCHNET_SocketSend(client->fd, (char*)client->buffer, &sendBufSize);

    if (written != WCHNET_ERR_SUCCESS) {
    	client_close(client);
#ifdef DEBUG_DATA_WEBSOCKET
        printf(" === Sending data failed\n");
#endif
        return EXIT_FAILURE;
    }

    if (sendBufSize != bufferSize) {
    	client_close(client);
#ifdef DEBUG_DATA_WEBSOCKET
    	printf(" === Not all bytes written\n");
#endif
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


uint8_t client_handler(struct fds *client) {

    int frameSize = BUF_LEN;
    struct http_header hdr;

    if (client->readedLength > BUF_LEN) {
#ifdef DEBUG_DATA_WEBSOCKET
        printf(" === Data length exceeds buffer size\n");
#endif
        //client_close(client);
        return EXIT_FAILURE;
    }

    WCHNET_SocketRecv(client->fd, client->buffer, &client->readedLength);

#ifdef DEBUG_DATA_WEBSOCKET
    printf(client->buffer);
    printf("WEBSOCKET socket received data length:%d\r\n", client->readedLength);

    int masked_bit = ((client->buffer[1] & 0x80) != 0); // extract masked bit

    // Print frame control information
    printf(" === Frame FIN: %d\n", client->fr.fin);
    printf(" === Frame RSV1: %d\n", client->fr.rsv1);
    printf(" === Frame RSV2: %d\n", client->fr.rsv2);
    printf(" === Frame RSV3: %d\n", client->fr.rsv3);
    printf(" === Frame opcode: 0x%X\n", client->fr.opcode);
    printf(" === Frame Masked: %s\n", masked_bit ? "Yes" : "No");
    printf(" === Frame Payload Length vs. buffer length: %lu, %lu\n", client->fr.payload_length, BUF_LEN);

    if (client->state == 0) printf(" === <<CONNECTING>>\n");
    if (client->state == 1) printf(" === <<OPEN>>\n");
    if (client->state == 2) printf(" === <<CLOSING>>\n");
    if (client->state == 3) printf(" === <<CLOSED>>\n");
#endif

    // Process the frame
    switch (client->state) {
        case CONNECTING:
            // Process handshake and check for WS_OPENING_FRAME
            ws_handshake(&hdr, client->buffer, client->readedLength, &frameSize);

#ifdef DEBUG_DATA_WEBSOCKET
            printf("=========================== HANDSHAKE ===============================\n");
#endif

            if (hdr.type != WS_OPENING_FRAME) {
                send_buff(client, frameSize);
                client_close(client);
                return EXIT_FAILURE;
            } else {
                if (strcmp(hdr.uri, "/echo") != 0) {
                    frameSize = sprintf((char *)client->buffer, "HTTP/1.1 404 Not Found\r\n\r\n");
                    send_buff(client, frameSize);
                    client->state = CLOSING;
                    break;
                }

                if (send_buff(client, frameSize) == EXIT_FAILURE) {
                    return EXIT_FAILURE;
                }
                client->state = OPEN;
                client->readedLength = 0;
            }
            return EXIT_SUCCESS;

        case OPEN:
#ifdef DEBUG_DATA_WEBSOCKET
            printf("=====================================================================\n");
#endif
            ws_parse_frame(&client->fr, client->buffer, client->readedLength);

            // Check payload length
            if (client->fr.payload_length > MAX_PAYLOAD_SIZE) {
#ifdef DEBUG_DATA_WEBSOCKET
                printf(" === Payload size exceeds maximum allowed size\n");
#endif
                return EXIT_FAILURE;
            }

#ifdef DEBUG_DATA_WEBSOCKET
            printf(" === Frame type: 0x%X\n", client->fr.type);
#endif
            if (client->fr.type == WS_ERROR_FRAME) {
#ifdef DEBUG_DATA_WEBSOCKET
                printf(" === Error or reserved frame received --- TREAT AS ERROR\n");
#endif
                //client->readedLength = 0;
                return EXIT_FAILURE;
            }
            else if (client->fr.type == WS_INCOMPLETE_FRAME) {
#ifdef DEBUG_DATA_WEBSOCKET
                printf(" === Incomplete frame received --- TREAT AS ERROR\n");
#endif
                //client->readedLength = 0;
                return EXIT_FAILURE;
            }
            else if (client->fr.type == WS_BINARY_FRAME) {
#ifdef DEBUG_DATA_WEBSOCKET
            	printf(" === Binary frame received --- TREAT AS ERROR\n");
#endif
            	//client->readedLength = 0;
                return EXIT_FAILURE;
            }
            else if (client->fr.type == WS_TEXT_FRAME) {
                // Ensure null-termination for text frames
                client->fr.payload[client->fr.payload_length] = '\0';

                // Print the payload as a string
#ifdef DEBUG_DATA_WEBSOCKET
                printf(" === Payload: %s\n", client->fr.payload);
                printf(" === Make frame '%s' length %lu \n", client->fr.payload, client->fr.payload_length);
#endif
                // Create and send a response frame
                ws_create_text_frame((char*)client->fr.payload, client->buffer, &frameSize);

                if (send_buff(client, frameSize) == EXIT_FAILURE) { // websocket_frame
#ifdef DEBUG_DATA_WEBSOCKET
                	printf(" === Buffer sent\n");
#endif
                    return EXIT_FAILURE;
                }
                client->readedLength = 0;
            }
            else if (client->fr.type == WS_CLOSING_FRAME) {
                //client->state = CLOSING;
            //case CLOSING:
#ifdef DEBUG_DATA_WEBSOCKET
            	printf(" === Close frame\n");
#endif
                ws_create_closing_frame(client->buffer, &frameSize);
                send_buff(client, frameSize);
                client->state = CLOSED;
                //return EXIT_SUCCESS;     // JOINED TOGETHER CLOSING & CLOSED STATES.
                                           // <<<=== HERE
            //case CLOSED:
#ifdef DEBUG_DATA_WEBSOCKET
                printf(" === Frame closed\n");
#endif
                //client_close(client);
                //return EXIT_SUCCESS;
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
/*
        case CLOSING:
            printf(" === Close frame\n");
            ws_create_closing_frame(client->buffer, &frameSize);
            send_buff(client, frameSize);
            client->state = CLOSED;
            return EXIT_SUCCESS;              // THESE TWO STATES ARE MOVED ABOVE TO WS_CLOSING_FRAME
                                              // BECAUSE WHEN THE WEBSOCKET CLIENT IS FORCED CLOSE DON'T SEND ANY ANSWER
        case CLOSED:                          // AND WCHNET_HandleSockInt IS TRIGGERED ONLY WHEN RECEIVING DATA
            printf(" === Frame closed\n");
            client_close(client);
            return EXIT_SUCCESS;
*/
        default:
#ifdef DEBUG_DATA_WEBSOCKET
        	printf(" === <<<UNKNOWN STATE>>>\n");
#endif
            return EXIT_FAILURE;
            break;
    }

    return EXIT_SUCCESS;
}

/*********************************************************************
 * @fn      WCHNET_HandleSockInt
 *
 * @brief   Socket Interrupt Handle
 *
 * @param   socketid - socket id.
 *          intstat - interrupt status
 *
 * @return  none
 */
void WCHNET_HandleSockInt(u8 socketid, u8 intstat)
{
    u32 len;
    u8 res = 0;

    if (intstat & SINT_STAT_RECV)                                      // Receive data
    {
        len = WCHNET_SocketRecvLen(socketid, NULL);

        if (SocketInf[socketid].SourPort == HTTP_SERVER_PORT) {        // Receive request for JSON data or AJAX request
            socket = socketid;
            WCHNET_SocketRecv(socketid, HTTPDataBuffer, &len);
			HTTPDataBuffer[len] = '\0';
#ifdef DEBUG_DATA_HTTP
			printf(" === HTTP socket received data length:%d\r\n",len);
		    printf(HTTPDataBuffer);
#endif
            Web_Server();
            // After the request is processed, the current socket connection is closed, and a new connection
            // will be established when the browser sends the next request
            WCHNET_SocketClose(socket, TCP_CLOSE_NORMAL);
            // Clear HTTP data buffer
            memset(HTTPDataBuffer, 0, sizeof(HTTPDataBuffer));
            BLUE_LED_TOGGLE;
        }
        if (SocketInf[socketid].SourPort == MODBUS_SERVER_PORT) {      // Receive MODBUS data
            socket = socketid;
            WCHNET_SocketRecv(socketid, MODBUSDataBuffer, &len);
#ifdef DEBUG_DATA_MODBUS
            printf(" === MODBUS socket received data length:%d\r\n",len);
#endif
            MB_Parse_Data(socketid, len);
            memset(MODBUSDataBuffer, 0, sizeof(HTTPDataBuffer));
            RED_LED_TOGGLE;
        }
        if (SocketInf[socketid].SourPort == WEBSOCKET_SERVER_PORT) {   // Receive WEBSOCKET data
#ifdef DEBUG_DATA_WEBSOCKET
            printf(" === WEBSOCKET socket received data length:%d\r\n",len);
#endif
            // Find an empty slot for the new connection
            if (client_socket.fd == -1) {
                client_socket.fd = socketid;
                client_socket.state = CONNECTING;
            }

            // Store socket received bytes length to client
            client_socket.readedLength = len;

        	res = client_handler(&client_socket);

        	if (res == EXIT_FAILURE)
        		client_close(&client_socket);

            /* Use this to see raw websocket data only, not handle the client
            socket = socketid;
            WCHNET_SocketRecv(socketid, WEBSOCKETDataBuffer, &len);
            printf("WEBSOCKET socket received data length:%d\r\n",len);
            // After the request is processed, the current socket connection is closed, and a new connection
            // will be established when the browser sends the next request
            WCHNET_SocketClose(socket, TCP_CLOSE_NORMAL);
            // Clear HTTP data buffer
            memset(WEBSOCKETDataBuffer, 0, sizeof(WEBSOCKETDataBuffer));
            */
        }
    }

    if (intstat & SINT_STAT_CONNECT)                                // Connect successfully
    {
        WCHNET_ModifyRecvBuf(socketid, (u32)SocketRecvBuf[socketid], RECE_BUF_LEN);
#ifdef DEBUG_DATA_HTTP
        if (SocketInf[socketid].SourPort == HTTP_SERVER_PORT)
        	printf(" === HTTP TCP socket %d connected\n", socketid);
#endif
#ifdef DEBUG_DATA_MODBUS
        if (SocketInf[socketid].SourPort == MODBUS_SERVER_PORT)
        	printf(" === MODBUS TCP socket %d connected\n", socketid);
#endif
#ifdef DEBUG_DATA_WEBSOCKET
        if (SocketInf[socketid].SourPort == WEBSOCKET_SERVER_PORT)
        	printf(" === WEBSOCKET TCP socket %d connected\n", socketid);
#endif
    }
    if (intstat & SINT_STAT_DISCONNECT)                             // Disconnect
    {
#ifdef DEBUG_DATA_HTTP
        if (SocketInf[socketid].SourPort == HTTP_SERVER_PORT)
        	printf(" === HTTP TCP socket %d disconnected\n", socketid);
#endif
#ifdef DEBUG_DATA_MODBUS
        if (SocketInf[socketid].SourPort == MODBUS_SERVER_PORT)
        	printf(" === MODBUS TCP socket %d disconnected\n", socketid);
#endif
#ifdef DEBUG_DATA_WEBSOCKET
        if (SocketInf[socketid].SourPort == WEBSOCKET_SERVER_PORT)
        	printf(" === WEBSOCKET TCP socket %d disconnected\n", socketid);
#endif
    }

    if (intstat & SINT_STAT_TIM_OUT)                                // Timeout disconnect
    {
    	// When python Websocket client is forced close it does not send any WS_CLOSING_FRAME
    	// and to correctly close the socket for the lost client we manage the timeout
    	// Keep in mind that the Websocket is a stay alive type, is not closed
    	// after each interrogation like Modbus, or JSON!
        if (SocketInf[socketid].SourPort == WEBSOCKET_SERVER_PORT)
        {
        	client_close(&client_socket);
#ifdef DEBUG_DATA_WEBSOCKET
        	printf(" === WEBSOCKET TCP Timeout\n", socketid);
#endif
        }
    }

}

/*********************************************************************
 * @fn      WCHNET_HandleGlobalInt
 *
 * @brief   Global Interrupt Handle
 *
 * @return  none
 */
void WCHNET_HandleGlobalInt(void)
{
    u8 intstat;
    u16 i;
    u8 socketint;


    intstat = WCHNET_GetGlobalInt();                              //get global interrupt flag
    if (intstat & GINT_STAT_UNREACH)                              //Unreachable interrupt
    {
#ifdef DEBUG_DATA_TCP
        printf("GINT_STAT_UNREACH\r\n");
#endif
    }
    if (intstat & GINT_STAT_IP_CONFLI)                            //IP conflict
    {
#ifdef DEBUG_DATA_TCP
    	printf("GINT_STAT_IP_CONFLI\r\n");
#endif
    }
    if (intstat & GINT_STAT_PHY_CHANGE)                           //PHY status change
    {
        i = WCHNET_GetPHYStatus();
        if (i & PHY_Linked_Status)
        {
#ifdef DEBUG_DATA_TCP
            printf("PHY Link Success\r\n");
#endif
            RED_LED_OFF;
        }
        else
        {
        	// Signall TCP link error
        	RED_LED_ON;
        }
    }
    if (intstat & GINT_STAT_SOCKET) {                             //socket related interrupt
        for (i = 0; i < WCHNET_MAX_SOCKET_NUM; i++) {
            socketint = WCHNET_GetSocketInt(i);
            if (socketint)
                WCHNET_HandleSockInt(i, socketint);
        }
    }
}

/*********************************************************************
 * @fn      GPIOInit
 *
 * @brief   GPIO initialization
 *
 * @return  none
 */
void GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/*********************************************************************
 * @fn      EXTI3_INT_INIT
 *
 * @brief   Initializes EXTI3 collection.
 *          PB3 sets the pull-up input, and the falling edge triggers the interrupt.
 *
 * @return  none
 */
void EXTI3_INT_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* GPIOB ----> EXTI_Line3 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program
 *
 * @return  none
 */
int main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    u8 i;

    SystemCoreClockUpdate();



	TimingDelay = 0;
	// SysTick counts miliseconds
	SysTick_Config(SystemCoreClock/1000-1);

	TimingDelay = 500;
    while (TimingDelay != 0x00);

    //Delay_Init();
    USART_Printf_Init(115200);
    GPIOInit();

    // Nu folosim intrerupere pentru ca trebuie sa blocam sistemul cand
    // facem recunoasterea modulelor
    // Oricum PB0 -> EXTI0, PB3 -> EXTI3, intelegi regula
    // EXTI3_INT_INIT();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // button
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);



    printf("Web Server\r\n");
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("net version:%x\n", WCHNET_GetVer());
    if (WCHNET_LIB_VER != WCHNET_GetVer()) {
        printf("version error.\n");
    }

    WCHNET_GetMacAddr(&MACAddr[0]);

    printf("MAC Address: ");
    for(i = 0; i < 6; i++)
        printf("%x ", MACAddr[i]);
    printf("\n");


    TIM2_Init();

    // In line 943 eth_driver_10M.c commented a Delay_ms !!!!!! - we use SysTick now !!!!
    // Also in line 545 on ReInitMACReg we commented a Delay_Us
    // Also in line 786 on ETH_Configuration we commented a Delay_Us
    i = ETH_LibInit(IPAddr, GWIPAddr, IPMask, MACAddr);                         //Ethernet library initialize

    mStopIfError(i);

    if (i == WCHNET_ERR_SUCCESS)
        printf("WCHNET_LibInit Success\r\n");

    WCHNET_CreateHTTPSocket();
    WCHNET_CreateMODBUSSocket();
    WCHNET_CreateWEBSOCKETSocket();

    while(1)
    {
    	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0)
    	{
    		USART2_Print("/SENDCHID\\");
    		//__disable_irq();
    		TimingDelay = 2000;
    	    while (TimingDelay != 0x00);
            //__enable_irq();
    	}

        /*Ethernet library main task function,
         * which needs to be called cyclically*/
        WCHNET_MainTask();
        /*Query the Ethernet global interrupt,
         * if there is an interrupt, call the global interrupt handler*/
        if(WCHNET_QueryGlobalInt())
        {
        	PARAMETERSDataBuffer[0] = 123;
        	PARAMETERSDataBuffer[1] = 456;
        	PARAMETERSDataBuffer[2] = 789;
        	PARAMETERSDataBuffer[3] = 12345;
        	PARAMETERSDataBuffer[4] = 6789;
        	PARAMETERSDataBuffer[5] = counter++;
        	PARAMETERSDataBuffer[6] = 123;
        	PARAMETERSDataBuffer[7] = 456;
        	PARAMETERSDataBuffer[8] = 789;
        	PARAMETERSDataBuffer[9] = 12345;
        	PARAMETERSDataBuffer[10] = 6789;
        	PARAMETERSDataBuffer[11] = counter++;

        	WCHNET_HandleGlobalInt();
        }
    }
}

/*// ======================================================= what is received from CHROME when pressing Run Websocket in client
WEBSOCKET socket received data length:489


G E T   / e c h o   H T T P / 1 . 1

H o s t :   1 9 2 . 1 6 8 . 1 . 1 0 : 8 0 8 8

C o n n e c t i o n :   U p g r a d e

P r a g m a :   n o - c a c h e

C a c h e - C o n t r o l :   n o - c a c h e

U s e r - A g e n t :   M o z i l l a / 5 . 0   ( W i n d o w s   N T   1 0 . 0 ;   W i n 6 4 ;   x 6 4 )   A p p l e W e b K i t / 5 3 7 . 3 6   ( K H T M L ,   l i k e   G e c k o )   C h r o m e / 1 2 8 . 0 . 0 . 0   S a f a r i / 5 3 7 . 3 6

U p g r a d e :   w e b s o c k e t

O r i g i n :   n u l l

S e c - W e b S o c k e t - V e r s i o n :   1 3

A c c e p t - E n c o d i n g :   g z i p ,   d e f l a t e

A c c e p t - L a n g u a g e :   e n - G B , e n - U S ; q = 0 . 9 , e n ; q = 0 . 8

S e c - W e b S o c k e t - K e y :   c Z B 3 c z 7 8 Y O F N D D e Q P i P B 3 Q = =

S e c - W e b S o c k e t - E x t e n s i o n s :   p e r m e s s a g e - d e f l a t e ;   c l i e n t _ m a x _ w i n d o w _ b i t s



    	// Print data received
        socket = socketid;
        WCHNET_SocketRecv(socketid, WEBSOCKETDataBuffer, &len);
        printf("WEBSOCKET socket received data length:%d\n",len);
        for (int i = 0; i < len; i++)
        	printf("%c ", WEBSOCKETDataBuffer[i]);
        printf("\n");
        // Clear HTTP data buffer
        memset(WEBSOCKETDataBuffer, 0, sizeof(WEBSOCKETDataBuffer));
        RED_LED_TOGGLE;
        BLUE_LED_TOGGLE;



*/
