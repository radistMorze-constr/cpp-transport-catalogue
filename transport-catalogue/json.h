#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stddef.h>
#include <stdexcept>
#include <optional>

namespace json {

    class Node;
    // —охраните объ¤влени¤ Dict и Array без изменени¤
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    using JsonType = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    // Ёта ошибка должна выбрасыватьс¤ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : private std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> {
    public:
        /* –еализуйте Node, использу¤ std::variant */
        using variant::variant;
        /*explicit Node();
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(std::string value);
        Node(bool value);
        Node(double value);
        Node(std::nullptr_t value);*/

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const JsonType& GetValue() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
    };

    inline bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root);
        explicit Document();

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json