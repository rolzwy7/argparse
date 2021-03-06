#include <iterator>
#include <iostream>
#include <string>
#include "argparse.h"


int main(int argc, char * argv[]) {
	using argparse::ArgumentParser;
	using argparse::ArgType;
	using argparse::ArgImportance;
	using argparse::ArgSpecialBehavior;
	ArgumentParser parser("ArgumentParser Example", "Example", "rolzwy7");
	parser.add_argument("filepath", "Path to input file", ArgType::tString, ArgImportance::iPositional);
	parser.add_argument("output", "Path to output file", ArgType::tString, ArgImportance::iOptional);
	parser.add_argument("number-int", "Some int number", ArgType::tInt, ArgImportance::iOptional);
	parser.add_argument("number-float", "Some float number", ArgType::tFloat, ArgImportance::iOptional);
	parser.add_argument("number-double", "Some double number", ArgType::tDouble, ArgImportance::iOptional);
	parser.add_argument("v", "Verbose", ArgType::tStoreTrue, ArgImportance::iOptional);
	parser.add_argument("vv", "Very verbose", ArgType::tStoreTrue, ArgImportance::iOptional);
	parser.add_argument("no-smth", "Set some flag to false", ArgType::tStoreFalse, ArgImportance::iOptional);


	if (parser.parse_check_help(argc, argv)) {
		std::cout << parser.ret_help().str() << std::endl;
		return 0;
	}
	try {
		parser.parse_args(argc, argv);
	}
	catch (argparse::Error & err) {
		std::cout << "[-] " << err.what() << std::endl << std::endl;
		std::cout << parser.ret_help().str() << std::endl;
		return 1;
	}

	std::string filepath;
	std::string output;
	int number_int;
	float number_float;
	double number_double;
	bool v;
	bool vv;
	bool no_smth;

	if (parser.get_arg("filepath", filepath))
		std::cout << "filepath:" << filepath << std::endl;

	if (parser.get_arg("output", output))
		std::cout << "output:" << output << std::endl;

	if (parser.get_arg("number-int", number_int))
		std::cout << "number-int:" << number_int << std::endl;

	if (parser.get_arg("number-float", number_float))
		std::cout << "number-float:" << number_float << std::endl;

	if (parser.get_arg("number-double", number_double))
		std::cout << "number-double:" << number_double << std::endl;

	if (parser.get_arg("v", v))
		std::cout << "v:" << v << std::endl;

	if (parser.get_arg("vv", vv))
		std::cout << "vv:" << vv << std::endl;

	if (parser.get_arg("no-smth", no_smth))
		std::cout << "no-smth:" << no_smth << std::endl;

	return 0;
}
