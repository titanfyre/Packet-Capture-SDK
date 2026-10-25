#include "../SDK/PacketCapture.h"
#include <iostream>
#include <thread>
#include <chrono>
int main(int argc,char** argv){
 if(argc==2&&std::string_view(argv[1])=="--preview"){std::cout<<"OFFLINE PREVIEW: no adapter opened and no packets captured.\n";return 0;}
 PacketCapture capture;std::vector<PC_ADAPTER> adapters;if(auto e=capture.Adapters(adapters)){std::cerr<<"Driver inventory: "<<PacketCapture::Error(e)<<'\n';return 1;}
 for(const auto& a:adapters)std::wcout<<a.id<<L" : "<<a.name<<L" ("<<(a.running?L"running":L"paused")<<L")\n";
 if(argc!=2||std::string_view(argv[1])!="capture"){std::cout<<"Read-only inventory. Usage: PacketCaptureCLI.exe capture\n";return 0;}
 std::string key;uint64_t selected=0;std::cout<<"Access key: ";std::cin>>key;auto result=capture.Authorize(key);SecureZeroMemory(key.data(),key.size());if(!result.empty()){std::cerr<<result<<'\n';return 1;}
 std::cout<<"Exact adapter ID from inventory: ";std::cin>>selected;if(auto e=capture.Start(selected)){std::cerr<<PacketCapture::Error(e)<<'\n';return 1;}
 std::cout<<"Capture for 10 seconds; packets continue unchanged.\n";auto until=GetTickCount64()+10000;while(GetTickCount64()<until){std::vector<PC_EVENT> batch;if(auto e=capture.Receive(batch)){std::cerr<<PacketCapture::Error(e)<<'\n';return 1;}for(const auto& f:batch)std::cout<<f.sequence<<" "<<(f.direction==PC_INBOUND?"IN ":"OUT ")<<f.length<<" bytes\n";std::this_thread::sleep_for(std::chrono::milliseconds(10));}return capture.Stop()?1:0;
}
