#include "../SDK/PacketCapture.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
int main(){
 std::vector<unsigned char> bytes;unsigned checks=0;
 for(const char* invalid:{"", "0 A", "GG", "00:1", "-", "00 11", "00,11", "00112233445566778899aabbccdd0", "00112233445566778899aabbccdd 0 A"}){if(ParseEthernetFrame(invalid,bytes)||!bytes.empty())return 1;++checks;}
 std::mt19937 random(0x731E);for(unsigned n=0;n<20000;++n){std::vector<unsigned char> expected(14+random()%1505);std::ostringstream text;for(auto& value:expected)value=static_cast<unsigned char>(random());
 text<<std::hex<<std::setfill('0');for(auto value:expected)text<<std::setw(2)<<unsigned(value)<<' ';
 if(!ParseEthernetFrame(text.str(),bytes)||bytes!=expected)return 1;++checks;
 std::string invalid=text.str();invalid[random()%invalid.size()]='?';if(ParseEthernetFrame(invalid,bytes)||!bytes.empty())return 1;++checks;
 }
 std::cout<<checks<<" deterministic parser boundary/round-trip/mutation checks passed; no driver opened.\n";
}
