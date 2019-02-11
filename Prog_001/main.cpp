#include <iterator>
#include <iostream>
#include <string>
#include "argparse.h"

/*
	Przejechać to fuzzerem
*/

int main(int argc, char * argv[]) {
	using argparse::ArgumentParser;
	using argparse::ArgType;
	using argparse::ArgImportance;
	using argparse::ArgSpecialBehavior;

	ArgumentParser parser("Test ArgumentParser Description", "ProgramName");
	parser.add_argument("string", "help", ArgType::tString, ArgImportance::iPositional);
	parser.add_argument("float", "help", ArgType::tFloat, ArgImportance::iPositional);
	parser.add_argument("strue", "help", ArgType::tStoreTrue, ArgImportance::iOptional);
	parser.add_argument("sfalse", "help", ArgType::tStoreFalse, ArgImportance::iOptional);
	
	try {
		parser.parse_args(argc, argv);
	}
	catch (argparse::Error & err) {
		std::cout << err.what() << std::endl;
		return 1;
	}

	std::string arg_string;
	float arg_float;
	bool arg_strue;
	bool arg_sfalse;

	if (parser.get_arg("string", arg_string))
		std::cout << arg_string << std::endl;

	if (parser.get_arg("float", arg_float))
		std::cout << arg_float << std::endl;

	if (parser.get_arg("strue", arg_strue))
		std::cout << arg_strue << std::endl;

	if (parser.get_arg("sfalse", arg_sfalse))
		std::cout << arg_sfalse << std::endl;

	return 0;
}
