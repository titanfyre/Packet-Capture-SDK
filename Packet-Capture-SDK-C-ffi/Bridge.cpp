#include "SDK/PacketCapture.h"
#include "WireTypes.h"
#include <map>
struct ProductSession{
PacketCapture client;
Json Invoke(const std::string& op,const Json& a){

if(op=="check" || op=="parse_frame"){std::vector<unsigned char> frame;bool valid=ParseEthernetFrame(a.at("hex").get<std::string>(),frame);return {{"valid",valid},{"data",frame}};}
if(op=="authorize"){CheckMessage(client.Authorize(a.at("key").get<std::string>()));return Json::object();}
if(op=="adapters"){std::vector<PC_ADAPTER> list;CheckCode(client.Adapters(list));return VectorWire(list);}
if(op=="start"){CheckCode(client.Start(Identifier(a,"adapter"),Number(a,"directions",PC_INBOUND|PC_OUTBOUND,3)));return Json::object();}
if(op=="stop"){CheckCode(client.Stop());return Json::object();}
if(op=="status"){PC_STATE state{};CheckCode(client.Status(state));return EncodeWire(state);}
if(op=="receive"){std::vector<PC_EVENT> events;CheckCode(client.Receive(events));return VectorWire(events);}
if(op=="send"){CheckCode(client.Send(Bytes(a.at("data"),PC_MAX_FRAME)));return Json::object();}
if(op=="close"){client.Close();return Json::object();}

throw SdkError(50,"Unknown operation");}
};
static Runtime<ProductSession> runtime;
extern "C" __declspec(dllexport) unsigned packet_capture_sdk_json_open(std::uint64_t* id)noexcept{return runtime.Open(id);}
extern "C" __declspec(dllexport) unsigned packet_capture_sdk_json_close(std::uint64_t id)noexcept{return runtime.Close(id);}
extern "C" __declspec(dllexport) char* packet_capture_sdk_json_call(std::uint64_t id,const char* request,std::uint32_t length)noexcept{return runtime.Call(id,request,length);}
extern "C" __declspec(dllexport) void packet_capture_sdk_json_free(char* response)noexcept{delete[] response;}
