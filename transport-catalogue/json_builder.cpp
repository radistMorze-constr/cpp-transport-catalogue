#include "json_builder.h"

namespace json {

Builder::BaseContext::BaseContext(Builder& builder)
	: builder_(builder)
{}

Builder::DictItemContext Builder::BaseContext::StartDict() {
	return builder_.StartDict();
}

Builder::DictKeyContext Builder::BaseContext::Key(std::string&& key) {
	return builder_.Key(std::move(key));
}

Builder::BaseContext Builder::BaseContext::Value(Node::Value&& value) {
	return builder_.Value(std::move(value));
}

Builder::ArrayItemContext Builder::BaseContext::StartArray() {
	return builder_.StartArray();
}

Builder::BaseContext Builder::BaseContext::EndDict() {
	return builder_.EndDict();
}

Builder::BaseContext Builder::BaseContext::EndArray() {
	return builder_.EndArray();
}

Node Builder::BaseContext::Build() {
	return builder_.Build();
}

Builder::DictValueContext Builder::DictKeyContext::Value(Node::Value&& value) {
	builder_.Value(std::move(value));
	return DictValueContext(builder_);
}

Builder::ArrayValueContext Builder::ArrayItemContext::Value(Node::Value&& value) {
	builder_.Value(std::move(value));
	return ArrayValueContext(builder_);
}

Builder::ArrayValueContext Builder::ArrayValueContext::Value(Node::Value&& value) {
	builder_.Value(std::move(value));
	return ArrayValueContext(builder_);
}

Builder::DictItemContext Builder::StartDict() {
	if (command_list_.empty()) {
		auto* it = unfinished_element_.back();
		*it = { Dict() };
	}
	else if (command_list_.back() == Command::KEY) {
		auto* it = unfinished_element_.back();
		*it = { Dict() };
		command_list_.pop_back();
	}
	else if (command_list_.back() == Command::START_ARRAY) {
		auto* it = unfinished_element_.back();
		auto& it_as_array = std::get<Array>(it->GetValue());
		it_as_array.push_back({ Dict() });
		unfinished_element_.push_back(&it_as_array.back());
	}
	command_list_.push_back(Command::START_DICT);
	return DictItemContext(*this);
}

Builder::DictKeyContext Builder::Key(std::string&& key) {
	command_list_.push_back(Command::KEY);
	auto* it = unfinished_element_.back();
	auto& it_as_dict = std::get<Dict>(it->GetValue());
	unfinished_element_.push_back(&(it_as_dict[std::move(key)]));
	return DictKeyContext(*this);
}

Builder::BaseContext Builder::Value(Node::Value&& value) {
	if (command_list_.empty()) {
		auto* it = unfinished_element_.back();
		it->GetValue() = std::move(value);
		command_list_.push_back(Command::VALUE);
		return BaseContext(*this);
	}
	else if (command_list_.back() == Command::KEY) {
		auto* it = unfinished_element_.back();
		it->GetValue() = std::move(value);
		unfinished_element_.pop_back();
		command_list_.pop_back();
		return DictValueContext(*this);
	}
	//Последняя команда Command::START_ARRAY
	else {
		auto* it = unfinished_element_.back();
		auto& it_as_array = std::get<Array>(it->GetValue());
		it_as_array.push_back({ });
		it_as_array.back().GetValue() = std::move(value);
		return ArrayValueContext(*this);
	}
}

Builder::ArrayItemContext Builder::StartArray() {
	if (command_list_.empty()) {
		auto* it = unfinished_element_.back();
		*it = { Array() };
	}
	else if (command_list_.back() == Command::KEY) {
		auto* it = unfinished_element_.back();
		*it = { Array() };
		command_list_.pop_back();
	}
	else if (command_list_.back() == Command::START_ARRAY) {
		auto* it = unfinished_element_.front();
		auto& it_as_array = std::get<Array>(it->GetValue());
		it_as_array.push_back({ Array() });
		unfinished_element_.push_back(&it_as_array.back());
	}
	command_list_.push_back(Command::START_ARRAY);
	return ArrayItemContext(*this);
}

Builder::BaseContext Builder::EndDict() {
	unfinished_element_.pop_back();
	command_list_.pop_back();
	return BaseContext(*this);
}

Builder::BaseContext Builder::EndArray() {
	unfinished_element_.pop_back();
	command_list_.pop_back();
	return BaseContext(*this);
}

Node Builder::Build() {
	return node_;
}
} //namespace json