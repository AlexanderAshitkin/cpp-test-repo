#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string &text) {
    vector<string> words;
    string word;
    for (const char c: text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {

    Document() {}

    Document(int id, double relevance) : id(id), relevance(relevance) {}

    int id;
    double relevance;
};

class SearchServer {
public:

    void SetStopWords(const string &text) {
        for (const string &word: SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string &document) {
        const vector<string> &words = SplitIntoWordsNoStop(document);
        const auto words_size = words.size();
        document_count_++;

        double tf_unit = 1.0 / words_size;
        for (const auto &word: words) {
            word_to_doc_and_tf[word][document_id] += tf_unit;
        }
    }

    vector<Document> FindTopDocuments(const string &raw_query) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document &lhs, const Document &rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;

    }

private:

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    int document_count_ = 0;
    map<string, map<int, double>> word_to_doc_and_tf;
    set<string> stop_words_;

    bool IsStopWord(const string &word) const {
        return stop_words_.contains(word);
    }

    vector<string> SplitIntoWordsNoStop(const string &text) const {
        const vector<string> &all_words = SplitIntoWords(text);
        vector<string> whitelisted;
        for (const string &word: all_words) {
            if (!IsStopWord(word)) {
                whitelisted.push_back(word);
            }
        }
        return whitelisted;
    }

    Query ParseQuery(const string &text) const {
        Query query;
        for (const string &word: SplitIntoWordsNoStop(text)) {
            if (word.starts_with('-')) {
                if (word.size() > 1) {
                    query.minus_words.insert(word.substr(1, word.length()));
                }
            } else {
                query.plus_words.insert(word);
            }
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query &query) const {

        map<int, double> doc_to_relevance;
        for (const string &plus_word: query.plus_words) {
            if (word_to_doc_and_tf.contains(plus_word)) {
                const map<int, double> &doc_to_tf = word_to_doc_and_tf.at(plus_word);
                const int total_docs_with_word = static_cast<int>(doc_to_tf.size());
                double idf = log(((double) document_count_) / total_docs_with_word);
                for (const auto &[doc_id, tf]: doc_to_tf) {
                    doc_to_relevance[doc_id] += tf * idf;
                }
            }
        }

        for (const string &minus_word: query.minus_words) {
            if (word_to_doc_and_tf.contains(minus_word)) {
                const auto &minus_docs = word_to_doc_and_tf.at(minus_word);
                for (const auto &[doc_id, _]: minus_docs) {
                    doc_to_relevance.erase(doc_id);
                }
            }
        }

        vector<Document> result;
        for (const auto &[id, relevance]: doc_to_relevance) {
            result.push_back(Document(id, relevance));
        }

        return result;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;

    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {

    const SearchServer search_server = CreateSearchServer();
    const string &query = ReadLine();
    const vector<Document> &documents = search_server.FindTopDocuments(query);
    for (const auto &[document_id, relevance]: documents) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }

}