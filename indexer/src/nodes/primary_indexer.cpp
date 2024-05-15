#include <indexer/nodes/primary_indexer.hpp>

namespace se {

namespace indexer {

PrimaryIndexer::PrimaryIndexer(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db) :
    CompressionBuffer{ 0 },
    db_{ db }
{ 
    options_ = opts.primary_options;
}

double PrimaryIndexer::get_weight(const std::string& tag) {
    return options_.resource_tag_weights.contains(tag)
        ? options_.resource_tag_weights.at(tag)
        : 0;
}

void PrimaryIndexer::operator()(const se::utils::CrawledResourceData& data, output_ports_type& ports) {     
    LOG_INFO_WITH_TAGS(
        logging::primary_indexer_category, 
        "New resource to index: {}.",
        data.url.c_str()
    );        
    Resource res { data.url, "", options_.resource_compression_type, "", 0 };
    {
        html_analyzer::HTMLAnalyzer analyzer{data.content};
        if (!analyzer.is_valid()) {
            LOG_ERROR_WITH_TAGS(
                logging::primary_indexer_category, 
                "Not valid HTML-resource {}.",
                data.url.c_str()
            );
            return;
        }
        res.content = analyzer.crop_content(options_.resource_skip_tags);
        res.size = res.content.size();
    }
    html_analyzer::PageInfo info;
    {
        html_analyzer::HTMLAnalyzer analyzer{ res.content };
        info = analyzer.analyze<HTMLAutomaton>();
    }
    res.title = info.title;
    compress_content(res);
    auto [res_id, upd] = db_->upload_resource(res);
    if (upd)
        return;
    if (!res_id.has_value()) {
        LOG_ERROR_WITH_TAGS(
            logging::primary_indexer_category, 
            "Failed to upload for indexing HTML-resource {}.",
            data.url.c_str()
        );
        return;            
    }
    std::unordered_map<Lexem, double> lexems;
    lexems.max_load_factor(0.2);
    split_to_lexems(info.keywords, info.language, get_weight("keywords"), lexems);
    split_to_lexems(info.description, info.language, get_weight("description"), lexems);
    std::for_each(info.excerpts.begin(), info.excerpts.end(), [&lexems, this](const auto& ex) {
        split_to_lexems(ex.text, ex.lang, ex.rank, lexems);  
    });
    std::vector<LexemEntry> entries;
    entries.reserve(lexems.size());
    std::transform(lexems.begin(), lexems.end(), std::back_inserter(entries), [](const auto& lex) {
        return LexemEntry{ lex.first, lex.second };
    });
    if (!entries.empty())
        db_->upload_lexem_entries(res_id.value(), entries);
    std::get<0>(ports).try_put({});
    LOG_INFO_WITH_TAGS(
        logging::primary_indexer_category, 
        "Resource {} was successfully indexed.",
        res.url.c_str()
    ); 
}

bool PrimaryIndexer::is_token_valid(const tqp::Token& s) {
    return 
        s.token_symbols_count >= options_.min_lexem_size && 
        s.token_symbols_count <= options_.max_lexem_size && 
        s.type != tqp::TokenType::NUMBER;
}

void PrimaryIndexer::split_to_lexems(
    const std::string& text, 
    const std::string& lang, 
    double weight, 
    std::unordered_map<Lexem, double> &cont
) {
    auto language = tqp::Language(lang);
    if (language.type() == tqp::LanguageType::LT_UNKNOWN) {
        LOG_TRACE_L1_WITH_TAGS(
            logging::primary_indexer_category, 
            "Met unsupported language: {}. Skipping excerpt...",
            lang
        );
        return;
    }
    for(auto&& token : TextParser{}(text, language)) {
        Lexem lex{ language, token.token };
        cont[lex] += weight;
    }
}

void PrimaryIndexer::compress_content(Resource& resource) {
    if (resource.compression_type.empty())
        return;
    auto sz =  resource.size;
    auto& buffer = CompressionBuffer::get_thread_resource();
    if (buffer.size() < sz)
        buffer.resize(sz * 2);
    std::string compressed;
    std::string error;
    if (se::utils::ComperssionHelper::compress_in_memory(
        resource.content, 
        compressed,
        resource.compression_type,
        buffer,
        error
    )) {
        LOG_ERROR_WITH_TAGS(
            logging::primary_indexer_category, 
            "Failed to compress resource {} with {}: {}.",
            resource.url.c_str(),
            resource.compression_type,
            error
        ); 
        resource.compression_type.clear();
    }
    else {
        resource.content = std::move(compressed);
    }
}

} // namespace indexer

} // namespace se