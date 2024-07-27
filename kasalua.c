#include "kasalua.h"

#define KASA "kasa"

sem_t mutex;
static char recvbuf[2048] = {0};

static int kasa_init(lua_State *L)
{
    sem_init(&mutex, 0, 1);
    lua_settop(L, 0);
    return 0;
}

static int kasa_close(lua_State *L)
{
    sem_destroy(&mutex);
    lua_settop(L, 0);
    return 0;
}

static void dumpstack (lua_State *L) {
  int top=lua_gettop(L);
  for (int i = 1; i <= top; i++) {
    printf("%d\t%s\t", i, luaL_typename(L,i));
    switch (lua_type(L, i)) {
      case LUA_TNUMBER:
        printf("%g\n",lua_tonumber(L,i));
        break;
      case LUA_TSTRING:
        printf("%s\n",lua_tostring(L,i));
        break;
      case LUA_TBOOLEAN:
        printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
        break;
      case LUA_TNIL:
        printf("%s\n", "nil");
        break;
      default:
        printf("%p\n",lua_topointer(L,i));
        break;
    }
  }
}

static uint16_t kasa_encrypt(const char *data, int length, uint8_t addLengthByte, char *encryped_data)
{
    uint8_t key = KASA_ENCRYPTED_KEY;
    uint8_t en_b;
    int index = 0;

    if (addLengthByte)
    {
        encryped_data[index++] = 0;
        encryped_data[index++] = 0;
        encryped_data[index++] = (char)(length >> 8);
        encryped_data[index++] = (char)(length & 0xFF);
    }

    for (int i = 0; i < length; i++)
    {
        en_b = data[i] ^ key;
        encryped_data[index++] = en_b;
        key = en_b;
    }
    return index;
}

static uint16_t kasa_decrypt(char *data, int length, char *decryped_data, int startIndex)
{
    uint8_t key = KASA_ENCRYPTED_KEY;
    uint8_t dc_b;
    int retLength = 0;
    // printf("%s\n", decryped_data);

    for (int i = startIndex; i < length; i++)
    {
        dc_b = data[i] ^ key;
        key = data[i];
        decryped_data[retLength++] = dc_b;
    }
    decryped_data[retLength] = 0;
    //printf("RECIEVED: %d\n", retLength);
    //printf("\n%s\n", decryped_data);

    return retLength;
}

static void close_sock(int sock)
{
    if (sock != -1)
    {
        shutdown(sock, 0);
        close(sock);
    }
}

static bool open_sock(Device *dev)
{
    int err;
    dev->kasaIF.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    fd_set fdset;
    struct timeval tv;
    int arg;

    if (dev->kasaIF.sock < 0)
    {
        perror("Error unable to open socket...\n");
        return false;
    }

    // Using non blocking connect
    arg = fcntl(dev->kasaIF.sock, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(dev->kasaIF.sock, F_SETFL, O_NONBLOCK);
    err = connect(dev->kasaIF.sock, (struct sockaddr *)&dev->kasaIF.dest_addr, sizeof(dev->kasaIF.dest_addr));

    if (err < 0)
    {
        do
        {
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            FD_ZERO(&fdset);
            FD_SET(dev->kasaIF.sock, &fdset);

            // Set connect timeout to 1 sec.
            err = select(dev->kasaIF.sock + 1, NULL, &fdset, NULL, &tv);
            if (err < 0 && errno != EINTR)
            {
                printf("Unable to open sock errno: %d\n", errno);
                break;
            }

            if (err == 1)
            {
                int so_error = 0;
                socklen_t len = sizeof so_error;

                getsockopt(dev->kasaIF.sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0)
                {
                    arg &= (~O_NONBLOCK);
                    fcntl(dev->kasaIF.sock, F_SETFL, arg);
                    return true;
                }
                else
                    break;
            }
        } while (1);
    }
    perror("Error can not open sock...\n");
    close_sock(dev->kasaIF.sock);

    return false;
}

static bool read_fully(int fd, void *buf, size_t n) {
	while (n) {
		ssize_t r = read(fd, buf, n);
		if (r < 0) {
			perror("Socket Read Error");
            return false;
		}
		buf = (unsigned char *) buf + r;
		n -= r;
	}
    return true;
}

static void writev_fully(int fd, struct iovec iov[], int iovcnt) {
	for (struct iovec *end = iov + iovcnt; iov != end;) {
		ssize_t w = writev(fd, iov, (int) (end - iov));
		if (w < 0) {
			perror("write");
		}
		for (; w > 0; ++iov) {
			if (iov->iov_len > (size_t) w) {
				iov->iov_base = (char *) iov->iov_base + w;
				iov->iov_len -= w;
				break;
			}
			w -= iov->iov_len;
			iov->iov_base = (char *) iov->iov_base + iov->iov_len;
			iov->iov_len = 0;
		}
	}
}

static void update_device_ip_address(Device *dev, const char *ip)
{
    if (strcmp(dev->kasaIF.ip, ip) != 0)
    {
        dev->kasaIF.ip = ip;
    }
    dev->kasaIF.dest_addr.sin_addr.s_addr = inet_addr(dev->kasaIF.ip);
    dev->kasaIF.sock = -1;
    dev->kasaIF.dest_addr.sin_family = AF_INET;
    dev->kasaIF.dest_addr.sin_port = htons(9999);
    return;
}

static int send_cmd(Device *dev, const char *cmd) {
    
    update_device_ip_address(dev, dev->kasaIF.ip);
    bool sock_open = open_sock(dev);

    if(!sock_open)
    {
        
        perror("failed to open socket");
        return 1;
    }

	struct timeval timeout = { .tv_sec = 1 };

	if (setsockopt(dev->kasaIF.sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
		perror("setsockopt");
        close_sock(dev->kasaIF.sock);
	}

	char buf[2048] = {0}, *p = buf;
    int sze = strlen(cmd);

	strcpy(p, cmd);
	p[sze+1] = 0;

    bool full_read = false;

	if ((char*)p == buf)  {
		size_t n = (size_t) sze;
		uint32_t len_be = htonl((uint32_t) n);
        char outbuf[2048] = {0};
        kasa_encrypt(buf, n, 0, outbuf);

		struct iovec iov[2] = {
			{ .iov_base = &len_be, .iov_len = sizeof len_be },
			{ .iov_base = outbuf, .iov_len = n }
		};
		writev_fully(dev->kasaIF.sock, iov, sizeof iov / sizeof *iov);
		full_read = read_fully(dev->kasaIF.sock, &len_be, sizeof len_be);
        if(!full_read)
        {
            fprintf(stderr, "failed to read full message [ code: %d ]\n", full_read);
            return 1;
        }
		n = ntohl(len_be);

		if (n > sizeof buf) {
            close_sock(dev->kasaIF.sock);
			return 1;
		}
		full_read = read_fully(dev->kasaIF.sock, buf, n);
        if(full_read)
        {
            memset(outbuf, 0, sizeof(outbuf));
            kasa_decrypt(buf, n, outbuf, 0 );
            strncpy(recvbuf, outbuf, n);
            recvbuf[n] = '\0';
        }
        else
        {
            close_sock(dev->kasaIF.sock);
            sem_post(&dev->kasaIF.mutex);
            sem_wait(&dev->kasaIF.mutex);
            send_cmd(dev, cmd);
        }
        p = buf;
	}
    close_sock(dev->kasaIF.sock);
    return (int)strlen(buf);
}

static int kasa_send_raw_cmd(lua_State *L) 
{
    //dumpstack(L);
    Device dev = {0};
    dev.kasaIF.ip = luaL_checkstring(L, -2);
    dev.kasaIF.mutex = mutex;
    const char *cmd = luaL_checkstring(L, -1);
    
    sem_wait(&dev.kasaIF.mutex);
    send_cmd(&dev, cmd);
    sem_post(&dev.kasaIF.mutex);
    lua_pushstring(L, recvbuf);

    return 1;
}

static int kasa_update_sysinfo(lua_State *L)
{
    Device dev = {0};
    dev.kasaIF.ip = luaL_checkstring(L, -1);
    dev.kasaIF.mutex = mutex;

    sem_wait(&dev.kasaIF.mutex);
    send_cmd(&dev, get_kasa_info);
    sem_post(&dev.kasaIF.mutex);
    lua_pushstring(L, recvbuf);

    return 1;
}

static int kasa_scan(lua_State *L)
{
    const char *bcast = luaL_checkstring(L, -3);
    uint32_t timeoutMs = luaL_checkint(L, -2);
    uint8_t is_query = lua_tonumber(L, -1) > 0 ? lua_tonumber(L, -1) : 0;
    uint8_t idx = 1;
	
    // invalid arguments .. bail
    if(!bcast || !timeoutMs ){
      printf("Scan2 Invalid Arguments !!\n");
      return 0;
    }
    // clear the stack
    lua_settop(L, 0);

    // begin the scan 
    if (DEBUG > 0 )
    {
        printf("BROADCASTING ON IP: %s\n", bcast);
    }

    struct sockaddr_in dest_addr;
    int ret = 0;
    int boardCaseEnable = 1;
    int retValue = 0;
    int sock;

    int err = 1;
    char sendbuf[128];
    int len;

    len = strlen(get_kasa_info);
    dest_addr.sin_addr.s_addr = inet_addr(bcast);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(9999);
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (sock < 0)
    {
        printf("Unable to create socket  [ errno: %d ]\n", errno);
        retValue = -1;
        close_sock(sock);
        return retValue;
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boardCaseEnable, sizeof(boardCaseEnable));
    if (ret < 0)
    {
        printf("Unable to set broadcase option  [ ernno: %d ]\n", errno);
        retValue = -2;
        close_sock(sock);
        return retValue;
    }
    
    len = kasa_encrypt(get_kasa_info, len, 0, sendbuf);
    if ((unsigned int)len > sizeof(sendbuf))
    {
        perror("Overflowed multicast sendfmt buffer!!\n");
        retValue = -3;
        close_sock(sock);
        return retValue;
    }

    err = sendto(sock, sendbuf, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        printf("Error occurred during sending [ errno %d ]\n", errno);
        close_sock(sock);
        return -4;
    }

    int send_loop = 0;
    long time_out_us = (long)timeoutMs * 1000;
    
    lua_newtable(L); 
    {
        while ((err > 0) && (send_loop < 1))
        {
            struct timeval tv = {
                .tv_sec = 0,
                .tv_usec = time_out_us,
            };
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);

            int s = select(sock + 1, &rfds, NULL, NULL, &tv);

            if (s < 0)
            {
                printf("Select failed  [ errno %d ]\n", errno);
                err = -1;
                break;
            }
            else if (s > 0)
            {
                if (FD_ISSET(sock, &rfds))
                {
                    char recvbuf[2048];
                    char raddr_name[32] = {0};
                    static char *ip;

                    struct sockaddr_storage raddr;
                    socklen_t socklen = sizeof(raddr);
                    int len = recvfrom(sock, recvbuf, sizeof(recvbuf) - 1, 0,
                                    (struct sockaddr *)&raddr, &socklen);
                    if (len < 0)
                    {
                        printf("multicast recvfrom failed  [ errno %d ]\n", errno);
                        err = -1;
                        break;
                    }
                    else
                    {
                        len = kasa_decrypt(recvbuf, len, recvbuf, 0);
                    }

                    if (raddr.ss_family == PF_INET)
                    {
                        ip = inet_ntoa(((struct sockaddr_in *)&raddr)->sin_addr);
                        strcpy(raddr_name, ip);
                    }

                    recvbuf[len] = 0;
                    {
                            lua_pushinteger(L, idx++);
                            lua_newtable(L);
                            { 
                                lua_pushstring(L, raddr_name);
                                lua_setfield(L, -2, "ip");
                                lua_pushstring(L, recvbuf);
                                lua_setfield(L, -2, "sysinfo");
                                dumpstack(L);   
                            }
                            lua_settable(L, -3);
                            dumpstack(L);
                    }
                }
                else
                {
                    err = sendto(sock, sendbuf, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    if (err < 0)
                    {
                        printf("Error occurred during sending  [ errno %d ]\n", errno);
                        retValue = -5;
                    }
                }
            }
            else if (s == 0)
            {
                send_loop++;
                err = sendto(sock, sendbuf, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0)
                {
                    printf("Error occurred during sending [ errno %d ]\n", errno);
                    retValue = -1;
                    close_sock(sock);
                    lua_pushnil(L);
                    return 1;
                }
            }
            usleep(300000);
            
        
            if (is_query > 0)
            {
                close_sock(sock);
                return 1;
            }
        }
        
        close_sock(sock);
    }
    
    lua_setglobal(L, "devices");
    lua_getglobal(L, "devices");
    
    dumpstack(L);
    
    return 1;
}

// GAURDS FOR g++ to prevent name mangling during linking
#ifdef __cplusplus
extern "C" {
#endif

static const luaL_reg kasa_methods[] = {
    { "init",		    kasa_init           },
	{ "scan",			kasa_scan           },
	{ "send",			kasa_send_raw_cmd   },
    { "update",		    kasa_update_sysinfo },
    { "close",		    kasa_close          },
	

	{ }
};

int luaopen_kasa(lua_State *L)
{
	luaL_register(L, KASA, kasa_methods);

	return 1;
}

#ifdef __cplusplus
}
#endif
