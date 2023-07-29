#include "../include/platanium.hpp"

std::string FAESKey::to_hex_string(void)
{
    std::stringstream ss;
    ss << "0x";
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < KeySize; i++)
    {
        ss << std::setw(2) << static_cast<int>(Key[i]);
    }

    return ss.str();
}