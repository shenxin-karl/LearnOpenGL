#include "common.h"

Preprocess::Preprocess(const std::string &path) : fin_(path, std::ios::in), condition_break_(false) {
	if (!fin_.is_open()) {
		std::cerr << path << "can be not find" << std::endl;
		assert(false);
		return;
	}

	auto npos = path.find_last_of("/");
	if (npos != std::string::npos)
		directory_ = path.substr(0, npos) + "/";
}

void Preprocess::parse(std::list<std::string> &strs) {
	constexpr std::string_view keyword1 = "#define ";
	constexpr std::string_view keyword2 = "#include ";
	constexpr std::string_view keyword3 = "#ifndef ";
	constexpr std::string_view keyword4 = "#ifdef ";
	constexpr std::string_view keyword5 = "#else";

	std::string line;
	while (!fin_.eof()) {
		std::getline(fin_, line);
		if (line.compare(0, keyword1.length(), keyword1) == 0) {			// #define
			declare_.insert(trim_space(line.substr(keyword1.length()-1)));
		} else if (line.compare(0, keyword2.length(), keyword2) == 0) {		// #include
			std::string path = splic_directory(line.substr(keyword2.length()-1));
			Preprocess pps(path);
			pps.insert_declare(declare_);
			std::list<std::string> result;
			pps.parse(result);
			strs.insert(strs.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
			continue;
		} else if (line.compare(0, keyword3.length(), keyword3) == 0) {			// #ifndef
			std::string key = trim_space(line.substr(keyword3.length()));
			if (declare_.find(key) != declare_.end())
				find_else_or_endif();
			else
				condition_break_ = true;
		} else if (line.compare(0, keyword4.length(), keyword4) == 0) {			// #ifdef
			std::string key = trim_space(line.substr(keyword4.length()));
			if (declare_.find(key) == declare_.end())
				find_else_or_endif();
			else
				condition_break_ = true;
		} else if (condition_break_ && line.compare(0, keyword5.length(), keyword5) == 0) {		//#else
			find_endif();
			condition_break_ = false;
		}
		strs.emplace_back(std::move(line));
	}
}

std::string Preprocess::parse() {
	std::stringstream sos;
	std::list<std::string> strs;
	parse(strs);
	std::move(strs.begin(), strs.end(), std::ostream_iterator<std::string>(sos, "\n"));
	return sos.str();
}

void Preprocess::insert_declare(const std::unordered_set<std::string> &other) {
	declare_.insert(other.begin(), other.end());
}

std::string Preprocess::splic_directory(const std::string &path) {
	auto first = path.find("\"");
	auto last = path.find_last_of("\"");
	auto new_path = path.substr(first+1, last - first - 1);

	std::string res = directory_;
	if (path.compare("./") == 0) {
		res += new_path.substr(2);
		return res;
	} 
#ifdef WIN32
	if (path.find(":/") != std::string::npos || path.find(":\\") != std::string::npos) {
		res = new_path;
		return res;
	}
#endif
	res += new_path;
	return res;
}


std::string Preprocess::trim_space(std::string &&str) {
	if (str.empty())
		return str;

	int start = 0;
	int last = static_cast<int>(str.size()) - 1;
	while (start < str.size() && str[start] == ' ')
		++start;
	while (last > start && str[last] == ' ')
		--last;
	if (start == 0 && last == str.size()-1)
		return str;

	str = str.substr(start, last - start + 1);
	return str;
}

void Preprocess::find_else_or_endif() {
	int size = 1;
	std::string line;
	while (!fin_.eof() && size > 0) {
		std::getline(fin_, line);
		if (line.compare(0, 3, "#if") == 0)
			++size;
		else if (line.compare(0, 6, "#ifdef") == 0)
			++size;
		else if (line.compare(0, 7, "#ifndef") == 0)
			++size;
		else if (line.compare(0, 6, "#endif") == 0)
			--size;
		else if (line.compare(0, 5, "#else") == 0)
			--size;
	}
}

void Preprocess::find_endif() {
	int size = 1;
	std::string line;
	while (!fin_.eof() && size > 0) {
		std::getline(fin_, line);
		if (line.compare(0, 3, "#if") == 0)
			++size;
		else if (line.compare(0, 6, "#ifdef") == 0)
			++size;
		else if (line.compare(0, 7, "#ifndef") == 0)
			++size;
		else if (line.compare(0, 6, "#endif") == 0)
			--size;
	}

}

