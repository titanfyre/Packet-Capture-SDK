#include "PacketCapture.h"
#include <cstring>
#include <cctype>
namespace {std::string RequestDriverAuthorization(std::string_view,const TITAN_AUTHORIZATION_CHALLENGE&,TITAN_AUTHORIZATION_TOKEN&);}
void PacketCapture::Close() noexcept{if(handle_!=INVALID_HANDLE_VALUE){CloseHandle(handle_);handle_=INVALID_HANDLE_VALUE;}}
DWORD PacketCapture::Call(DWORD code,const void* in,DWORD n,void* out,DWORD size) noexcept {
 if(handle_==INVALID_HANDLE_VALUE){handle_=CreateFileW(L"\\\\.\\TitanPacketCapture",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);if(handle_==INVALID_HANDLE_VALUE)return GetLastError();}
 DWORD got=0;if(!DeviceIoControl(handle_,code,const_cast<void*>(in),n,out,size,&got,nullptr)){auto error=GetLastError();if(error==ERROR_DEVICE_NOT_CONNECTED||error==ERROR_DELETE_PENDING||error==ERROR_INVALID_HANDLE)Close();return error;}return got==size?ERROR_SUCCESS:ERROR_INVALID_DATA;
}
DWORD PacketCapture::Adapters(std::vector<PC_ADAPTER>& output) noexcept {
 output.clear();PC_ADAPTERS data{};auto error=Call(PC_ENUM,nullptr,0,&data,sizeof(data));if(error)return error;if(data.version!=PC_VERSION||data.count>PC_MAX_ADAPTERS)return ERROR_INVALID_DATA;
 try{for(ULONG i=0;i<data.count;++i){const auto& a=data.adapters[i];if(!a.id||a.running>1||a.name[127]){output.clear();return ERROR_INVALID_DATA;}output.push_back(a);}}catch(...){output.clear();return ERROR_NOT_ENOUGH_MEMORY;}return ERROR_SUCCESS;
}
DWORD PacketCapture::Status(PC_STATE& state) noexcept {state={};auto e=Call(PC_STATUS,nullptr,0,&state,sizeof(state));if(!e&&(state.version!=PC_VERSION||state.size!=sizeof(state)||state.queued>PC_RING_CAPACITY||state.pending_sends>32||!state.frequency||state.active>1||state.authorized>1))e=ERROR_INVALID_DATA;if(e)state={};return e;}
std::string PacketCapture::Authorize(std::string_view key){TITAN_AUTHORIZATION_CHALLENGE challenge{};TITAN_AUTHORIZATION_TOKEN token{};if(auto e=Call(PC_CHALLENGE,nullptr,0,&challenge,sizeof(challenge)))return "Driver challenge: "+Error(e);auto result=RequestDriverAuthorization(key,challenge,token);if(!result.empty())return result;auto e=Call(PC_AUTHORIZE,&token,sizeof(token),nullptr,0);SecureZeroMemory(&token,sizeof(token));return e?"Driver authorization: "+Error(e):std::string{};}
DWORD PacketCapture::Start(uint64_t id,uint32_t directions) noexcept {if(!id||!directions||(directions&~3u))return ERROR_INVALID_PARAMETER;PC_SELECTION selected{id,directions,0};return Call(PC_START,&selected,sizeof(selected),nullptr,0);}
DWORD PacketCapture::Stop() noexcept{return Call(PC_STOP,nullptr,0,nullptr,0);}
DWORD PacketCapture::Receive(std::vector<PC_EVENT>& output) noexcept {output.clear();PC_BATCH data{};auto e=Call(PC_READ,nullptr,0,&data,sizeof(data));if(e)return e;if(data.version!=PC_VERSION||data.count>PC_BATCH_CAPACITY||data.state.size!=sizeof(PC_STATE))return ERROR_INVALID_DATA;
 try{for(ULONG i=0;i<data.count;++i){auto& f=data.events[i];if(f.length<14||f.length>PC_MAX_FRAME||f.original_length<f.length||(f.direction!=PC_INBOUND&&f.direction!=PC_OUTBOUND)||f.reserved){output.clear();return ERROR_INVALID_DATA;}output.push_back(f);}}catch(...){output.clear();return ERROR_NOT_ENOUGH_MEMORY;}return ERROR_SUCCESS;
}
DWORD PacketCapture::Send(const std::vector<unsigned char>& bytes) noexcept {if(bytes.size()<14||bytes.size()>PC_MAX_FRAME)return ERROR_BAD_LENGTH;PC_FRAME frame{};frame.length=static_cast<ULONG>(bytes.size());std::memcpy(frame.data,bytes.data(),bytes.size());return Call(PC_SEND,&frame,sizeof(frame),nullptr,0);}
std::string PacketCapture::Error(DWORD e){char* text=nullptr;FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,nullptr,e,MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),reinterpret_cast<char*>(&text),0,nullptr);std::string s=text?text:"Windows operation failed";if(text)LocalFree(text);while(!s.empty()&&(s.back()=='\r'||s.back()=='\n'))s.pop_back();return s+" (Windows error "+std::to_string(e)+")";}
bool ParseEthernetFrame(std::string_view hex,std::vector<unsigned char>& output){output.clear();if(hex.size()>PC_MAX_FRAME*3u)return false;int high=-1;for(char raw:hex){unsigned char c=static_cast<unsigned char>(raw);if(std::isspace(c)||c=='-'||c==':'){if(high!=-1){output.clear();return false;}continue;}int value=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;if(value<0){output.clear();return false;}if(high<0)high=value;else{output.push_back(static_cast<unsigned char>(high*16+value));high=-1;if(output.size()>PC_MAX_FRAME){output.clear();return false;}}}if(high!=-1||output.size()<14){output.clear();return false;}return true;}

#include "PacketCapture.h"
#include <winhttp.h>
#include <wincrypt.h>
#include <array>
#include <cstring>
#include <utility>
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"crypt32.lib")

namespace {
struct Http {
    HINTERNET value = nullptr;
    explicit Http(HINTERNET handle) : value(handle) {}
    ~Http() { if (value) WinHttpCloseHandle(value); }
    Http(const Http&) = delete;
    Http& operator=(const Http&) = delete;
};
std::string Failure(const char* stage) {
    return std::string(stage) + " failed (Windows error " +
        std::to_string(GetLastError()) + ").";
}
bool ValidKey(std::string_view key) {
    if (key.size() != 81 || key.substr(0, 5) != "app1_" || key[37] != '_')
        return false;
    for (size_t i = 5; i != 37; ++i)
        if (!((key[i] >= '0' && key[i] <= '9') ||
              (key[i] >= 'a' && key[i] <= 'f'))) return false;
    for (size_t i = 38; i != key.size(); ++i)
        if (!((key[i] >= 'a' && key[i] <= 'z') ||
              (key[i] >= 'A' && key[i] <= 'Z') ||
              (key[i] >= '0' && key[i] <= '9') ||
               key[i] == '_' || key[i] == '-')) return false;
    return true;
}
}

namespace {
std::string RequestDriverAuthorization(std::string_view key,
    const TITAN_AUTHORIZATION_CHALLENGE& challenge,
    TITAN_AUTHORIZATION_TOKEN& token) {
    token = {};
    if (!ValidKey(key)) return "Enter a valid application access key (app1_...).";
    if (challenge.magic != TITAN_AUTHORIZATION_MAGIC ||
        challenge.version != TITAN_AUTHORIZATION_VERSION ||
        challenge.duration_seconds != 0 ||
        challenge.product != TITAN_AUTHORIZATION_PACKET_CAPTURE)
        return "The driver returned an invalid authorization challenge.";
    const char* sku = "packet-capture-sdk";
    std::string nonce;
    constexpr char hex[] = "0123456789abcdef";
    for (unsigned char value : challenge.nonce) {
        nonce += hex[value >> 4]; nonce += hex[value & 15];
    }
    std::string body = "{\"key\":\"" + std::string(key) +
        "\",\"sku\":\"" + sku + "\",\"nonce\":\"" + nonce + "\"}";
    Http session(WinHttpOpen(L"Titan Driver SDK/0.1",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session.value) { SecureZeroMemory(body.data(), body.size()); return Failure("HTTPS initialization"); }
    WinHttpSetTimeouts(session.value, 5000, 5000, 10000, 10000);
    Http connection(WinHttpConnect(session.value, L"licences.configcsa.com",
        INTERNET_DEFAULT_HTTPS_PORT, 0));
    if (!connection.value) { SecureZeroMemory(body.data(), body.size()); return Failure("HTTPS connection"); }
    Http request(WinHttpOpenRequest(connection.value, L"POST",
        L"/portfolio/api/v1/driver-authorize", nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE));
    if (!request.value) { SecureZeroMemory(body.data(), body.size()); return Failure("Authorization request"); }
    DWORD disabled = WINHTTP_DISABLE_REDIRECTS | WINHTTP_DISABLE_COOKIES;
    if (!WinHttpSetOption(request.value, WINHTTP_OPTION_DISABLE_FEATURE,
        &disabled, sizeof(disabled))) {
        SecureZeroMemory(body.data(), body.size()); return Failure("HTTPS policy");
    }
    const BOOL sent = WinHttpSendRequest(request.value,
        L"Content-Type: application/json\r\n", static_cast<DWORD>(-1),
        body.data(), static_cast<DWORD>(body.size()),
        static_cast<DWORD>(body.size()), 0);
    const DWORD sendError = sent ? ERROR_SUCCESS : GetLastError();
    SecureZeroMemory(body.data(), body.size());
    if (!sent) return "Authorization request failed (Windows error " + std::to_string(sendError) + ").";
    if (!WinHttpReceiveResponse(request.value, nullptr)) return Failure("Authorization response");
    DWORD status = 0, length = sizeof(status);
    if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status,
        &length, WINHTTP_NO_HEADER_INDEX)) return Failure("HTTP status");
    std::string response;
    std::array<char, 1024> chunk{};
    for (;;) {
        DWORD count = 0;
        if (!WinHttpReadData(request.value, chunk.data(),
            static_cast<DWORD>(chunk.size()), &count)) return Failure("Response read");
        if (!count) break;
        if (response.size() + count > 8192) return "Authorization response is too large.";
        response.append(chunk.data(), count);
    }
    if (status != 200) {
        for (char& c : response) if (static_cast<unsigned char>(c) < 32) c = ' ';
        return "License server refused authorization (HTTP " +
            std::to_string(status) + "): " + response;
    }
    // Only extract a bounded base64 field. The kernel verifies every byte of
    // the decoded permission and its signature before enabling any producer.
    const auto field = response.find("\"grant_base64\"");
    if (field == std::string::npos) return "Missing authorization grant in server response.";
    const auto colon = response.find(':', field + 14);
    const auto start = colon == std::string::npos ? std::string::npos : response.find('"', colon + 1);
    const auto end = start == std::string::npos ? std::string::npos : response.find('"', start + 1);
    if (end == std::string::npos || end - start - 1 != 152)
        return "Invalid authorization grant encoding.";
    const std::string grant = response.substr(start + 1, end - start - 1);
    DWORD bytes = sizeof(token);
    if (!CryptStringToBinaryA(grant.c_str(), static_cast<DWORD>(grant.size()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_STRICT,
        reinterpret_cast<BYTE*>(&token), &bytes, nullptr, nullptr) ||
        bytes != sizeof(token)) {
        token = {}; return "Invalid authorization grant length or encoding.";
    }
    if (token.permission.magic != challenge.magic ||
        token.permission.version != challenge.version ||
        token.permission.product != challenge.product ||
        token.permission.duration_seconds == 0 ||
        token.permission.duration_seconds > TITAN_AUTHORIZATION_MAX_SECONDS ||
        std::memcmp(token.permission.nonce, challenge.nonce, sizeof(challenge.nonce))) {
        token = {}; return "Authorization grant does not match this driver session.";
    }
    return {};
}

} // private HTTPS implementation
