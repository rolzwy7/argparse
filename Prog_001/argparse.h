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
#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <regex>

namespace argparse {

	enum ArgErrorCode {
		NO_ERROR,
		POSITIONAL_ERROR,
		DUPLICATE_ARGUMENT_ERROR,
		INVALID_OS_SEP,
		RAW_VECTOR_OUT_OF_RANGE,
		OPTIONAL_PROVIDED_WITH_NO_VALUE,
		CONVERT_ARG_ERROR
	};

	struct Error: public std::exception {
		ArgErrorCode code;
		const char * msg;
		Error(ArgErrorCode code, const char *  msg);
		Error();
	};

	enum class ArgSpecialBehavior {
		None,
		DROP_POSITIONAL_CHECK
	};

	enum class ArgType {
		tStoreFalse,
		tStoreTrue,
		tString,
		tFloat,
		tDouble,
		tInt
	};

	enum class ArgImportance {
		iPositional,
		iOptional
	};

	struct ArgConfig {
		std::string arg_name;
		std::string arg_help;
		ArgType arg_type;
		ArgImportance arg_imp;
		ArgSpecialBehavior arg_sb;
		bool arg_set;
		std::string arg_str_value;
		int pos;
		ArgConfig(std::string arg_name, std::string arg_help,
				  ArgType arg_type, ArgImportance arg_imp,
				  ArgSpecialBehavior arg_sb, int pos);
		ArgConfig();
	};

	class ArgumentParser {
	public:
		inline bool get_arg(std::string name, std::string & target) {
			target = (_arg_map_string.count(name)) ? _arg_map_string[name] : "";
			return (target != "") ? true : false;
		}

		inline bool get_arg(std::string name, int & target) {
			target = (_arg_map_int.count(name)) ? _arg_map_int[name] : 0;
			return (target != 0) ? true : false;
		}

		inline bool get_arg(std::string name, float & target) {
			target = (_arg_map_float.count(name)) ? _arg_map_float[name] : 0;
			return (target != 0) ? true : false;
		}

		inline bool get_arg(std::string name, double & target) {
			target = (_arg_map_double.count(name)) ? _arg_map_double[name] : 0;
			return (target != 0) ? true : false;
		}

		inline bool get_arg(std::string name, bool & target) {
			if (_arg_map_bool.count(name)) {
				target = _arg_map_bool[name];
				return true;
			}
			return false;
		}
	private:
		// maps
		std::map<std::string, std::string> _arg_map_string;
		std::map<std::string, int> _arg_map_int;
		std::map<std::string, double> _arg_map_double;
		std::map<std::string, float> _arg_map_float;
		std::map<std::string, bool> _arg_map_bool;
		
		unsigned short arguments_count;
		unsigned short positional_count;
		unsigned short optional_count;
		int _argc;
		std::string exec_name;
		std::string description;
		char os_sep;
		std::vector<std::string> raw_arguments;
		std::vector<std::string> sanitized_arguments;
		std::map<std::string, ArgConfig> _arguments;
		bool _drop_positional;
		std::regex re_optional_argument;
		std::regex re_float;
		std::regex re_int;

		void _init();
		void _parse_exec_name();
		void _parse_positional();
		void _parse_optional();
		void _sanitize_arguments_vector();
		void _convert_arguments();
	public:
		ArgumentParser(const std::string & description, const std::string & app_name);
		ArgumentParser();
		~ArgumentParser();

		Error parse_args(int argc, char * argv[]);

		inline unsigned short get_arguments_count() const {
			return this->arguments_count;
		}

		bool is_optional(std::string str);

		inline bool is_drop_positional_check() {
			return this->_drop_positional;
		}

		inline std::string get_description() const {
			return this->description;
		}

		inline void set_description(std::string description) {
			this->description = description;
		}

		void add_argument(
			std::string arg_name,
			std::string arg_help,
			ArgType arg_type = ArgType::tString,
			ArgImportance arg_imp = ArgImportance::iPositional,
			ArgSpecialBehavior arg_sb = ArgSpecialBehavior::None
		);

	};

} // argparse