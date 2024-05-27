#pragma once

#include <ostream>
#include <functional>
#include <string>
#include <sstream>
#include <algorithm>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>
#include <cld3/src/nnet_language_identifier.h>
#include "language_type.hpp"

namespace tqp {

class Language final {
public:
	struct LanguageEntry;

public:
	Language();
	Language(LanguageType type);
	explicit Language(size_t num);
	explicit Language(const std::string& code);

	static struct LanguageEntry find_language(const std::string& text);
	static std::vector<struct LanguageEntry> find_top_nmost_freq_langs(
		const std::string& text, 
		double threshold, 
		int max_count = std::numeric_limits<int>::max()
	);

	size_t number() const;
	LanguageType type() const;
	std::string name() const;

	operator LanguageType() const;

	bool operator==(Language lang) const;
	bool operator!=(Language lang) const;

	friend std::ostream& operator<<(std::ostream& ostr, const Language& obj);

private:
	LanguageType get_type_by_code(const std::string& code);

private:
	static const boost::bimap<std::string, Language> code_type_interpreter_;
	static const Language default_lang_;

private:
	LanguageType type_;
};

struct Language::LanguageEntry {
	int begin;
	int end;
	double probability;
	Language lang;
};

} // namespace tqp

namespace std {

template<>
struct hash<tqp::Language> {
	size_t operator()(const tqp::Language& lang) const noexcept {
		return hash<size_t>()(lang.number());
	}
};

template<>
struct less<tqp::Language> {
	bool operator()(
        const tqp::Language& lhs, 
        const tqp::Language& rhs
    ) const noexcept {
		return less<size_t>()(lhs.number(), rhs.number());
	}
};

template<>
struct equal_to<tqp::Language> {
	bool operator()(
        const tqp::Language& lhs, 
        const tqp::Language& rhs
    ) const noexcept {
		return equal_to<std::size_t>()(lhs.number(), rhs.number());
	}
};

} // namespace std
