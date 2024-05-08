#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <fstream>
#include <chrono>

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
        const auto words_size = words.size();
        document_count_++;

        unordered_map<string, double> &frequencies = doc_word_tf[document_id];
        for (const auto &word: words) {
            word_to_doc_ids[word].insert(document_id);
            frequencies[word] += 1.0 / words_size;
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
        unordered_set<string> plus_words;
        unordered_set<string> minus_words;
    };

    int document_count_ = 0;
    unordered_map<int, unordered_map<string, double>> doc_word_tf;
    unordered_map<string, set<int>> word_to_doc_ids;
    unordered_set<string> stop_words_;

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

        for (const auto &pw: query.plus_words) {
            if (word_to_doc_ids.contains(pw)) {
                const set<int> &matched_docs_ids = word_to_doc_ids.at(pw);
                const int matched_docs_size = static_cast<int>(matched_docs_ids.size());
                double word_idf = log(((double) document_count_) / matched_docs_size);
                for (const auto &doc_id: matched_docs_ids) {
                    double dtf = doc_word_tf.at(doc_id).at(pw);
                    doc_to_relevance[doc_id] += dtf * word_idf;
                }
            }
        }

        unordered_set<int> minus_docs;
        for (const auto &mw: query.minus_words) {
            if (word_to_doc_ids.contains(mw)) {
                const auto &matched_docs = word_to_doc_ids.at(mw);
                minus_docs.insert(matched_docs.begin(), matched_docs.end());
            }
        }

        for (const auto &doc_id: minus_docs) {
            doc_to_relevance.erase(doc_id);
        }

        vector<Document> result;
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

//    search_server.SetStopWords("is are was a an in the with near at"s);
//    ifstream file("./book-war-and-peace.txt");
//
//    cout << "reading file"s << endl;
//    string line;
//    int doc_counter = 0;
//    if (file.is_open()) {
//        while (getline(file, line)) {
//            if (doc_counter % 1000 == 0) {
//                cout << "indexing line "s << doc_counter << endl;
//            }
//            search_server.AddDocument(doc_counter++, line);
//        }
//        file.close();
//    }

    return search_server;
}

int main() {

//    auto start_indexing = std::chrono::high_resolution_clock::now();
    const SearchServer search_server = CreateSearchServer();
//    auto elapsed_indexing = std::chrono::high_resolution_clock::now() - start_indexing;
//    long elapsed_indexing_millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_indexing).count();


//    const string query = "white horse long tail";
    const string &query = ReadLine();

//    auto start_searching = std::chrono::high_resolution_clock::now();

    const vector<Document> &documents = search_server.FindTopDocuments(query);

//    auto elapsed_searching = std::chrono::high_resolution_clock::now() - start_searching;
//    long elapsed_searching_micros = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_searching).count();
//
//    cout << "indexing: "s << elapsed_indexing_millis << " millis, searching: "s << elapsed_searching_micros
//         << " micros"s
//         << endl;

    for (const auto &[document_id, relevance]: documents) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}