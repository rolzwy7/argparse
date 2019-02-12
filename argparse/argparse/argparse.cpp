// MIT License
// 
// Copyright(c) 2018 Bartosz Nowakowski (https://github.com/rolzwy7)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "argparse.h"
#include <iostream>
#include <iomanip>

namespace argparse {

	void ArgumentParser::_init()
	{
		_max_arg_name_len = 0;
		this->_argc = 1;
		this->arguments_count = 0;
		this->positional_count = 0;
		this->optional_count = 0;
		#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		os_sep = '\\';
		#else
		os_sep = '/';
		#endif
		exec_name += ".exe";
		std::regex re_optional_argument("^-{1,2}([a-zA-z0-9]-{0,1}){0,}[a-zA-z0-9]+=(('[\\w\\W]+')|(\"[\\w\\W]+\")|([\\w\\W]+))$");
		this->re_optional_argument = re_optional_argument;
		std::regex re_float("^(([0-9]+)|([0-9]+.[0-9]+))$");
		this->re_float = re_float;
		std::regex re_int("^[0-9]+$");
		this->re_int = re_int;
	}

	void ArgumentParser::_parse_exec_name()
	{
		std::string temp = this->raw_arguments[0];
		while (temp.find(this->os_sep) != std::string::npos)
			temp.erase(0, temp.find(this->os_sep) + 1);
		this->exec_name = temp;
	}

	void ArgumentParser::_parse_positional()
	{
		if (this->positional_count == 0)
			return;
		if (this->positional_count > this->sanitized_arguments.size())
			throw Error(POSITIONAL_ERROR, "Not enough positional arguments provided");
		for (auto it = this->_arguments.begin(); it != this->_arguments.end(); ++it) {
			if (it->second.arg_imp != ArgImportance::iPositional)
				continue;
			int loc_pos = it->second.pos - 1; // -1 -> no executable path in sanitized_arguments
			size_t loc_size = this->sanitized_arguments.size() - 1;
			if ((loc_pos > loc_size) | (loc_pos < 0))
				continue;
			if (this->sanitized_arguments[loc_pos].find('-') == 0) {
				std::ostringstream err_msg;
				err_msg << "Positional argument can't start with '-' character" << std::endl;
				err_msg << "Error while parsing positional argument: " << it->second.arg_name;
				throw Error(POSITIONAL_ERROR, err_msg.str().c_str());
			}
			it->second.arg_str_value = this->sanitized_arguments[loc_pos];
			it->second.arg_set = true;
		}
	}

	void ArgumentParser::_parse_optional()
	{
		if (this->optional_count == 0) return;
		for (auto it = this->sanitized_arguments.begin(); it != this->sanitized_arguments.end(); ++it) {
			std::string temp = *it;
			if (temp.find('-') != 0) continue;
			while (temp.find('-') == 0) temp.erase(0, 1);
			// check map
			if (this->_arguments.count(temp) == 1) {
				if (this->_arguments[temp].arg_sb == ArgSpecialBehavior::DROP_POSITIONAL_CHECK)
					this->_drop_positional = true;
				// Optional types WITHOUT value
				if (this->_arguments[temp].arg_type == ArgType::tStoreTrue) {
					this->_arguments[temp].arg_str_value = "<store_true>";
					this->_arguments[temp].arg_set = true;
					continue;
				}
				if (this->_arguments[temp].arg_type == ArgType::tStoreFalse) {
					this->_arguments[temp].arg_str_value = "<store_false>";
					this->_arguments[temp].arg_set = true;
					continue;
				}
				// Optional types WITH value
				if (it + 1 != this->sanitized_arguments.end()) {
					this->_arguments[temp].arg_set = true;
					this->_arguments[temp].arg_str_value = *(it + 1);
				} else {
					std::string error_msg = "[argparse] Optional parameter '"+ temp +"' provided with no value";
					throw Error(OPTIONAL_PROVIDED_WITH_NO_VALUE, error_msg.c_str());
				}
			} else {
				continue;
			}
		}
	}

	void ArgumentParser::_sanitize_arguments_vector()
	{
		// alloc memory
		unsigned short to_alloc = 2 * this->optional_count + this->positional_count;
		this->sanitized_arguments.reserve(to_alloc);
		// split optional arguments with '=' character
		for (auto it = this->raw_arguments.begin() + 1; it != this->raw_arguments.end(); ++it) {
			if (this->is_optional(*it)) {
				auto ch = it->find('=');
				this->sanitized_arguments.push_back(it->substr(0, ch));
				this->sanitized_arguments.push_back(it->substr(ch + 1, it->length() - 1));
			} else {
				this->sanitized_arguments.push_back(*it);
			}
		}
		// trim ' character
		for (auto it = this->sanitized_arguments.begin(); it != this->sanitized_arguments.end(); ++it) {
			if (it->find_first_of('\'') == 0 && it->find_last_of('\'') == it->length()-1) {
				it->erase(0, 1);
				it->erase(it->length() - 1, 1);
			}
		}
	}

	void ArgumentParser::_convert_arguments()
	{
		auto get_err_msg = [](ArgConfig obj)->std::string {
			std::string ret = "Can't convert argument '" + obj.arg_name + "' to ";
			if (obj.arg_type == ArgType::tInt)
				ret += "int";
			if (obj.arg_type == ArgType::tFloat)
				ret += "float";
			if (obj.arg_type == ArgType::tDouble)
				ret += "double";
			ret += " | provided value: " + obj.arg_str_value;
			return ret;
		};

		for (auto it = _arguments.begin(); it != _arguments.end(); ++it) {
			if (it->second.arg_set) {
				// String
				if (it->second.arg_type == ArgType::tString) {
					_arg_map_string[it->first] = it->second.arg_str_value;
					continue;
				}
				// Integer
				if (it->second.arg_type == ArgType::tInt) {
					std::cmatch m;
					if (!std::regex_match(it->second.arg_str_value.c_str(), m, re_int))
						throw Error(CONVERT_ARG_ERROR, get_err_msg(it->second).c_str());
					std::istringstream iss(it->second.arg_str_value);
					int temp;
					iss >> temp;
					_arg_map_int[it->first] = temp;
					continue;
				}
				// Float
				if (it->second.arg_type == ArgType::tFloat) {
					std::cmatch m;
					if (!std::regex_match(it->second.arg_str_value.c_str(), m, re_float))
						throw Error(CONVERT_ARG_ERROR, get_err_msg(it->second).c_str());
					std::istringstream iss(it->second.arg_str_value);
					float temp;
					iss >> temp;
					_arg_map_float[it->first] = temp;
					continue;
				}
				// Double
				if (it->second.arg_type == ArgType::tDouble) {
					std::cmatch m;
					if (!std::regex_match(it->second.arg_str_value.c_str(), m, re_float))
						throw Error(CONVERT_ARG_ERROR, get_err_msg(it->second).c_str());
					std::istringstream iss(it->second.arg_str_value);
					double temp;
					iss >> temp;
					_arg_map_double[it->first] = temp;
					continue;
				}
				// Store True (set)
				if (it->second.arg_type == ArgType::tStoreTrue) {
					_arg_map_bool[it->first] = true;
					continue;
				}
				// Store False (set)
				if (it->second.arg_type == ArgType::tStoreFalse) {
					_arg_map_bool[it->first] = false;
					continue;
				}
			} // arg not set
			else {
				// Store True (not set)
				if (it->second.arg_type == ArgType::tStoreTrue) {
					_arg_map_bool[it->first] = false;
					continue;
				}
				// Store False (not set)
				if (it->second.arg_type == ArgType::tStoreFalse) {
					_arg_map_bool[it->first] = true;
					continue;
				}
			}
		}
	}

	ArgumentParser::ArgumentParser(const std::string & description, const std::string & app_name, const std::string & author):
		description(description),
		exec_name(app_name),
		author(author)
	{
		this->_init();
	}

	ArgumentParser::ArgumentParser():
		description("")
	{
		this->_init();
	}

	ArgumentParser::~ArgumentParser()
	{
	}

	bool ArgumentParser::parse_check_help(int argc, char * argv[]) {
		std::string help[2] = {"--help", "/?"};
		std::string temp("");
		for (int i = 0; i < argc; ++i) {
			temp = argv[i];
			for (int j = 0; j < 2; ++j) {
				if (temp == help[j])
					return true;
			}
		}
		return false;
	}

	void ArgumentParser::add_argument(std::string arg_name, std::string arg_help, ArgType arg_type, ArgImportance arg_imp, ArgSpecialBehavior arg_sb)
	{
		if (this->_arguments.count(arg_name) != 0)
			throw Error(DUPLICATE_ARGUMENT_ERROR, "");
		if (arg_imp == ArgImportance::iPositional) {
			++this->positional_count;
			this->_arguments[arg_name] = ArgConfig(arg_name, arg_help, arg_type, arg_imp, arg_sb, this->positional_count);
		}
		if (arg_imp == ArgImportance::iOptional) {
			++this->optional_count;
			this->_arguments[arg_name] = ArgConfig(arg_name, arg_help, arg_type, arg_imp, arg_sb, -1);
		}
		this->arguments_count++;
		if (_max_arg_name_len < (arg_name.length() * 2) + 4 + 3)
			_max_arg_name_len = (arg_name.length() * 2) + 4 + 3;
	}

	Error ArgumentParser::parse_args(int argc, char * argv[]) {
		_argc = argc;
		// Save command line params
		this->raw_arguments.reserve(argc);
		for (int it = 0; it < argc; ++it) {
			std::string temp = argv[it];
			this->raw_arguments.push_back(temp);
		}
		try {
			// Parse exec name
			this->_parse_exec_name();
			// Sanitize arguments vector for positionals and optionals
			this->_sanitize_arguments_vector();
			// Parse optional arguments
			this->_parse_optional();
			// Parse positional arguments
			if(!this->_drop_positional)
				this->_parse_positional();
			// Convert arguments
			this->_convert_arguments();
		}
		catch (Error & err) {
			throw err;
		}
		return Error();
	}

	bool ArgumentParser::is_optional(std::string str)
	{
		std::cmatch m;
		return std::regex_match(str.c_str(), m, re_optional_argument);
	}

	ArgConfig::ArgConfig(
		std::string arg_name,
		std::string arg_help,
		ArgType arg_type,
		ArgImportance arg_imp,
		ArgSpecialBehavior arg_sb,
		int pos
	):
		arg_name(arg_name),
		arg_help(arg_help),
		arg_type(arg_type),
		arg_imp(arg_imp),
		arg_sb(arg_sb),
		arg_set(false),
		arg_str_value(""),
		pos(pos)
	{}

	ArgConfig::ArgConfig():
		arg_set(false),
		arg_str_value("")
	{
	}

	Error::Error(ArgErrorCode code, const char *  msg):
		std::exception(msg),
		code(code),
		msg(msg)
	{
	}

	Error::Error():
		std::exception(""),
		code(NO_ERROR),
		msg("")
	{
	}


	std::ostringstream ArgumentParser::ret_help() {
		std::ostringstream oss;
		auto upper = [](ArgConfig arg)->std::string {
			std::string upper = arg.arg_name;
			if (arg.arg_type == ArgType::tStoreTrue || arg.arg_type == ArgType::tStoreFalse) {
				return "";
			}
			for (auto it = upper.begin(); it != upper.end(); ++it) {
				*it = ::toupper(*it);
			}
			return upper;
		};

		std::string optional_args_usage = "";
		std::string positional_args_usage = "";

		for(auto it=_arguments.begin(); it != _arguments.end(); ++it) {
			if (it->second.arg_imp == ArgImportance::iOptional) {
				optional_args_usage += "[-" + it->second.arg_name + "] ";
			}
			if (it->second.arg_imp == ArgImportance::iPositional) {
				positional_args_usage += it->second.arg_name + " ";
			}

		}

		oss << "usage: " << exec_name << " ";
		oss << positional_args_usage << std::endl;
		oss << optional_args_usage << std::endl;
		oss << std::endl;
		oss << " Author      : " << author << std::endl;
		oss << " Description : " << description << std::endl;

		oss << std::endl << "positional arguments:" << std::endl;
		for (auto it = _arguments.begin(); it != _arguments.end(); ++it) {

			if (it->second.arg_imp == ArgImportance::iPositional) {
				oss << std::left << std::setfill(' ') << std::setw(_max_arg_name_len)
					<< "  " + it->second.arg_name
					<< it->second.arg_help << std::endl;
			}

		}
		oss << std::endl;
		oss << "optional arguments:" << std::endl;
		for (auto it = _arguments.begin(); it != _arguments.end(); ++it) {
			if (it->second.arg_imp == ArgImportance::iOptional) {
				optional_args_usage = it->second.arg_name;

				oss << std::left << std::setfill(' ') << std::setw(_max_arg_name_len)
					<< "  -" + it->second.arg_name + " " + upper(it->second)
					<< it->second.arg_help << std::endl;

			}
		}
		return oss;
	}

} // argparse