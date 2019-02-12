# argparse

Argument parser for C++ console applications inspired slightly by Python [argparse](https://docs.python.org/3/library/argparse.html) module.

---

#### Usage example
See [main.cpp](https://github.com/rolzwy7/argparse/blob/master/argparse/argparse/main.cpp)

#### Defining console arguments
```c++
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
```

#### Handling exceptions & displaying help
````c++
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
````

#### Getting argument value
```c++
std::string filepath;
bool is_set_filepath;
// filepath is set by reference
is_set_filepath = parser.get_arg("filepath", filepath)
if(is_set_filepath)
    std::cout << filepath << std::endl;
```
