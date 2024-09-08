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

#include "string.h"
#include "websocket.h"

//#if BYTE_ORDER == LITTLE_ENDIAN
//#define HTONS(v) (v >> 8) | (v << 8)
//#else
//#define HTONS(v) v
//#endif

#define HTONS(v) (((v) >> 8) | (((v) & 0xFF) << 8))

#define MASK_LEN 4


void ws_create_frame(struct ws_frame *frame, uint8_t *out_data, int *out_len)
{
    out_data[0] = 0x80 | frame->type;

    if(frame->payload_length <= 125) 
    {
        out_data[1] = frame->payload_length;
        *out_len = 2;
    } 
    else if (frame->payload_length >= 126 && frame->payload_length <= 0xFFFF) 
    {
        out_data[1] = 126;
        out_data[2] = (uint8_t)( frame->payload_length >> 8 ) & 0xFF;
        out_data[3] = (uint8_t)( frame->payload_length      ) & 0xFF;
        *out_len = 4;
    } 
/* ========= we will never use more than 65535 bytes in a frame !!!    
    else 
    {
        out_data[1] = 127;
        out_data[2] = (uint8_t)( frame->payload_length >> 56 ) & 0xFF;
        out_data[3] = (uint8_t)( frame->payload_length >> 48 ) & 0xFF;
        out_data[4] = (uint8_t)( frame->payload_length >> 40 ) & 0xFF;
        out_data[5] = (uint8_t)( frame->payload_length >> 32 ) & 0xFF;
        out_data[6] = (uint8_t)( frame->payload_length >> 24 ) & 0xFF;
        out_data[7] = (uint8_t)( frame->payload_length >> 16 ) & 0xFF;
        out_data[8] = (uint8_t)( frame->payload_length >>  8 ) & 0xFF;
        out_data[9] = (uint8_t)( frame->payload_length       ) & 0xFF;
        *out_len = 10;
    }
*/
    memcpy(&out_data[*out_len], frame->payload, frame->payload_length);

    *out_len += frame->payload_length;
}

static int ws_parse_opcode(struct ws_frame *frame)
{
    switch(frame->opcode) 
    {
        case WS_TEXT_FRAME:
        case WS_BINARY_FRAME:
        case WS_CLOSING_FRAME:
        case WS_PING_FRAME:
        case WS_PONG_FRAME:
            frame->type = frame->opcode;
            break;
        default:
            frame->type = WS_ERROR_FRAME; //Reserved frames are also treated as errors !!!!
            break;
    }

    return frame->type;
}

void ws_parse_frame(struct ws_frame *frame, uint8_t *data, int len) {
    int masked = 0;
    uint64_t payloadLength = 0;
    int byte_count = 2, i;
    uint8_t maskingKey[MASK_LEN];

    if (len < 2) {
        frame->type = WS_INCOMPLETE_FRAME;
        return;
    }

    // Parse the first byte
    uint8_t b = data[0];
    frame->fin  = ((b & 0x80) != 0);  // 0b1000 0000
    frame->rsv1 = ((b & 0x40) != 0);  // 0b0100 0000
    frame->rsv2 = ((b & 0x20) != 0);  // 0b0010 0000
    frame->rsv3 = ((b & 0x10) != 0);  // 0b0001 0000
    frame->opcode = (uint8_t)(b & 0x0F);  // 0b0000 1111

    // Validate opcode
    if (ws_parse_opcode(frame) == WS_ERROR_FRAME) {
        frame->type = WS_ERROR_FRAME;
        return;
    }

    // Parse the second byte
    b = data[1];
    masked = ((b & 0x80) != 0);
    payloadLength = (uint8_t)(127 & b);

    // Handle extended payload lengths
    if (payloadLength == 127) {
        if (len < 10) { // 2 (initial bytes) + 8 (extended payload length)
            frame->type = WS_INCOMPLETE_FRAME;
            return;
        }
        payloadLength = 0;
        for (i = 2; i < 10; i++) {
            payloadLength = (payloadLength << 8) | data[i];
        }
        byte_count = 10;
    } else if (payloadLength == 126) {
        if (len < 4) { // 2 (initial bytes) + 2 (extended payload length)
            frame->type = WS_INCOMPLETE_FRAME;
            return;
        }
        payloadLength = (data[2] << 8) | data[3];
        byte_count = 4;
    }

    // Ensure the payload length is reasonable and fits within the remaining data
    if (payloadLength > UINT32_MAX - MASK_LEN || (payloadLength + byte_count + (masked ? MASK_LEN : 0)) > len) {
        frame->type = WS_ERROR_FRAME;
        return;
    }
    
    // Copy the masking key if present
    if (masked) {
        if (len < byte_count + MASK_LEN) {
            frame->type = WS_INCOMPLETE_FRAME;
            return;
        }
        for (i = 0; i < MASK_LEN; i++) {
            maskingKey[i] = data[byte_count + i];
        }
        byte_count += MASK_LEN;
    }

    // Ensure we have enough data for the payload
    if (len < byte_count + payloadLength) {
        frame->type = WS_INCOMPLETE_FRAME;
        return;
    }

    // Assign the payload
    frame->payload = &data[byte_count];
    frame->payload_length = payloadLength;

    // Unmask the payload if necessary
    if (masked) {
        for (uint32_t len = 0; len < frame->payload_length; len++) {
            frame->payload[len] ^= maskingKey[len % MASK_LEN];
        }
    }
}

void ws_create_closing_frame(uint8_t *out_data, int *out_len)
{
    struct ws_frame frame;
    frame.payload_length = 0;
    frame.payload = NULL;
    frame.type = WS_CLOSING_FRAME;
    ws_create_frame(&frame, out_data, out_len);
}

void ws_create_text_frame(const char *text, uint8_t *out_data, int *out_len)
{
    struct ws_frame frame;
    frame.payload_length = strlen(text);
    frame.payload = (uint8_t *)text;
    frame.type = WS_TEXT_FRAME;
    ws_create_frame(&frame, out_data, out_len);
}

void ws_create_binary_frame(const uint8_t *data,uint16_t datalen, uint8_t *out_data, int *out_len)
{
    struct ws_frame frame;
    frame.payload_length = datalen;
    frame.payload = (uint8_t *)data;
    frame.type = WS_BINARY_FRAME;
    ws_create_frame(&frame, out_data, out_len);
}

void ws_create_control_frame(enum wsFrameType type, const uint8_t *data, int data_len, uint8_t *out_data, int *out_len)
{
    struct ws_frame frame;
    frame.payload_length = data_len;
    frame.payload = (uint8_t *)data;
    frame.type = type;
    ws_create_frame(&frame, out_data, out_len);
}

