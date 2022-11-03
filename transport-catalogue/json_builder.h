#pragma once

#include "json.h"

#include <string>
#include <deque>
#include <stdexcept>

namespace json {
using namespace std::literals;

enum class Command {
	key,
	value,
	start_dict,
	end_dict,
	start_array,
	end_array
};

class Builder {
private:
	class BaseContext;

	class DictKeyContext;

	class DictItemContext;

	class DictValueContext;

	class ArrayItemContext;

	class ArrayValueContext;

public:
	Builder() = default;
	DictItemContext StartDict();
	DictKeyContext Key(std::string&& key);
	BaseContext Value(Node::Value&& value);
	ArrayItemContext StartArray();
	BaseContext EndDict();
	BaseContext EndArray();
	Node Build();

private:
	bool CommandIsCorrect(Command value);

	std::deque<Command> command_list_;
	std::deque<Node*> unfinished_element_ = { &node_ };
	Node node_;
};

class Builder::BaseContext {
public:
	BaseContext(Builder& builder);

	DictItemContext StartDict();

	DictKeyContext Key(std::string&& key);

	BaseContext Value(Node::Value&& value);

	ArrayItemContext StartArray();

	BaseContext EndDict();

	BaseContext EndArray();

	Node Build();

	Builder& builder_;
};

class Builder::DictKeyContext final : public BaseContext {
public:
	DictKeyContext(Builder& builder)
		: BaseContext(builder)
	{}
	DictKeyContext Key(std::string&& key) = delete;
	DictValueContext Value(Node::Value&& value);
	BaseContext EndDict() = delete;
	BaseContext EndArray() = delete;
	Node Build() = delete;
};

class Builder::DictItemContext final : public BaseContext {
public:
	DictItemContext(Builder& builder)
		: BaseContext(builder)
	{}
	DictItemContext StartDict() = delete;
	DictItemContext Value(Node::Value&& value) = delete;
	ArrayItemContext StartArray() = delete;
	BaseContext EndArray() = delete;
	Node Build() = delete;
};

class Builder::DictValueContext final : public BaseContext {
public:
	DictValueContext(Builder& builder)
		: BaseContext(builder)
	{}
	DictValueContext StartDict() = delete;
	DictValueContext Value(Node::Value&& value) = delete;
	ArrayItemContext StartArray() = delete;
	BaseContext EndArray() = delete;
	Node Build() = delete;
};

class Builder::ArrayItemContext final : public BaseContext {
public:
	ArrayItemContext(Builder& builder)
		: BaseContext(builder)
	{}
	DictKeyContext Key(std::string&& key) = delete;
	ArrayValueContext Value(Node::Value&& value);
	BaseContext EndDict() = delete;
	Node Build() = delete;
};

class Builder::ArrayValueContext final : public BaseContext {
public:
	ArrayValueContext(Builder& builder)
		: BaseContext(builder)
	{}
	DictKeyContext Key(std::string&& key) = delete;
	ArrayValueContext Value(Node::Value&& value);
	BaseContext EndDict() = delete;
	Node Build() = delete;
};

} //namespace json