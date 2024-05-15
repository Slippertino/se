#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <tbb/flow_graph.h>
#include <indexer/db/data_provider.hpp>
#include <indexer/indexing_options.hpp>
#include <indexer/models/resource.hpp>
#include <indexer/models/lexem.hpp>
#include <indexer/caches/memory_buffer_tags.hpp>
#include <indexer/logging/logging.hpp>
#include <seutils/models/resource_data.hpp>
#include <seutils/threaded_memory_buffer.hpp>
#include <seutils/compression_helper.hpp>
#include <htmlanalyzer/html_analyzer.hpp>
#include <htmlanalyzer/automatons/automatons.hpp>
#include <tqp/parser.hpp>
#include <tqp/transformers/transformers.hpp>
#include <tqp/lexers/basic_lexer.hpp>

namespace se {

namespace indexer {

class PrimaryIndexer : public se::utils::ThreadedMemoryBuffer<crawled_resource_compression_tag> {
private:
    using CompressionBuffer = se::utils::ThreadedMemoryBuffer<crawled_resource_compression_tag>;
    using output_ports_type = tbb::flow::multifunction_node<
        se::utils::CrawledResourceData, 
        std::tuple<tbb::flow::continue_msg>
    >::output_ports_type;

public:
    PrimaryIndexer(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db);
    static double get_weight(const std::string& tag);
    void operator()(const se::utils::CrawledResourceData& data, output_ports_type& ports);

private:
    static bool is_token_valid(const tqp::Token& s);
    void split_to_lexems(
        const std::string& text, 
        const std::string& lang, 
        double weight, 
        std::unordered_map<Lexem, double> &cont
    );
    void compress_content(Resource& resource);

private:
    static inline PrimaryIndexerOptions options_;
    std::shared_ptr<IDataProvider> db_;

private:
    using HTMLAutomaton = html_analyzer::CombinedAutomaton<
        html_analyzer::KeywordsAutomaton,
        html_analyzer::DescriptionAutomaton,
        html_analyzer::TitleAutomaton,
        html_analyzer::TextAutomaton<decltype([](const std::string& tag){ return PrimaryIndexer::get_weight(tag); })>
    >;

    using TextParser = tqp::TQParser<
        tqp::BasicLexer, 
        tqp::CombinedTransformer<
            tqp::WordsCaseTransformer<>,
            tqp::StopWordsTransformer,
            tqp::StemmingTransformer,
            tqp::FilterTransformer<decltype([](auto& token, auto) { 
                return !PrimaryIndexer::is_token_valid(token); 
            })>
        >
    >;
};

} // namespace indexer

} // namespace se