#include "Setup.h"
#include <setupapi.h>
#include <newdev.h>
#include <netcfgx.h>
#include <netcfgn.h>
#include <devguid.h>
#include <wintrust.h>
#include <softpub.h>
#include <wincrypt.h>
#include <mscat.h>
#include <sddl.h>
#include <objbase.h>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <set>
namespace fs=std::filesystem;
namespace {
std::wstring Message(DWORD e){wchar_t* s=nullptr;FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,nullptr,e,MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),reinterpret_cast<LPWSTR>(&s),0,nullptr);std::wstring r=s?s:L"Operation failed";if(s)LocalFree(s);return r+L" (Windows error "+std::to_wstring(e)+L")";}
void Require(bool ok){if(!ok)throw std::system_error(static_cast<int>(GetLastError()?GetLastError():ERROR_GEN_FAILURE),std::system_category());}
bool MicrosoftCatalog(const fs::path& path){
    WINTRUST_FILE_INFO file{};file.cbStruct=sizeof(file);file.pcwszFilePath=path.c_str();
    WINTRUST_DATA data{};data.cbStruct=sizeof(data);data.dwUIChoice=WTD_UI_NONE;data.fdwRevocationChecks=WTD_REVOKE_NONE;data.dwUnionChoice=WTD_CHOICE_FILE;data.pFile=&file;data.dwStateAction=WTD_STATEACTION_VERIFY;data.dwProvFlags=WTD_CACHE_ONLY_URL_RETRIEVAL;
    GUID action=WINTRUST_ACTION_GENERIC_VERIFY_V2;LONG status=WinVerifyTrust(nullptr,&action,&data);bool accepted=false;
    if(status==ERROR_SUCCESS){
        auto provider=WTHelperProvDataFromStateData(data.hWVTStateData);
        auto signer=provider?WTHelperGetProvSignerFromChain(provider,0,FALSE,0):nullptr;
        if(signer&&signer->csCertChain&&signer->pChainContext){
            wchar_t name[256]{},organization[256]{};auto certificate=signer->pasCertChain[0].pCert;
            CertGetNameStringW(certificate,CERT_NAME_ATTR_TYPE,0,const_cast<char*>(szOID_COMMON_NAME),name,256);
            CertGetNameStringW(certificate,CERT_NAME_ATTR_TYPE,0,const_cast<char*>(szOID_ORGANIZATION_NAME),organization,256);
            CERT_CHAIN_POLICY_PARA policy{};policy.cbSize=sizeof(policy);
            CERT_CHAIN_POLICY_STATUS verdict{};verdict.cbSize=sizeof(verdict);
            bool microsoftRoot=CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_MICROSOFT_ROOT,signer->pChainContext,&policy,&verdict)&&verdict.dwError==ERROR_SUCCESS;
            if(!microsoftRoot){policy.dwFlags=MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;verdict.dwError=ERROR_SUCCESS;microsoftRoot=CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_MICROSOFT_ROOT,signer->pChainContext,&policy,&verdict)&&verdict.dwError==ERROR_SUCCESS;}
            accepted=microsoftRoot&&!_wcsicmp(name,L"Microsoft Windows Hardware Compatibility Publisher")&&!_wcsicmp(organization,L"Microsoft Corporation");
        }
    }
    data.dwStateAction=WTD_STATEACTION_CLOSE;WinVerifyTrust(nullptr,&action,&data);return accepted;
}
bool CatalogMember(const fs::path& directory,const wchar_t* name){
    const auto path=directory/name;const auto catalog=directory/L"TitanPacketCapture.cat";
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    struct File{HANDLE h;~File(){CloseHandle(h);}} close{file};
    HCATADMIN admin=nullptr;if(!CryptCATAdminAcquireContext2(&admin,nullptr,L"SHA256",nullptr,0))return false;
    struct Admin{HCATADMIN h;~Admin(){CryptCATAdminReleaseContext(h,0);}} release{admin};
    BYTE hash[64]{};DWORD bytes=sizeof(hash);if(!CryptCATAdminCalcHashFromFileHandle2(admin,file,&bytes,hash,0)||bytes>64)return false;
    wchar_t tag[129]{};const wchar_t* hex=L"0123456789ABCDEF";for(DWORD i=0;i<bytes;++i){tag[i*2]=hex[hash[i]>>4];tag[i*2+1]=hex[hash[i]&15];}
    WINTRUST_CATALOG_INFO member{};member.cbStruct=sizeof(member);member.pcwszCatalogFilePath=catalog.c_str();member.pcwszMemberTag=tag;member.pcwszMemberFilePath=path.c_str();member.hMemberFile=file;member.pbCalculatedFileHash=hash;member.cbCalculatedFileHash=bytes;member.hCatAdmin=admin;
    WINTRUST_DATA data{};data.cbStruct=sizeof(data);data.dwUIChoice=WTD_UI_NONE;data.dwUnionChoice=WTD_CHOICE_CATALOG;data.pCatalog=&member;data.dwStateAction=WTD_STATEACTION_VERIFY;data.dwProvFlags=WTD_CACHE_ONLY_URL_RETRIEVAL|WTD_REVOCATION_CHECK_NONE;
    GUID action=WINTRUST_ACTION_GENERIC_VERIFY_V2;LONG result=WinVerifyTrust(nullptr,&action,&data);data.dwStateAction=WTD_STATEACTION_CLOSE;WinVerifyTrust(nullptr,&action,&data);return result==ERROR_SUCCESS;
}
bool Field(HINF inf,const wchar_t* section,const wchar_t* key,DWORD index,const wchar_t* expected){INFCONTEXT line{};wchar_t value[512]{};return SetupFindFirstLineW(inf,section,key,&line)&&SetupGetStringFieldW(&line,index,value,512,nullptr)&&!_wcsicmp(value,expected);}
bool OwnPackage(const fs::path& path){GUID cls{};wchar_t name[128]{};if(!SetupDiGetINFClassW(path.c_str(),&cls,name,128,nullptr)||!IsEqualGUID(cls,GUID_DEVCLASS_NETSERVICE))return false;HINF inf=SetupOpenInfFileW(path.c_str(),nullptr,INF_STYLE_WIN4,nullptr);if(inf==INVALID_HANDLE_VALUE)return false;struct Inf{HINF h;~Inf(){SetupCloseInfFile(h);}} close{inf};
 return Field(inf,L"Version",L"Class",1,L"NetService")&&Field(inf,L"Version",L"Provider",1,L"Titanfyre")&&Field(inf,L"Version",L"CatalogFile",1,L"TitanPacketCapture.cat")&&SetupGetLineCountW(inf,L"Models.NTamd64.10.0...19041")==1&&SetupGetLineCountW(inf,L"Models.NTamd64.10.0...25319")==1&&Field(inf,L"Models.NTamd64.10.0...19041",nullptr,1,L"InstallNC")&&Field(inf,L"Models.NTamd64.10.0...19041",nullptr,2,L"titan_packetcapture")&&Field(inf,L"Models.NTamd64.10.0...25319",nullptr,1,L"Install")&&Field(inf,L"Models.NTamd64.10.0...25319",nullptr,2,L"titan_packetcapture")&&Field(inf,L"Install.Services",L"AddService",1,L"TitanPacketCapture")&&Field(inf,L"InstallNC.Services",L"AddService",1,L"TitanPacketCapture")&&Field(inf,L"Install",L"NetCfgInstanceId",1,L"{F7F23E19-CAB5-4D94-9A2E-58CF86532513}")&&Field(inf,L"Ndi",nullptr,1,L"HKR")&&Field(inf,L"Ndi",nullptr,2,L"Ndi")&&Field(inf,L"Ndi",nullptr,3,L"Service")&&Field(inf,L"Ndi",nullptr,5,L"TitanPacketCapture");
}
struct Temporary {
    fs::path directory;
    bool owned=false;
    ~Temporary(){if(!owned)return;for(auto name:{L"TitanPacketCapture.inf",L"TitanPacketCapture.cat",L"TitanPacketCapture.sys"})DeleteFileW((directory/name).c_str());RemoveDirectoryW(directory.c_str());}
    void Create(){
        wchar_t windows[MAX_PATH]{};DWORD length=GetWindowsDirectoryW(windows,MAX_PATH);Require(length>0&&length<MAX_PATH);GUID guid{};Require(SUCCEEDED(CoCreateGuid(&guid)));wchar_t id[40]{};StringFromGUID2(guid,id,40);
        // Use a system-owned parent, not the initiating user's writable Temp tree.
        directory=fs::path(windows)/L"Temp"/(std::wstring(L"TitanPacketCapture-")+id);
        PSECURITY_DESCRIPTOR descriptor=nullptr;Require(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;;FA;;;SY)(A;;FA;;;BA)",SDDL_REVISION_1,&descriptor,nullptr)!=FALSE);
        SECURITY_ATTRIBUTES security{sizeof(security),descriptor,FALSE};bool made=CreateDirectoryW(directory.c_str(),&security)!=FALSE;DWORD error=GetLastError();LocalFree(descriptor);if(!made){SetLastError(error);Require(false);}
        owned=true;
        const wchar_t* names[]{L"TitanPacketCapture.inf",L"TitanPacketCapture.cat",L"TitanPacketCapture.sys"};
        for(int i=0;i<3;++i){
            HRSRC resource=FindResourceW(nullptr,MAKEINTRESOURCEW(101+i),RT_RCDATA);Require(resource!=nullptr);HGLOBAL loaded=LoadResource(nullptr,resource);Require(loaded!=nullptr);DWORD size=SizeofResource(nullptr,resource);void* bytes=LockResource(loaded);Require(bytes&&size&&size<16*1024*1024);
            HANDLE f=CreateFileW((directory/names[i]).c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);Require(f!=INVALID_HANDLE_VALUE);DWORD written=0;bool ok=WriteFile(f,bytes,size,&written,nullptr)&&written==size;error=GetLastError();CloseHandle(f);if(!ok){SetLastError(error);Require(false);}
        }
    }
};

struct NetConfig {
 INetCfg* cfg=nullptr;INetCfgLock* lock=nullptr;bool com=false,held=false,initialized=false;
 ~NetConfig(){if(initialized)cfg->Uninitialize();if(held)lock->ReleaseWriteLock();if(lock)lock->Release();if(cfg)cfg->Release();if(com)CoUninitialize();}
 HRESULT Open(bool write){auto hr=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);if(FAILED(hr))return hr;com=true;hr=CoCreateInstance(CLSID_CNetCfg,nullptr,CLSCTX_INPROC_SERVER,IID_INetCfg,reinterpret_cast<void**>(&cfg));if(FAILED(hr))return hr;if(write){hr=cfg->QueryInterface(IID_INetCfgLock,reinterpret_cast<void**>(&lock));if(FAILED(hr))return hr;LPWSTR other=nullptr;hr=lock->AcquireWriteLock(10000,L"Titan Packet Capture Setup",&other);if(other)CoTaskMemFree(other);if(hr!=S_OK)return FAILED(hr)?hr:HRESULT_FROM_WIN32(ERROR_BUSY);held=true;}hr=cfg->Initialize(nullptr);initialized=SUCCEEDED(hr);return hr;}
};
DWORD Code(HRESULT hr){return HRESULT_FACILITY(hr)==FACILITY_WIN32?HRESULT_CODE(hr):static_cast<DWORD>(hr);}
Setup::Result Configure(bool install){
 Temporary temp;
 if(install){
  if(!FindResourceW(nullptr,MAKEINTRESOURCEW(101),RT_RCDATA))return {ERROR_FILE_NOT_FOUND,false,L"Source preview: no driver package is embedded. Use a complete Microsoft-signed release when available."};
  temp.Create();if(!OwnPackage(temp.directory/L"TitanPacketCapture.inf"))return {ERROR_INVALID_DATA,false,L"Exact Packet Capture INF identity refused."};
  if(!MicrosoftCatalog(temp.directory/L"TitanPacketCapture.cat")||!CatalogMember(temp.directory,L"TitanPacketCapture.inf")||!CatalogMember(temp.directory,L"TitanPacketCapture.sys"))return {ERROR_INVALID_IMAGE_HASH,false,L"A trusted Microsoft-root hardware-publisher catalog with matching INF/SYS membership is required. Unsigned and test-signed packages are refused."};
  wchar_t published[32768]{};if(!SetupCopyOEMInfW((temp.directory/L"TitanPacketCapture.inf").c_str(),temp.directory.c_str(),SPOST_PATH,0,published,32768,nullptr,nullptr))return {GetLastError(),false,L"Driver Store staging failed. "+Message(GetLastError())};
 }
 NetConfig network;auto hr=network.Open(true);if(FAILED(hr))return {Code(hr),false,L"Windows network configuration lock failed. "+Message(Code(hr))};
 INetCfgComponent* component=nullptr;hr=network.cfg->FindComponent(L"titan_packetcapture",&component);if(FAILED(hr))return {Code(hr),false,L"Network component inventory failed."};
 struct Component{INetCfgComponent*& p;~Component(){if(p)p->Release();}} release{component};
 if(install&&component)return {0,false,L"Packet capture filter is already installed. Close capture applications and use Uninstall before installing a different driver version."};
 INetCfgClassSetup* setup=nullptr;hr=network.cfg->QueryNetCfgClass(&GUID_DEVCLASS_NETSERVICE,IID_INetCfgClassSetup,reinterpret_cast<void**>(&setup));if(FAILED(hr))return {Code(hr),false,L"Cannot open the NetService installation class."};
 struct Class{INetCfgClassSetup* p;~Class(){p->Release();}} releaseClass{setup};OBO_TOKEN token{};token.Type=OBO_USER;
 if(install)hr=setup->Install(L"titan_packetcapture",&token,0,0,nullptr,nullptr,&component);
 else if(component)hr=setup->DeInstall(component,&token,nullptr);else hr=S_OK;
 if(FAILED(hr)){network.cfg->Cancel();return {Code(hr),false,L"Network component update failed. "+Message(Code(hr))};}
 hr=network.cfg->Apply();if(FAILED(hr)){network.cfg->Cancel();return {Code(hr),false,L"Windows did not apply the network binding update. "+Message(Code(hr))};}
 bool restart=hr==NETCFG_S_REBOOT;
 if(!install){wchar_t windows[MAX_PATH]{};Require(GetWindowsDirectoryW(windows,MAX_PATH)!=0);fs::path directory=fs::path(windows)/L"INF";WIN32_FIND_DATAW data{};HANDLE search=FindFirstFileW((directory/L"oem*.inf").c_str(),&data);if(search!=INVALID_HANDLE_VALUE){struct Find{HANDLE h;~Find(){FindClose(h);}} close{search};unsigned count=0;do{if(++count>16384)return {ERROR_BUFFER_OVERFLOW,restart,L"Driver Store scan exceeded its bound."};if(OwnPackage(directory/data.cFileName)&&!SetupUninstallOEMInfW(data.cFileName,0,nullptr)){auto e=GetLastError();return {e,true,L"Filter unbound; Windows retained an in-use package. Close capture programs, restart and run Uninstall again. "+Message(e)};}}while(FindNextFileW(search,&data));if(GetLastError()!=ERROR_NO_MORE_FILES)return {GetLastError(),restart,L"Driver Store inventory failed."};}}
 return {0,restart,install?L"NDIS filter installed. It passes traffic unchanged until an authorized application selects one adapter; there is no automatic capture.":L"Packet Capture component and its unused packages removed. Unrelated network components were retained."};
}
}
unsigned Setup::Installed(DWORD* error){if(error)*error=0;SC_HANDLE scm=OpenSCManagerW(nullptr,nullptr,SC_MANAGER_CONNECT);if(!scm){if(error)*error=GetLastError();return 0;}SC_HANDLE service=OpenServiceW(scm,L"TitanPacketCapture",SERVICE_QUERY_STATUS);if(service)CloseServiceHandle(service);CloseServiceHandle(scm);return service?1u:0u;}
bool Setup::PackageMatches(const std::wstring& path) noexcept{try{return OwnPackage(path);}catch(...){return false;}}
bool Setup::CatalogTrusted(const std::wstring& path) noexcept{try{return MicrosoftCatalog(path);}catch(...){return false;}}
Setup::Result Setup::Execute(bool install) noexcept {HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Global\\TitanPacketCaptureSetup");if(!mutex)return {GetLastError(),false,L"Cannot open setup lock."};auto wait=WaitForSingleObject(mutex,0);if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);return {ERROR_BUSY,false,L"Another Packet Capture installation is running."};}Result result;try{result=Configure(install);}catch(const std::system_error& e){result={static_cast<DWORD>(e.code().value()),false,Message(static_cast<DWORD>(e.code().value()))};}catch(...){result={ERROR_GEN_FAILURE,false,L"Setup could not complete. Read the result, refresh and retry."};}ReleaseMutex(mutex);CloseHandle(mutex);return result;}
bool Setup::Restart() noexcept {HANDLE token{};if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&token))return false;TOKEN_PRIVILEGES p{};p.PrivilegeCount=1;bool ok=LookupPrivilegeValueW(nullptr,SE_SHUTDOWN_NAME,&p.Privileges[0].Luid)!=FALSE;p.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;if(ok){SetLastError(0);ok=AdjustTokenPrivileges(token,FALSE,&p,0,nullptr,nullptr)&&GetLastError()==ERROR_SUCCESS;}CloseHandle(token);return ok&&ExitWindowsEx(EWX_REBOOT,SHTDN_REASON_MAJOR_APPLICATION|SHTDN_REASON_MINOR_INSTALLATION);}
