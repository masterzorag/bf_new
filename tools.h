#ifndef TYPES_H__
#define TYPES_H__

//#include <stdint.h>

typedef unsigned char u8;
typedef unsigned int u32;
typedef char s8;
typedef unsigned long long int u64;

/*
typedef uint64_t  u64;
typedef uint32_t   u32;
typedef uint16_t u16;
typedef uint8_t  u8;
*/

u64 _x_to_u64(const s8 *hex){
        u64 t = 0, res = 0;
        u32 len = strlen(hex);
        char c;

        while(len--){
                c = *hex++;
                if(c >= '0' && c <= '9')        t = c - '0';
                else if(c >= 'a' && c <= 'f')   t = c - 'a' + 10;
                else if(c >= 'A' && c <= 'F')   t = c - 'A' + 10;
                else                            t = 0;
                res |= t << (len * 4);
        }

        return res;
}

u8 *_x_to_u8_buffer(const s8 *hex){
        u32 len = strlen(hex);
        if(len % 2 != 0) return NULL;   // (add sanity check in caller)

        s8 xtmp[3] = {0, 0, 0};
        u8 *res = (u8 *)malloc(sizeof(u8) * len);
        u8 *ptr = res;

        while(len--){
                xtmp[0] = *hex++;
                xtmp[1] = *hex++;
                *ptr++ = (u8) _x_to_u64(xtmp);
        }

        return res;
}

#endif
