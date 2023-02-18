#include "shooby_db.h"
#include "shooby_metamap.h"
#include <iostream>

struct Bl
{
    int hi{97};
    char f{'b'};
    int a{2566};
    char bye{'G'};

    bool operator==(const Bl &other) const
    {
        return hi == other.hi && f == other.f && a == other.a && bye == other.bye;
    }
};

std::ostream &operator<<(std::ostream &os, const Bl &bl)
{
    os << "Bl{hi: " << bl.hi << ", f: " << bl.f << ", a: " << bl.a << ", bye: " << bl.bye << "}";
    return os;
}

Bl bl;

DEFINE_SHOOBY_META_MAP(Dooby)
SHOOBY_ENUMS(
    NUMBER_U16,
    NUMBER_16,
    NUMBER_U32,
    BOOL,
    STRING,
    BLOB)

SHOOBY_META_MAP_START
META_MAP_INTEGRAL(NUMBER_U16, uint16_t, 16)
META_MAP_INTEGRAL(NUMBER_16, int16_t, -16)
META_MAP_INTEGRAL(NUMBER_U32, uint32_t, 250)
META_MAP_INTEGRAL(BOOL, bool, true)
META_MAP_STRING(STRING, "WHATEVER", 33)
META_MAP_BLOB(BLOB, Bl, bl)
SHOOBY_META_MAP_END

using namespace std;
using enum Dooby::enum_type;
using DB = Shooby::DB<Dooby>;

template <class T>
bool is_equal(T a, T b)
{
    if constexpr (std::is_same_v<T, const char *>)
    {
        return (strcmp(a, b) == 0);
    }
    else
    {
        return (a == b);
    }
}

template <class T>
void test_equals(T a, T b)
{
    if (!is_equal(a, b))
    {
        cout << "ERROR: " << a << " != " << b << endl;
        throw "ERROR";
    }
}

template <class T>
void test_unequals(T a, T b)
{
    if (is_equal(a, b))
    {
        cout << "ERROR: " << a << " != " << b << endl;
        throw "ERROR";
    }
}

void test_string()
{
    auto unsafe_str = DB::Get<const char *>(STRING);
    auto safe_fixed_str = DB::GetString<STRING>();
    test_equals(unsafe_str, safe_fixed_str.c_str());

    DB::Set(STRING, "HELLO");
    test_unequals(unsafe_str, safe_fixed_str.c_str());

    safe_fixed_str = DB::GetString<STRING>();
    test_equals(unsafe_str, safe_fixed_str.c_str());

    cout << "TEST PASSED" << endl;
}

void test_blob()
{
    auto blob1 = DB::Get<Bl>(BLOB);
    auto unsafe_blob = DB::Get<const Bl *>(BLOB);

    test_equals(blob1, *unsafe_blob);

    Bl blob2;
    blob2.hi = 100;
    blob2.f = 'c';
    blob2.a = 2000;
    blob2.bye = 'H';

    test_unequals(blob1, blob2);
    DB::Set(BLOB, blob2);

    test_unequals(blob1, blob2);
    test_unequals(blob1, *unsafe_blob);
    test_equals(blob2, *unsafe_blob);

    blob1 = DB::Get<Bl>(BLOB);
    test_equals(blob1, blob2);
    test_equals(blob2, *unsafe_blob);

    cout << "TEST PASSED" << endl;
}

void test_number()
{
    uint16_t test_num_100 = 100;
    uint16_t test_num_16 = 16;

    test_equals(DB::Get<uint16_t>(NUMBER_U16), test_num_16);

    DB::Set(NUMBER_U16, test_num_100);
    test_equals(DB::Get<uint16_t>(NUMBER_U16), test_num_100);

    // DB::Set(NUMBER, int16_t(100)); - FAILURE!

    cout << "TEST PASSED" << endl;
}

void test_bool()
{

    test_equals(DB::Get<bool>(BOOL), true);

    DB::Set(BOOL, false);
    test_equals(DB::Get<bool>(BOOL), false);

    // DB::Set(BOOL, int16_t(100)); - FAILURE!

    cout << "TEST PASSED" << endl;
}

void observer_callback(Dooby::enum_type type, void *data)
{
    switch (type)
    {
    case NUMBER_16:
        cout << "NUMBER ";
        break;
    case BOOL:
        cout << "BOOL ";
        break;
    case STRING:
        cout << "STRING ";
        break;
    case BLOB:
        cout << "BLOB ";
        break;
    }
}

class Visitor
{
public:
    void operator()(Dooby::enum_type type, Shooby::value_variant_t &value)
    {
        std::visit(
            Shooby::Overload{
                [&](uint16_t value)
                { cout << Dooby::get_name(type) << " is a uint16 with " << value << endl; },
                [&](int16_t value)
                { cout << Dooby::get_name(type) << " is a int16 with " << value << endl; },
                [&](uint32_t value)
                { cout << Dooby::get_name(type) << " is a uint32_t with " << value << endl; },
                [&](bool value)
                { cout << Dooby::get_name(type) << std::boolalpha << " is a bool with " << value << endl; },
                [&](const char *value)
                { cout << Dooby::get_name(type) << " is a string with " << value << endl; },
                [&](const void *value)
                { cout << Dooby::get_name(type) << " is a blob with " << *(Bl *)value << endl; },
                [&](auto value)
                { cout << "unknown" << endl; },
            },
            value);
    }
};

void visit_test()
{
    Visitor visitor;
    DB::VisitEach(visitor);
}

int main(void)
{

    Shooby::DB<Dooby>::Init();
    DB::SetObserver(observer_callback, nullptr);

    visit_test();

    try
    {
        test_string();
        test_blob();
        test_number();
        test_bool();
    }
    catch (const char *e)
    {
        cout << "TEST FAILED" << endl;
        return 1;
    }

    cout << "TEST PASSED" << endl;

    return 0;
}