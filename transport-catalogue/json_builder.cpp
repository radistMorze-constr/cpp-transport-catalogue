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
	if (!CommandIsCorrect(Command::start_dict)) {
		throw std::logic_error("Not correct call of command \"StartDict\"!"s);
	}
	if (command_list_.empty()) {
		auto* it = unfinished_element_.back();
		*it = { Dict() };
	}
	else if (command_list_.back() == Command::key) {
		auto* it = unfinished_element_.back();
		*it = { Dict() };
		command_list_.pop_back();
	}
	else if (command_list_.back() == Command::start_array) {
		auto* it = unfinished_element_.back();
		auto& it_as_array = std::get<Array>(it->GetValue());
		it_as_array.push_back({ Dict() });
		unfinished_element_.push_back(&it_as_array.back());
	}
	command_list_.push_back(Command::start_dict);
	return DictItemContext(*this);
}

Builder::DictKeyContext Builder::Key(std::string&& key) {
	if (!CommandIsCorrect(Command::key)) {
		throw std::logic_error("Not correct call of command \"Key\"!"s);
	}
	command_list_.push_back(Command::key);
	auto* it = unfinished_element_.back();
	auto& it_as_dict = std::get<Dict>(it->GetValue());
	unfinished_element_.push_back(&(it_as_dict[std::move(key)]));
	return DictKeyContext(*this);
}

Builder::BaseContext Builder::Value(Node::Value&& value) {
	if (!CommandIsCorrect(Command::value)) {
		throw std::logic_error("Not correct call of command \"Value\"!"s);
	}
	if (command_list_.empty()) {
		auto* it = unfinished_element_.back();
		it->GetValue() = std::move(value);
		command_list_.push_back(Command::value);
		return BaseContext(*this);
	}
	else if (command_list_.back() == Command::key) {
		auto* it = unfinished_element_.back();
		it->GetValue() = std::move(value);
		unfinished_element_.pop_back();
		command_list_.pop_back();
		return DictValueContext(*this);
	}
	//Последняя команда Command::start_array
	else {
		auto* it = unfinished_element_.back();
		auto& it_as_array = std::get<Array>(it->GetValue());
		it_as_array.push_back({ });
		it_as_array.back().GetValue() = std::move(value);
		return ArrayValueContext(*this);
	}
}

Builder::ArrayItemContext Builder::StartArray() {
	if (!CommandIsCorrect(Command::start_array)) {
		throw std::logic_error("Not correct call of command \"StartArray\"!"s);
	}
	if (command_list_.empty()) {
		auto* it = unfinished_element_.back();
		*it = { Array() };
	}
	else if (command_list_.back() == Command::key) {
		auto* it = unfinished_element_.back();
		*it = { Array() };
		command_list_.pop_back();
	}
	else if (command_list_.back() == Command::start_array) {
		auto* it = unfinished_element_.front();
		auto& it_as_array = std::get<Array>(it->GetValue());
		it_as_array.push_back({ Array() });
		unfinished_element_.push_back(&it_as_array.back());
	}
	command_list_.push_back(Command::start_array);
	return ArrayItemContext(*this);
}

Builder::BaseContext Builder::EndDict() {
	if (!CommandIsCorrect(Command::end_dict)) {
		throw std::logic_error("Not correct call of command \"EndDict\"!"s);
	}
	unfinished_element_.pop_back();
	command_list_.pop_back();
	return BaseContext(*this);
}

Builder::BaseContext Builder::EndArray() {
	if (!CommandIsCorrect(Command::end_array)) {
		throw std::logic_error("Not correct call of command \"EndArray\"!"s);
	}
	unfinished_element_.pop_back();
	command_list_.pop_back();
	return BaseContext(*this);
}

Node Builder::Build() {
	if ((command_list_.empty() || command_list_.back() == Command::value) && !node_.IsNull()) {
		return node_;
	}
	throw std::logic_error("Not correct call of command \"Build\"!"s);
}

bool Builder::CommandIsCorrect(Command value) {
	if (value == Command::key) {
		if (!command_list_.empty() && command_list_.back() == Command::start_dict) {
			return true;
		}
		return false;
	}
	else if (value == Command::value || value == Command::start_dict || value == Command::start_array) {
		if (command_list_.empty() || command_list_.back() == Command::key || command_list_.back() == Command::start_array) {
			return true;
		}
		return false;
	}
	else if (value == Command::end_dict) {
		if (!command_list_.empty() && command_list_.back() == Command::start_dict) {
			return true;
		}
		return false;
	}
	else {
		if (!command_list_.empty() && command_list_.back() == Command::start_array) {
			return true;
		}
		return false;
	}
}
} //namespace json