#include "Get_Message.h"

#include <regex>
#include <string>

std::string Get_Message(std::string data)
{
	std::string pattern = "COMMAND===NAME_IDENTIFIER===\\(([[:print:]]+)\\)===MESSAGE===([[:print:]]*)";
	std::regex r(pattern);
	std::string fmt_message("$2");
	std::smatch match;
	if (std::regex_search(data, match, r))
	{
		return std::regex_replace(match.str(), r, fmt_message);
	}

	return "MESSAGE_ERROR";
}