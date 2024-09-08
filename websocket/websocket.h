/* websocket.c - websocket lib
 *
 * Copyright (C) 2016 Borislav Sapundzhiev
 * Modifications made by: Nedelcu Bogdan Sebastian
 * Date of modifications: 03-September-2024
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H


#include "base64.h"
#include "sha1.h"


#define WS_VERSION 13
#define WS_WEBSOCK "websocket"
#define WS_MAGIC   "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum wsFrameType {
    WS_EMPTY_FRAME = 0xF0,
    WS_ERROR_FRAME = 0xF1,
    WS_INCOMPLETE_FRAME = 0xF2,
    WS_TEXT_FRAME = 0x01,
    WS_BINARY_FRAME = 0x02,
    WS_PING_FRAME = 0x09,
    WS_PONG_FRAME = 0x0A,
    WS_OPENING_FRAME = 0xF3,
    WS_CLOSING_FRAME = 0x08
};

enum wsState {
    CONNECTING = 0,  /*The connection is not yet open.*/
    OPEN =       1,  /*The connection is open and ready to communicate.*/
    CLOSING =    2,  /*The connection is in the process of closing.*/
    CLOSED =     3,  /*The connection is closed or couldn't be opened.*/
};


struct ws_frame {
    uint8_t fin;
    uint8_t rsv1;
    uint8_t rsv2;
    uint8_t rsv3;
    uint8_t opcode;
    uint8_t *payload;
    uint32_t payload_length;
    enum wsFrameType type;
};


void ws_parse_frame(struct ws_frame *frame, uint8_t *data, int len);
void ws_create_frame(struct ws_frame *frame, uint8_t *out_data, int *out_len);
void ws_create_closing_frame(uint8_t *out_data, int *out_len);
void ws_create_text_frame(const char *text, uint8_t *out_data, int *out_len);
void ws_create_binary_frame(const uint8_t *data, uint16_t datalen, uint8_t *out_data, int *out_len);
void ws_create_control_frame(enum wsFrameType type, const uint8_t *data, int data_len, uint8_t *out_data, int *out_len);


#endif /* WEBSOCKET_H */
