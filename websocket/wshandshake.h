/* wshandshake.h - websocket lib
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

#ifndef WSHANDSHAKE_H
#define WSHANDSHAKE_H


#define WS_HDR_KEY "Sec-WebSocket-Key"
#define WS_HDR_VER "Sec-WebSocket-Version"
#define WS_HDR_ACP "Sec-WebSocket-Accept"
#define WS_HDR_ORG "Origin"
#define WS_HDR_HST "Host"
#define WS_HDR_UPG "Upgrade"
#define WS_HDR_CON "Connection"

struct http_header {
    char method[4];
    char uri[128];
    char key[32];
    unsigned char version;
    unsigned char upgrade;
    unsigned char websocket;
    enum wsFrameType type;
};

int ws_handshake(struct http_header *header, uint8_t *in_buf, int in_len, int *out_len);


#endif /* WSHANDSHAKE_H */
