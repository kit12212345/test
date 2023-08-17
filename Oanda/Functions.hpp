#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <codecvt>
#include <regex>

inline std::wstring strToWStr(std::string str) {
	return std::wstring(str.begin(), str.end());
}

inline std::string wstrToStr(std::wstring wstr) {
	std::string str;
	std::transform(wstr.begin(), wstr.end(), std::back_inserter(str), [](wchar_t c) {
		return (char)c;
		});
	return str;
}

inline std::string ltrim(std::string str) {
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
		return !isspace(ch);
		}));
	return str;
}
inline std::string rtrim(std::string str) {
	str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
		return !isspace(ch);
		}).base(), str.end());
	return str;
}

inline std::string trim(std::string str) {
	return ltrim(rtrim(str));
}

inline std::string removeAllSpaces(std::string str) {
	str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char x) { return isspace(x); }), str.end());
	return str;
}

inline bool isFloat(std::string str) {
	std::istringstream iss(str);
	float f;
	iss >> std::noskipws >> f;
	return iss.eof() && !iss.fail();
}
inline bool isFloat(std::wstring str) {
	std::wistringstream iss(str);
	float f;
	iss >> std::noskipws >> f;
	return iss.eof() && !iss.fail();
}

inline bool isNumber(std::string str) {
	return std::regex_match(str, std::regex("-?[0-9]+([\\.][0-9]+)?"));;
}

inline bool isNumber(std::wstring str) {
	return !str.empty() && std::find_if(str.begin(),
		str.end(), [](wchar_t c) { return !std::isdigit(c); }) == str.end();
}

inline int strToInt(std::string str) {
	if (str.length() == 0) return 0;
	if (!isNumber(str)) return 0;
	return std::stoi(str);
}

inline int strToInt(std::wstring str) {
	if (str.length() == 0) return 0;
	if (!isNumber(str)) return 0;
	return std::stoi(str);
}

inline float strToFLoat(std::string str) {
	float val = 0.00;
	if (str.length() == 0) return val;
	if (!isFloat(str)) return val;
	val = std::stof(str);
	return val;
}

inline float strToFLoat(std::wstring str) {
	float val = 0.00;
	if (str.length() == 0) return val;
	if (!isFloat(str)) return val;
	val = std::stof(str);
	return val;
}

inline void replaceFirst(std::wstring& str, std::wstring fv, std::wstring val) {
	std::size_t pos = str.find(fv);
	if (pos == std::wstring::npos) return;
	str.replace(pos, fv.length(), val);
}
inline void replaceFirst(std::string& str, std::string fv, std::string val) {
	std::size_t pos = str.find(fv);
	if (pos == std::string::npos) return;
	str.replace(pos, fv.length(), val);
}

inline std::string strToLow(std::string str) {
	std::for_each(str.begin(), str.end(), [](char& c) {
		c = ::tolower(c);
		});
	return str;
}

inline std::wstring wstrToLow(std::wstring str) {
	std::for_each(str.begin(), str.end(), [](wchar_t& c) {
		c = ::tolower(c);
		});
	return str;
}

inline std::string strToUp(std::string str) {
	std::for_each(str.begin(), str.end(), [](char& c) {
		c = ::toupper(c);
		});
	return str;
}

inline std::wstring wstrToUp(std::wstring str) {
	std::for_each(str.begin(), str.end(), [](wchar_t& c) {
		c = ::toupper(c);
		});
	return str;
}

inline std::vector<std::string> splitStr(std::string s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

inline bool IsFileExists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

inline std::string FloatToStr(float v, int p = 2) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(p) << v;
	std::string s = stream.str();
	return s;
}

inline std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

inline std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}