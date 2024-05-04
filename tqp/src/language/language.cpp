#include <tqp/language/language.hpp>

namespace tqp {

const boost::bimap<std::string, Language> Language::code_type_interpreter_ =
	boost::assign::list_of<boost::bimap<std::string, Language>::relation>
	( "en",			LanguageType::LT_ENGLISH   	)
	( "ru",			LanguageType::LT_RUSSIAN   	)
	( "unknown", 	LanguageType::LT_UNKNOWN 	);

const Language Language::default_lang_ = LanguageType::LT_UNKNOWN;

Language::LanguageEntry Language::find_language(const std::string& text) {
	auto sz = static_cast<int>(text.size());
	chrome_lang_id::NNetLanguageIdentifier net{ 0, sz * 4 };
	auto res = net.FindLanguage(text);
	return LanguageEntry {
		0,
		static_cast<int>(text.size()),
		res.probability,
		Language(res.language)
	};
}

std::vector<Language::LanguageEntry> Language::find_top_nmost_freq_langs (
	const std::string& text, 
	double threshold, 
	int max_count
) {
	std::vector<Language::LanguageEntry> res;
	auto sz = static_cast<int>(text.size());
	chrome_lang_id::NNetLanguageIdentifier net{ 0, sz * 4 };
	auto info = net.FindTopNMostFreqLangs(text, max_count);
	for(auto& lang : info) {
		for(auto& entry : lang.byte_ranges) {
			if (entry.probability < threshold)
				continue;
			res.emplace_back(entry.start_index, entry.end_index, entry.probability, Language(lang.language));
		}
	}
	std::sort(res.begin(), res.end(), [](const auto& v1, const auto& v2) {
		return v1.begin < v2.begin;
	});
	return res;
}

Language::Language() : type_{ default_lang_.type() }
{ }

Language::Language(LanguageType type) : type_(type)
{ }

Language::Language(size_t num) : Language(static_cast<LanguageType>(num))
{ }

Language::Language(const std::string& code) : type_(get_type_by_code(code))
{ }

size_t Language::number() const {
	return static_cast<size_t>(type_);
}

LanguageType Language::type() const {
	return type_;
}

std::string Language::name() const {
	return code_type_interpreter_.right.at(type_); 
}

Language::operator LanguageType() const {
	return type_;
}

bool Language::operator==(Language lang) const {
	return type_ == lang.type_;
}

bool Language::operator!=(Language lang) const {
	return type_ != lang.type_;
}

LanguageType Language::get_type_by_code(const std::string& code) {
	auto& cont = code_type_interpreter_.left;
	return cont.find(code) != cont.end()
		? cont.at(code) 
		: default_lang_;
}

std::ostream& operator<<(std::ostream& ostr, const Language& obj) {
	ostr << obj.type();
	return ostr;
}

} // namespace tqp