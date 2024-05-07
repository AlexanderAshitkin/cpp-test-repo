#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <cmath>

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
        document_count_++;
        for (const auto &word: words) {
            if (!stop_words_.contains(word)) {
                word_to_doc_ids[word].insert(document_id);
                DocumentWord dw(document_id, word);
                doc_word_tf[dw] += 1.0 / words.size();
            }
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

    struct DocumentWord {

        int doc_id;
        string word;

        DocumentWord(int docId, const string &word) : doc_id(docId), word(word) {}

        bool operator==(const DocumentWord &rhs) const {
            return doc_id == rhs.doc_id &&
                   word == rhs.word;
        }

        bool operator!=(const DocumentWord &rhs) const {
            return !(rhs == *this);
        }
    };

    struct DocumentWordHash {
        auto operator()(const DocumentWord &document_word) const -> size_t {
            return 31 * hash<int>{}(document_word.doc_id) * hash<string>{}(document_word.word);
        }
    };

    int document_count_ = 0;
    unordered_map<DocumentWord, double, DocumentWordHash> doc_word_tf;
    unordered_map<string, set<int>> word_to_doc_ids;
    set<string> stop_words_;

    bool IsStopWord(const string &word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string &text) const {
        vector<string> words;
        for (const string &word: SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
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
        vector<Document> result;

        map<int, double> doc_to_relevance;

        for (const auto &pw: query.plus_words) {
            if (word_to_doc_ids.find(pw) != word_to_doc_ids.end()) {
                const set<int> &matched_docs_ids = word_to_doc_ids.at(pw);
                double word_idf = log(((double) document_count_) / matched_docs_ids.size());
                for (const auto &doc_id: matched_docs_ids) {
                    DocumentWord document_word(doc_id, pw);
                    double dtf = doc_word_tf.at(document_word);
                    doc_to_relevance[doc_id] += dtf * word_idf;
                }
            }
        }

        for (const auto &mw: query.minus_words) {
            if (word_to_doc_ids.find(mw) != word_to_doc_ids.end()) {
                const auto &matched_docs = word_to_doc_ids.at(mw);
                for (const auto &doc_id: matched_docs) {
                    doc_to_relevance.erase(doc_id);
                }
            }
        }

        for (const auto &[id, relevance]: doc_to_relevance) {
            Document doc;
            doc.id = id;
            doc.relevance = relevance;
            result.push_back(doc);
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

    const string query = ReadLine();
    for (const auto &[document_id, relevance]: search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}