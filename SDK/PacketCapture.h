#pragma once
#include "PacketProtocol.h"
#include "AuthorizationProtocol.h"
#include <string>
#include <string_view>
#include <vector>
// Serialize calls on one session. It owns the exclusive administrator control handle.
class PacketCapture {
 HANDLE handle_=INVALID_HANDLE_VALUE;
 DWORD Call(DWORD,const void*,DWORD,void*,DWORD) noexcept;
public:
 PacketCapture()=default;~PacketCapture(){Close();}
 PacketCapture(const PacketCapture&)=delete;PacketCapture& operator=(const PacketCapture&)=delete;
 void Close() noexcept;
 DWORD Adapters(std::vector<PC_ADAPTER>&) noexcept;
 DWORD Status(PC_STATE&) noexcept;
 std::string Authorize(std::string_view key);
 DWORD Start(uint64_t adapter,uint32_t directions=PC_INBOUND|PC_OUTBOUND) noexcept;
 DWORD Stop() noexcept;
 DWORD Receive(std::vector<PC_EVENT>&) noexcept;
 DWORD Send(const std::vector<unsigned char>&) noexcept;
 static std::string Error(DWORD);
};
bool ParseEthernetFrame(std::string_view,std::vector<unsigned char>&);
