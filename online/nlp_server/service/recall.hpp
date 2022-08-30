#include <fstream>
#include "base_service.pb.h"
#include <brpc/controller.h>

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <algorithm>
#include <cmath>

using std::pair;
using std::set;
using std::string;
using std::unordered_map;
using std::vector;


namespace NLP {

    const char* const INVERT_INDEX_PATH = "/home/ubuntu/tiny-search-engine/offline/data/invertIndex.lib";

    class RecallServiceImp: public RecallService
    {
        public:
            RecallServiceImp() {
                load_invert_index();
            };
            virtual ~RecallServiceImp() {};
            void recall(
                google::protobuf::RpcController *cntl_base,
                const RecallRequest *request,
                RecallResponse *response,
                google::protobuf::Closure *done
            )
            {
                brpc::ClosureGuard guard_done(done);
                brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);
                vector<int> pages;
                for (auto word: request->tokens())
                {
                    auto it = invertIndex_.find(word);
                    if(it == invertIndex_.end())
                    {
                        LOG(INFO) << "not found the word " << word << " at invert index ";
                        continue;
                    }
                    else
                    {
                        for(auto& item : invertIndex_[word])
                            pages.push_back(item.first);
                    }
                }
                std::unique(pages.begin(),pages.end());
                for (auto DocId: pages)
                {
                    response->add_docids(DocId);
                    //res->set_docids(pages);
                }
            }

            void load_invert_index() {
                std::ifstream Index(INVERT_INDEX_PATH);
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