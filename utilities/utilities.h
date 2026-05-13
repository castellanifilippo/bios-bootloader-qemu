#ifndef UTILITIES32
#define UTILITIES32

/*====== TYPES ======*/
#define NULL ((void*)0)
typedef _Bool bool;
enum {
  false,
  true
};

/*stdint.h style*/
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t uintptr_t;

/*====== IO ======*/
extern void outb(uint8_t data, uint16_t port);
extern void outw(uint16_t data, uint16_t port);
extern void outl(uint32_t data, uint16_t port);

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t inl(uint16_t port);

/*===== MEM ======*/
void *memcpy(void *dest, void *src, uint32_t  count);
void *memset(void *dest, uint8_t val, uint32_t count);

/*====== VIDEO  ======*/
extern volatile uint8_t * const videoMemory;

//ATTRIBUTES ON BLACK
#define BLACKONBLACK          0x00
#define BLUEONBLACK           0x01
#define GREENONBLACK          0x02
#define CYANONBLACK           0x03
#define REDONBLACK            0x04
#define MAGENTAONBLACK        0x05
#define BROWNONBLACK          0x06
#define LIGHTGRAYONBLACK      0x07
#define DARKGRAYONBLACK       0x08
#define LIGHTBLUEONBLACK      0x09
#define LIGHTGREENONBLACK     0x0A
#define LIGHTCYANONBLACK      0x0B
#define LIGHTREDONBLACK       0x0C
#define LIGHTMAGENTAONBLACK   0x0D
#define YELLOWONBLACK         0x0E
#define WHITEONBLACK          0x0F

#define NORMAL WHITEONBLACK
#define SUCCESS GREENONBLACK
#define NUMBER YELLOWONBLACK
#define ERROR REDONBLACK

void print(int8_t cols, int8_t rows, const char *string, const uint8_t attribute);/*printf only works \n*/
void clearScreen(void);/*Fill the screen with blank spaces*/
void intToStr(uint32_t value, char *buffer, int base);/*convert int in string*/

/*print after last character*/
static inline void append(char* stringPointer,uint8_t attribute){
  print(-1,-1,stringPointer,attribute);
}

/*print new line*/
static inline void appendl(char *stringPointer,uint8_t attribute){
  print(-1,0,stringPointer,attribute);
}

/*print integer after last character*/
static inline void appendInt(uint32_t integer,char *buffer,int base,uint8_t attribute){
  intToStr(integer,buffer,base);
  append(buffer,attribute);
}

/*print integer new line*/
static inline void appendIntl(uint32_t integer, char *buffer, int base, uint8_t attribute){
  intToStr(integer,buffer,base);
  appendl(buffer,attribute);
}

/*====== QEMU ======*/

#define QEMU_CFG_SELECT 0x510
#define QEMU_CFG_DATA 0x511
#define QEMU_CFG_RAM_SIZE 0x03

uint64_t get_qemu_ram_size(void);

#endif
