#include "shooby_db.h"
#include "shooby_metamap.h"
#include <iostream>

struct Bl
{
    int hi{97};
    char f{'b'};
    int a{2566};
    char bye{'G'};
};

Bl bl;

DEFINE_SHOOBY_META_MAP(Shooby)
SHOOBY_ENUMS(
    NUMBER,
    BOOL,
    STRING,
    BLOB)

SHOOBY_META_MAP_START
META_MAP_INTEGRAL(NUMBER, uint16_t, 250)
META_MAP_INTEGRAL(BOOL, uint16_t, 250)
META_MAP_STRING(STRING, "WHATEVER", 32)
META_MAP_BLOB(BLOB, Bl, bl)
SHOOBY_META_MAP_END

int main(void)
{
    using namespace std;
    using enum Shooby::enum_type;

    ShoobyDB<Shooby>::Reset();
    auto num = ShoobyDB<Shooby>::Get<const char *>(Shooby::STRING);
    cout << "num: " << num << endl;

    ShoobyDB<Shooby>::Set(Shooby::STRING, "WHOOOAA");

    num = ShoobyDB<Shooby>::Get<const char *>(Shooby::STRING);
    cout << "num: " << num << endl;

    return 0;
}