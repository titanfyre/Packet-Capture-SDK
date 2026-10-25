#include "../SDK/PacketCapture.h"
#include <iostream>
#include <thread>
#include <chrono>
int main(){
 std::cout<<"Select one adapter, optionally send an explicit Ethernet frame, and observe for 10 seconds.\n";
 PacketCapture capture;std::vector<PC_ADAPTER> adapters;
 if(auto e=capture.Adapters(adapters)){std::cerr<<PacketCapture::Error(e);return 1;}
 for(const auto& adapter:adapters)std::wcout<<adapter.id<<L" : "<<adapter.name<<L'\n';
 std::string key;uint64_t id=0;std::cout<<"Access key and exact adapter ID: ";std::cin>>key>>id;
 auto error=capture.Authorize(key);SecureZeroMemory(key.data(),key.size());
 if(!error.empty()){std::cerr<<error;return 1;}
 if(auto e=capture.Start(id)){std::cerr<<PacketCapture::Error(e);return 1;}
 std::cout<<"Optional hex Ethernet frame to send (Enter leaves traffic unchanged): ";
 std::cin.ignore(10000,'\n');std::string hex;std::getline(std::cin,hex);
 if(!hex.empty()){
  std::vector<unsigned char> frame;if(!ParseEthernetFrame(hex,frame)){std::cerr<<"Invalid complete 14..1518-byte frame.\n";return 1;}
  if(auto e=capture.Send(frame)){std::cerr<<PacketCapture::Error(e);return 1;}
 }
 auto end=GetTickCount64()+10000;
 while(GetTickCount64()<end){
  std::vector<PC_EVENT> frames;
  if(auto e=capture.Receive(frames)){std::cerr<<PacketCapture::Error(e);return 1;}
  for(const auto& frame:frames)std::cout<<(frame.direction==PC_INBOUND?"IN ":"OUT ")<<frame.length<<" Ethernet bytes\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
 }
} // Scope exit closes the owner, stops capture and clears queued records.
