#include <math.h>
#include <algorithm> 
#include <vector>
#include <unordered_map>
#include <brpc/controller.h>
#include "base_service.pb.h"
#include <fstream>
using std::pair;
using std::set;
using std::string;
using std::unordered_map;
using std::vector;

namespace NLP {

struct SimilarityCompare {
    SimilarityCompare(vector<double>& base) : _base(base) {}

    bool operator()(
        const pair<int, vector<double>>& lhs,
        const pair<int, vector<double>>& rhs) {  // 都与基准向量进行计算
        double lhsCrossProduct = 0;
        double rhsCrossProduct = 0;
        double lhsVectorLength = 0;
        double rhsVectorLength = 0;

        for (int index = 0; index != _base.size(); ++index) {
            lhsCrossProduct += (lhs.second)[index] * _base[index];
            rhsCrossProduct += (rhs.second)[index] * _base[index];
            lhsVectorLength += pow((lhs.second)[index], 2);
            rhsVectorLength += pow((rhs.second)[index], 2);
        }

        if (lhsCrossProduct / sqrt(lhsVectorLength) <
            rhsCrossProduct / sqrt(rhsVectorLength)) {
            return false;
        } else {
            return true;
        }
    }
    vector<double> _base;
};

    class SortServiceImp: public SortService
    {
        public:
            SortServiceImp() {
                load_invert_index();
            };
            virtual ~SortServiceImp() {};
            void sort(
                google::protobuf::RpcController *cntl_base,
                const SortRequest *request,
                SortResponse *response,
                google::protobuf::Closure *done
            )
            {
                brpc::ClosureGuard guard_done(done);
                brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);

                std::unordered_map<std::string, int> qtf_map;
                std::vector<std::string> querywords;
                auto pages = request->docids();

                for (auto it: request->cut_result())
                {
                    LOG(INFO) << "word = " << it.token() << " qtf = " << it.tf();
                    qtf_map[it.token()] = it.tf();
                    querywords.push_back(it.token());
                }
                LOG(INFO) << "queryweightstart";
                std::vector<double> queryweight;
                double weightSum = 0;
                
                for (auto& word : querywords) 
                {
                    int tf = qtf_map[word];
                    int df = invertIndex_[word].size();
                    double idf = log2(static_cast<double>(1000 / (df + 1)));
                    double w = idf * tf;
                    weightSum += pow(w, 2);
                    queryweight.push_back(w);
                }
                
                LOG(INFO) << "queryweight";
                vector<pair<int, vector<double>>> resultList;
                for(auto& docid : pages)
                {
                    vector<double> wordWeight;
                    for(auto& word : querywords)
                    {
                        auto tmpSet = invertIndex_.find(word)->second;
                        for(auto& item :tmpSet)
                        {
                            if(item.first == docid)
                            {
                                wordWeight.push_back(item.second);
                            }
                        }
                    }
                    resultList.emplace_back(std::make_pair(docid, wordWeight));
                }
                LOG(INFO) << "resultList";
                SimilarityCompare similarityCom(queryweight);
                std::stable_sort(resultList.begin(), resultList.end(), similarityCom);
                LOG(INFO) << "resultList_sort";
                //pages.clear();
                vector<int> result;

                for(auto& item : resultList)
                {
                    result.push_back(item.first);
                }

                for (auto DocId: result)
                {
                    response->add_docids(DocId);
                }
                LOG(INFO) << "response";
            }

            void load_invert_index() {
                std::ifstream Index("/home/ubuntu/tiny-search-engine/offline/data/invertIndex.lib");
                string word, line;
                int docid;
                double weight;
                while(getline(Index, line))
                {
                    std::istringstream iss(line);
                    iss >> word;
                    set<pair<int, double>> tempSet;
                    while(iss >> docid >> weight)
                    {
                        tempSet.insert(std::make_pair(docid, weight));
                    }
                    invertIndex_.insert(std::make_pair(word, tempSet));
                }
                Index.close();
                LOG(INFO) << "load invert index success！";
            }
            
        private:
            unordered_map<string, set<pair<int, double>>> invertIndex_;
    };
}
