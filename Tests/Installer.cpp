#include "../Installer/Setup.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <objbase.h>
#include <cstring>
int main(){
 wchar_t temp[MAX_PATH]{};if(!GetTempPathW(MAX_PATH,temp))return 1;GUID guid{};if(FAILED(CoCreateGuid(&guid)))return 1;wchar_t name[40]{};StringFromGUID2(guid,name,40);auto folder=std::filesystem::path(temp)/(std::wstring(L"PacketIdentity-")+name);std::filesystem::create_directory(folder);auto path=folder/L"identity.inf";
 struct Cleanup{std::filesystem::path file,folder;~Cleanup(){std::error_code e;std::filesystem::remove(file,e);std::filesystem::remove(folder,e);}} cleanup{path,folder};
 const std::string valid=R"INF([Version]
Signature="$Windows NT$"
Class=NetService
ClassGuid={4D36E974-E325-11CE-BFC1-08002BE10318}
Provider=%Provider%
CatalogFile=TitanPacketCapture.cat
DriverVer=10/25/2026,0.1.0.0
PnpLockdown=1
[Manufacturer]
%Provider%=Models,NTamd64.10.0...19041,NTamd64.10.0...25319
[Models.NTamd64.10.0...19041]
%Device%=InstallNC,titan_packetcapture
[Models.NTamd64.10.0...25319]
%Device%=Install,titan_packetcapture
[Install]
Characteristics=0x40000
NetCfgInstanceId="{F7F23E19-CAB5-4D94-9A2E-58CF86532513}"
CopyFiles=Copy
AddReg=Ndi
[InstallNC]
Characteristics=0x40000
NetCfgInstanceId="{F7F23E19-CAB5-4D94-9A2E-58CF86532513}"
CopyFiles=CopyNC
AddReg=Ndi
[Ndi]
HKR,Ndi,Service,,"TitanPacketCapture"
HKR,Ndi,CoServices,0x00010000,"TitanPacketCapture"
HKR,Ndi,HelpText,,%Device%
HKR,Ndi,FilterClass,,diagnostic
HKR,Ndi,FilterType,0x00010001,2
HKR,Ndi,FilterRunType,0x00010001,2
HKR,Ndi,UnbindOnAttach,0x00010001,0
HKR,Ndi,UnbindOnDetach,0x00010001,0
HKR,Ndi\Interfaces,UpperRange,,"noupper"
HKR,Ndi\Interfaces,LowerRange,,"nolower"
HKR,Ndi\Interfaces,FilterMediaTypes,,"ethernet"
[Install.Services]
AddService=TitanPacketCapture,,Service
[InstallNC.Services]
AddService=TitanPacketCapture,,ServiceNC
[Service]
DisplayName=%Device%
ServiceType=1
StartType=1
ErrorControl=1
ServiceBinary=%13%\TitanPacketCapture.sys
LoadOrderGroup=NDIS
[ServiceNC]
DisplayName=%Device%
ServiceType=1
StartType=1
ErrorControl=1
ServiceBinary=%12%\TitanPacketCapture.sys
LoadOrderGroup=NDIS
[DestinationDirs]
Copy=13
CopyNC=12
[SourceDisksNames]
1=%Device%,,,
[SourceDisksFiles]
TitanPacketCapture.sys=1
[Copy]
TitanPacketCapture.sys,,,2
[CopyNC]
TitanPacketCapture.sys,,,2
[Strings]
Provider="Titanfyre"
Device="Titan Packet Capture SDK"
)INF";auto write=[&](const std::string& text){std::ofstream f(path,std::ios::binary);f<<text;f.close();return !f.fail();};
 if(!write(valid)||!Setup::PackageMatches(path.wstring())){std::cerr<<"Positive identity failed Windows="<<GetLastError()<<"\n";return 1;}unsigned checks=1;
 for(const auto& change:{std::pair{"Class=NetService","Class=Net"},std::pair{"Provider=\"Titanfyre\"","Provider=\"Different\""},std::pair{"TitanPacketCapture.cat","Other.cat"},std::pair{"titan_packetcapture","different_component"},std::pair{"F7F23E19","A7F23E19"}}){auto text=valid;auto offset=text.find(change.first);if(offset==text.npos){std::cerr<<"Missing test mutation: "<<change.first<<"\n";return 1;}text.replace(offset,std::strlen(change.first),change.second);if(!write(text)||Setup::PackageMatches(path.wstring())){std::cerr<<"Mutation incorrectly accepted: "<<change.first<<"\n";return 1;}++checks;}
 if(Setup::CatalogTrusted(path.wstring())||Setup::CatalogTrusted((folder/L"missing.cat").wstring()))return 1;checks+=2;std::cout<<checks<<" read-only installer identity/signature refusal checks passed; no install call made.\n";
}
