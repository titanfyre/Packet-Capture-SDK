#include "PacketCaptureSDKJsonSDK.h"
#include <string.h>
int main(void){uint64_t session=0;if(packet_capture_sdk_json_open(&session))return 1;char *reply=packet_capture_sdk_json_call(session,"{",1);if(!reply){packet_capture_sdk_json_close(session);return 2;}int failed=strstr(reply,"\"ok\":false")==0;packet_capture_sdk_json_free(reply);if(packet_capture_sdk_json_close(session))return 3;return failed;}
