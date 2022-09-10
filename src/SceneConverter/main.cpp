#include <iostream>

#include <string>



int main(int argc, char* argv[])
{
    std::string_view v = { "hello" };
    char l = v.data()[v.length()];
    std::string str(v.data());
}