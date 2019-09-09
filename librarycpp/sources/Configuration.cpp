#include "Configuration.hpp"
#include "StringEx.hpp"

namespace CoreLib
{
	Configuration::Configuration()
	{
		_ConfigFileName = "";
	}

	Configuration::~Configuration()
	{
	}

	void Configuration::setFileName(std::string fname)
	{
		_ConfigFileName = fname;
	}

	bool Configuration::isSection(const std::string &section)
	{
		if (_Configuration.find(section) == _Configuration.end())
		{
			return false;
		}

		return true;
	}

	std::string Configuration::getValue(const std::string &section, const std::string &settingKey, const std::string defval)
	{
		std::string str = defval;

		if (isSection(section))
		{
			std::map<std::string, std::string> sub_section = _Configuration[section];

			if (sub_section.find(settingKey) != sub_section.end())
			{
				str = sub_section[settingKey];
			}
		}

		return str;
	}

	bool Configuration::loadConfiguration(const std::string &configFile)
	{
		_ConfigFileName = configFile;
		return loadConfiguration();
	}

	bool Configuration::loadConfiguration()
	{
		std::string line, key, value;
		std::string current_section_header = "";
		std::map<std::string, std::string> kvlist;

		std::ifstream file;

		file.open(_ConfigFileName);

		while (file.good())
		{
			line.clear();
			std::getline(file, line);
			//Section header line
			if (line[0] == '[' && line[line.length() - 1] == ']')
			{
				//Check whether this is the first instance of a section header
				if (_Configuration.size() < 1)
				{
					//Don't need to do anything
					if (current_section_header.length() > 1)
					{
						//We reach here when a section is being read for the first time
						addSection(current_section_header, kvlist);
					}
				}
				else
				{
					//Before staring a new section parsing we need to store the last one
					addSection(current_section_header, kvlist);
				}

				//Store the string as current section header and clear the key value list
				current_section_header = line;
				kvlist.clear();
			}
			else
			{
				key = "";
				value = "";
				strsplit(line, '=', key, value);
				kvlist[key] = value;
			}

		}
		addSection(current_section_header, kvlist);
		file.close();
		return true;
	}

	void Configuration::addSection(std::string &str, const std::map<std::string, std::string> &list)
	{
		strremove(str, '[');
		strremove(str, ']');
		_Configuration[str] = list;
	}
}
