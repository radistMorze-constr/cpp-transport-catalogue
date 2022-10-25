#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(move(s));
}

Node LoadTrue(istream& input) {
    using namespace std::literals;
    std::string str;
    for (char c; input >> c;) {
        str.push_back(c);
        if (str == "true"s) {
            return { true };
        }
        if (str.size() == 4) {
            break;
        }
    }
    throw ParsingError(""s);
}

Node LoadFalse(istream& input) {
    using namespace std::literals;
    std::string str;
    for (char c; input >> c;) {
        str.push_back(c);
        if (str == "false"s) {
            return { false };
        }
        if (str.size() == 5) {
            break;
        }
    }
    throw ParsingError(""s);
}

Node LoadNull(istream& input) {
    using namespace std::literals;
    std::string str;
    for (char c; input >> c;) {
        str.push_back(c);
        if (str == "null"s) {
            return {nullptr};
        }
        if (str.size() == 4) {
            break;
        }
    }
    throw ParsingError(""s);
}

Node LoadArray(istream& input) {
    Array result;
    for (char c; input >> c;) {
        if (c == ']') {
            return Node(move(result));
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    throw ParsingError(""s);
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c;) {
        if (c == '}') {
            return Node(move(result));
        }
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({ move(key), LoadNode(input) });
    }

    throw ParsingError(""s);
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 't') {
        input.putback(c);
        return LoadTrue(input);
    }
    else if (c == 'f') {
        input.putback(c);
        return LoadFalse(input);
    }
    else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else {
        input.putback(c);
        Number number = LoadNumber(input);
        if (std::holds_alternative<int>(number)) {
            return std::get<int>(number);
        }
        else {
            return std::get<double>(number);
        }
    }
}

}  // namespace

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

void PrintNode(const Node& node, const PrintContext& ptcx);

void PrintValue(const int value, const PrintContext& ptcx) {
    ptcx.out << value;
}

void PrintValue(const double value, const PrintContext& ptcx) {
    ptcx.out << value;
}

void PrintValue(const std::string& value, const PrintContext& ptcx) {
    ptcx.out << "\""s;
    for (const char i : value) {
        switch (i) {
        case '\n':
            ptcx.out << "\\n"s;
            break;
        case '\t':
            ptcx.out << "\t"s;
            break;
        case '\r':
            ptcx.out << "\\r"s;
            break;
        case '\"':
            ptcx.out << "\\\""s;
            break;
        case '\\':
            ptcx.out << "\\\\"s;
            break;
        default:
            ptcx.out << i;
            break;
        }
    }
    ptcx.out  << "\""s;
}

void PrintValue(const bool value, const PrintContext& ptcx) {
    ptcx.out << std::boolalpha << value << std::noboolalpha;
}

void PrintValue(const std::nullptr_t, const PrintContext& ptcx) {
    ptcx.out << "null"s;
}

void PrintValue(const Array& value, const PrintContext& ptcx) {
    ptcx.out << "["s;
    bool is_first = true;
    for (const auto& node : value) {
        if (!is_first) {
            ptcx.out << ","s;
        }
        is_first = false;
        PrintNode(node, ptcx);
    }
    
    ptcx.out << "]"s;
}

void PrintValue(const Dict& value, const PrintContext& ptcx) {
    ptcx.out << "{"s;
    bool is_first = true;
    for (const auto& [key, node] : value) {
        if (!is_first) {
            ptcx.out << ","s;
        }
        is_first = false;
        ptcx.out << "\""s << key << "\""s;
        ptcx.out << ":"s;
        PrintNode(node, ptcx);
    }
    ptcx.out << "}"s;
}

Node::Node() = default;

Node::Node(Array array) 
    :value_(std::move(array))
{
}
Node::Node(Dict map)
    :value_(std::move(map))
{
}
Node::Node(int value)
    :value_(std::move(value))
{
}
Node::Node(std::string value)
    :value_(std::move(value))
{
}

Node::Node(bool value)
    :value_(std::move(value))
{
}

Node::Node(double value)
    :value_(std::move(value))
{
}

Node::Node(std::nullptr_t value)
    :value_(std::move(value))
{
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
bool Node::IsDouble() const {
    return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_);
}
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}
bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}
bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}
bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

const JsonType& Node::GetValue() const {
    return value_;
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return std::get<Array>(value_);
    }
    else {
        throw std::logic_error(""s);
    }
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Dict>(value_);
    }
    else {
        throw std::logic_error(""s);
    }
}

bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(value_);
    }
    else {
        throw std::logic_error(""s);
    }
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(value_);
    }
    if (IsInt()) {
        return std::get<int>(value_);
    }
    else {
        throw std::logic_error(""s);
    }
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(value_);
    }
    else {
        throw std::logic_error(""s);
    }
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<std::string>(value_);
    }
    else {
        throw std::logic_error(""s);
    }
}

Document::Document(Node root)
: root_(move(root)) {
}

Document::Document() = default;

const Node& Document::GetRoot() const {
return root_;
}

Document Load(istream& input) {
return Document{ LoadNode(input) };
}

void PrintNode(const Node& node, const PrintContext& ptcx) {
    std::visit(
        [&ptcx](const auto& value) { PrintValue(value, ptcx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext ptcx = { output };
    const auto& node = doc.GetRoot();
    PrintNode(node, ptcx);
}

}  // namespace json