/*
 * kasalua - KASA Smart Devices I/O library for lua
 *
 *   Copyright (C) 2009 TheRootED24 <TheRootED24@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef KASALUA_H
#define KASALUA_H

#ifndef __cplusplus
// LUA LIBS FOR gcc
#include <lua.h>                               
#include <lauxlib.h>                           
#include <lualib.h>
#endif

#ifdef __cplusplus
// LUA LIBS FOR g++
#include <lua.hpp>
extern "C" {
#endif

// STD LIBS
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// SOCKET LIBS
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h> 
#include <netdb.h>

// SEM LIB
#include <semaphore.h>

#ifdef __cplusplus
}
#endif

// GLOBAL DEFINES
#define DEBUG 1
#define KASA_ENCRYPTED_KEY 171

// KASA GET SYSINFO
static const char *get_kasa_info = "{\"system\":{\"get_sysinfo\":null}}";

// KasaIF Interface Class
typedef struct KasaIF
{
    struct sockaddr_in dest_addr;
    const char *ip;
    int sock;
    sem_t mutex;

} KasaIF;

// Device Base Class
typedef struct Device
{
    struct KasaIF kasaIF;

} Device;

// dump lua stack function for debugging
static void dumpstack (lua_State *L);

// LUA FACING FUNCTIONS
static int kasa_init(lua_State *L);
static int kasa_close(lua_State *L);
static int kasa_update_sysinfo(lua_State *L);
static int kasa_scan(lua_State *L);
static int kasa_send_raw_cmd(lua_State *L);

#endif /* KASALUA_H */
