/*
    calculator08buggy.cpp

    Helpful comments removed.

    We have inserted 3 bugs that the compiler will catch and 3 that it won't.

    Drill 1, 2, 3, 4 and 5

 */

#include "../includes/ppp.h"

struct Token {
    char kind;
    double value;
    std::string name;
    Token(char ch, double val = 0.0) :kind(ch), value(val) {}
    Token(char ch, std::string s) :kind(ch), value(0.0), name(s) {}
};

//------------------------------------------------------------------------------

// place to hold variable name value
struct Variable {
    std::string name;
    double value{};
};

// overload equality operator to simplify get, set and is_declared functions 
bool operator==(const Variable &v, const std::string s) { return v.name == s; }

// container for all the variables
std::vector<Variable> names;

// return the value of the Variable with the input name.
double get_value(const std::string s)
{
    auto vt_itr = std::find(names.cbegin(), names.cend(), s);
    if (vt_itr == names.cend()) {
        throw std::runtime_error("get: undefined name " + s);
    }
    return vt_itr->value;
}

// set the Variable of the named to a double value.
void set_value(const std::string s, const double d)
{
    auto vt_itr = std::find(names.begin(), names.end(), s);
    if (vt_itr == names.cend()) {
        throw std::runtime_error("set: undefined name " + s);
    }
    vt_itr->value = d;
}

// is a name already declared
bool is_declared(const std::string s)
{
    return names.cend() != std::find(names.cbegin(), names.cend(), s);
}

// add name value to a vector of Variables
double define_name(const std::string s, const double d)
{
    if (is_declared(s)) throw std::runtime_error(s + " declared twice");
    names.push_back(Variable{ s, d });
    return d;
}

//------------------------------------------------------------------------------

// place to hold valid Tokens from cin
class Token_stream {
public:
    Token_stream() :full(false), buffer('\0') { }

    // get a Token to place in the stream
    Token get();

    // put a Token back
    void putback(const Token t);

    // discard characters up to and including the given input kind token
    void ignore(const char c);

private:
    bool full;      // is there a Token in the buffer?
    Token buffer;   // here is where we keep a Token put back using putback()
};

//------------------------------------------------------------------------------

constexpr char number = '8';    // t.kind == number means that t is a number Token.
constexpr char quit = 'q';      // t.kind == quit means that t is a quit Token.
constexpr char print = ';';     // t.kind == print means that t is a print Token.

constexpr char name = 'a';      // name token
constexpr char let = 'L';       // declaration token
constexpr char* declkey = "let"; // declaration keyword

//------------------------------------------------------------------------------

Token Token_stream::get()
{
    Token t{ '\0' };
    if (full) {
        full = false;
        t = buffer;
    }
    else {
        char ch;
        std::cin >> ch;
        switch (ch) {
        case print:
        case quit:
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
            //case '%':
            t.kind = ch;
            break;
        case '=':
            if (this->buffer.kind != let) {
                const std::string s = (this->buffer.kind == name ? this->buffer.name : std::to_string(this->buffer.value));
                throw std::runtime_error(s + " can not be re-assigned");
            }
            t.kind = ch;
            break;
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            std::cin.putback(ch);
            double val;
            std::cin >> val;
            t.kind = number;
            t.value = val;
            break;
        }
        default:
            if (isalpha(ch)) {
                std::string s;
                s += ch;
                while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch))) s += ch;
                std::cin.putback(ch);
                if (s == declkey) {
                    t.kind = let;
                }
                //if (s == "quit") return Token(name);
                else {
                    t.kind = name;
                    t.name = s;
                }
            }
            else {
                throw std::runtime_error("Bad token");
            }
        }
    }
    return t;
}

void Token_stream::putback(const Token t)
{
    if (full) {
        throw std::runtime_error("putback() into a full buffer");
    }
    buffer = t;
    full = true;
}

void Token_stream::ignore(const char c)
{
    if (full && c == buffer.kind) {
        full = false;
    }
    else {
        full = false;
        ppp::clear_cin_buffer(c);
    }
}

Token_stream ts;

//------------------------------------------------------------------------------

double expression();

//------------------------------------------------------------------------------

double primary()
{
    Token t = ts.get();
    double d{};
    switch (t.kind) {
    case '(': {
        d = expression();
        t = ts.get();
        if (t.kind != ')') {
            throw std::runtime_error("')' expected");
        }
        return d;
    }
    case '-':
        return -1 * primary();
    case '+':
        return primary();
    case number:
        return t.value;
    case name:
        return get_value(t.name);
    default:
        throw std::runtime_error("primary expected");
    }
}

//------------------------------------------------------------------------------

double term()
{
    double left = primary();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '*':
            left *= primary();
            break;
        case '/':
        {	double d = primary();
        if (d == 0) throw std::runtime_error("divide by zero");
        left /= d;
        break;
        }
        //case '%':
            // not implemented yet
        default:
            ts.putback(t);
            return left;
        }
    }
}

//------------------------------------------------------------------------------

double expression()
{
    double left = term();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

//------------------------------------------------------------------------------

double declaration()
{
    Token t = ts.get();
    if (t.kind != name) {
        throw std::runtime_error("name expected in declaration");
    }
    if (is_declared(t.name)) {
        throw std::runtime_error(t.name + " declared twice");
    }
    Token t2 = ts.get();
    if (t2.kind != '=') {
        throw std::runtime_error("= missing in declaration of " + t.name);
    }
    double d = expression();
    define_name(t.name, d);
    return d;
}

//------------------------------------------------------------------------------

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let:
        return declaration();
    default:
        ts.putback(t);
        return expression();
    }
}

//------------------------------------------------------------------------------

void clean_up_mess()
{
    ts.ignore(print);
}

//------------------------------------------------------------------------------

void calculate()
{
    constexpr char* prompt = "> ";  // indicate a prompt
    constexpr char* result = "= ";  // indicate a result
    while (true) try {
        std::cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t = ts.get();
        if (t.kind == quit) return;
        ts.putback(t);
        std::cout << result << statement() << std::endl;
    }
    catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        clean_up_mess();
    }
}

//------------------------------------------------------------------------------

int main()
try {
    SET_IOSYNC;
    calculate();
    return 0;
}
catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    ppp::keep_window_open();
    return 1;
}
catch (...) {
    std::cerr << "exception\n";
    ppp::keep_window_open();
    return 2;
}