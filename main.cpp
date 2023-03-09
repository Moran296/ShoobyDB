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

#define Dooby(CONFIG_NUM, CONFIG_STR, CONFIG_BLOB) \
    CONFIG_NUM(SOME_NUMBER_U16, uint16_t, 16)      \
    CONFIG_NUM(SOME_NUMBER_16, int16_t, -16)       \
    CONFIG_NUM(SOME_BOOL, bool, true)              \
    CONFIG_STR(SOME_STRING, "WHATEVER", 34)        \
    CONFIG_NUM(SOME_NUMBER_32, uint32_t, 32)       \
    CONFIG_BLOB(SOME_BLOB, Bl, Bl{})

DEFINE_SHOOBY_META_MAP(Dooby)

struct g
{
    static const inline auto f = []
    { return 1; };
};

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
    auto unsafe_str = DB::Get<const char *>(SOME_STRING);
    auto safe_fixed_str = DB::GetString<SOME_STRING>();
    test_equals(unsafe_str, safe_fixed_str.c_str());

    DB::Set(SOME_STRING, "HELLO");
    test_unequals(unsafe_str, safe_fixed_str.c_str());

    safe_fixed_str = DB::GetString<SOME_STRING>();
    test_equals(unsafe_str, safe_fixed_str.c_str());

    cout << "TEST PASSED" << endl;
}

void test_blob()
{
    auto blob1 = DB::Get<Bl>(SOME_BLOB);
    auto unsafe_blob = DB::Get<const Bl *>(SOME_BLOB);

    test_equals(blob1, *unsafe_blob);

    Bl blob2;
    blob2.hi = 100;
    blob2.f = 'c';
    blob2.a = 2000;
    blob2.bye = 'H';

    test_unequals(blob1, blob2);
    DB::Set(SOME_BLOB, blob2);

    test_unequals(blob1, blob2);
    test_unequals(blob1, *unsafe_blob);
    test_equals(blob2, *unsafe_blob);

    blob1 = DB::Get<Bl>(SOME_BLOB);
    test_equals(blob1, blob2);
    test_equals(blob2, *unsafe_blob);

    cout << "TEST PASSED" << endl;
}

void test_number()
{
    uint16_t test_num_100 = 100;
    uint16_t test_num_16 = 16;

    test_equals(DB::Get<uint16_t>(SOME_NUMBER_U16), test_num_16);

    DB::Set(SOME_NUMBER_U16, test_num_100);
    test_equals(DB::Get<uint16_t>(SOME_NUMBER_U16), test_num_100);

    // DB::Set(NUMBER, int16_t(100)); - FAILURE!

    cout << "TEST PASSED" << endl;
}

void test_bool()
{

    test_equals(DB::Get<bool>(SOME_BOOL), true);

    DB::Set(SOME_BOOL, false);
    test_equals(DB::Get<bool>(SOME_BOOL), false);

    // DB::Set(BOOL, int16_t(100)); - FAILURE!

    cout << "TEST PASSED" << endl;
}

class Observer final : public Shooby::DB<Dooby>::IObserver
{
public:
    void OnChange(Dooby::enum_type type) override
    {
        switch (type)
        {
        case SOME_NUMBER_16:
            cout << "NUMBER ";
            break;
        case SOME_BOOL:
            cout << "BOOL ";
            break;
        case SOME_STRING:
            cout << "STRING ";
            break;
        case SOME_BLOB:
            cout << "BLOB ";
            break;
        }
    }
};

class Backend final : public Shooby::IBackend
{
public:
    void Save(const char *e_name, const void *data, size_t size) override
    {
        cout << "Backend Saved " << e_name << endl;
    }

    bool Load(const char *e_name, void *data, size_t size) override
    {
        cout << "Backend Loaded " << e_name << endl;
        return true;
    }
};

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

    Backend backend;
    Shooby::DB<Dooby>::Init(&backend);

    Observer observer;
    DB::SetObserver(&observer);

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