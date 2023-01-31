#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T* () { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

class Options
{
public:

	OPTION(int, platform, 0);
	OPTION(int, timeout, 3);
	OPTION(bool, be, false);
};

Options g_Options;