#pragma once

class Preprocess {
	std::string						directory_;
	std::unordered_set<std::string> declare_;
	std::fstream					fin_;
	bool							condition_break_;
public:
	Preprocess(const std::string &path);
	std::string parse();
public:
	void parse(std::list<std::string> &strs);
	void insert_declare(const std::unordered_set<std::string> &other);
	std::string splic_directory(const std::string &path);
	static std::string trim_space(std::string &&str);
	void find_else_or_endif();
	void find_endif();
};