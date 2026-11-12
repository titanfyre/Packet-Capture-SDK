#pragma once
#ifdef _KERNEL_MODE
#include <ntddk.h>
typedef ULONG PC_U32;typedef ULONGLONG PC_U64;typedef UCHAR PC_U8;typedef WCHAR PC_WCHAR;
#else
#include <windows.h>
#include <winioctl.h>
#include <stdint.h>
typedef uint32_t PC_U32;typedef uint64_t PC_U64;typedef uint8_t PC_U8;typedef wchar_t PC_WCHAR;
#endif
#define PC_VERSION 1u
#define PC_MAX_FRAME 1518u
#define PC_RING_CAPACITY 256u
#define PC_MAX_ADAPTERS 32u
#define PC_BATCH_CAPACITY 16u
#define PC_OUTBOUND 1u
#define PC_INBOUND 2u
#define PC_IOCTL(n) CTL_CODE(FILE_DEVICE_NETWORK,0x900+(n),METHOD_BUFFERED,FILE_READ_DATA|FILE_WRITE_DATA)
#define PC_ENUM PC_IOCTL(0)
#define PC_STATUS PC_IOCTL(1)
#define PC_START PC_IOCTL(2)
#define PC_STOP PC_IOCTL(3)
#define PC_READ PC_IOCTL(4)
#define PC_SEND PC_IOCTL(5)
#define PC_CHALLENGE PC_IOCTL(6)
#define PC_AUTHORIZE PC_IOCTL(7)
typedef struct PC_ADAPTER {PC_U64 id,luid;PC_U32 ifindex,running;PC_U8 mac[6],reserved[2];PC_WCHAR name[128];} PC_ADAPTER;
typedef struct PC_ADAPTERS {PC_U32 version,count;PC_ADAPTER adapters[PC_MAX_ADAPTERS];} PC_ADAPTERS;
typedef struct PC_SELECTION {PC_U64 adapter;PC_U32 directions,reserved;} PC_SELECTION;
typedef struct PC_STATE {PC_U32 version,size,active,authorized;PC_U64 selected,captured,overwritten,sent,send_failed,frequency;PC_U32 queued,pending_sends;} PC_STATE;
typedef struct PC_EVENT {PC_U64 sequence,qpc,adapter;PC_U32 direction,length,original_length,reserved;PC_U8 data[PC_MAX_FRAME],padding[2];} PC_EVENT;
typedef struct PC_BATCH {PC_U32 version,count;PC_STATE state;PC_EVENT events[PC_BATCH_CAPACITY];} PC_BATCH;
typedef struct PC_FRAME {PC_U32 length,reserved;PC_U8 data[PC_MAX_FRAME],padding[2];} PC_FRAME;
#ifdef __cplusplus
static_assert(sizeof(PC_FRAME)==1528);static_assert(sizeof(PC_EVENT)==1560);static_assert(sizeof(PC_ADAPTER)==288);static_assert(sizeof(PC_STATE)==72);
#else
C_ASSERT(sizeof(PC_FRAME)==1528);C_ASSERT(sizeof(PC_EVENT)==1560);C_ASSERT(sizeof(PC_ADAPTER)==288);C_ASSERT(sizeof(PC_STATE)==72);
#endif
