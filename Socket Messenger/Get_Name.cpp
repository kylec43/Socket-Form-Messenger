#include "Get_Name.h"

#include <regex>
#include <string>

std::string Get_Name(std::string data)
{
	std::string pattern = "COMMAND===NAME_IDENTIFIER===\\(([[:print:]]+)\\)===MESSAGE===([[:print:]]*)";
	std::regex r(pattern);
	std::string fmt_name("$1");
	std::smatch match;
	if (std::regex_search(data, match, r))
	{
		return std::regex_replace(match.str(), r, fmt_name);
	}

	return "ERROR_NAME";
}