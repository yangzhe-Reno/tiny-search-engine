/*
* @Author       : hiyoung
* @Date         : 2021/09/04 11:56:47
* @copyleft Apache 2.0
*/

#include "service.pb.h"
#include <unordered_map>
#include <string>
//#include <mysql/mysql.h>
#include "base_service.pb.h"
#include <brpc/controller.h>
#include <brpc/channel.h>
#include "WebPage.h"
#include "tinyxml2.h"
//#include "../connpool/sqlconnpool.h"
//#include "../connpool/sqlconnRAII.h"
//#include "../connpool/redisconnpool.h"
//#include "../connpool/redisconnRAII.h"
using namespace tinyxml2;
namespace NLP {
    class SearchServiceImp: public SearchService
    {
        public:
            SearchServiceImp() {
                nlp_client.Init("127.0.0.1:8003", &op);
                //_redis_pool->init("10.77.200.63", 6379, 5);
                load_pages();
            };

            virtual ~SearchServiceImp() {};
            void query(
                google::protobuf::RpcController *cntl_base,
                const SearchRequest *request,
                SearchResponse *response,
                google::protobuf::Closure *done
            )
            {
                brpc::ClosureGuard guard_done(done);
                brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);
                LOG(INFO) << "query = " << request->query();

                // 分词处理
                NLP::SegmentRequest segment_request;
                NLP::SegmentResponse segment_response;
                NLP::SegmentService_Stub segment_stub(&nlp_client);
                
                brpc::Controller req_cntl;
                segment_request.set_sentence(request->query());
                segment_stub.cut(&req_cntl, &segment_request, &segment_response, NULL);

                //redisContext* rconn;
                //RedisConnRAII(&rconn,  RedisConnPool::getInstance());
                //std::unordered_map<std::string, int> qtf_map;
                //for (auto it: segment_response.result())
                /*
                {
                    LOG(INFO) << "word = " << it.token() << " qtf = " << it.tf();
                    qtf_map[it.token()] = it.tf();

                    // 添加热门查询到redis
                    redisReply *reply;
                    std::string sql = "ZINCRBY myzset 1 " + it.token();
                    reply = (redisReply*)redisCommand(rconn, sql.c_str());
                    freeReplyObject(reply);
                }
                */
                // 召回处理
                NLP::RecallRequest recall_request;
                NLP::RecallResponse recall_response;
                NLP::RecallService_Stub recall_stub(&nlp_client);

                req_cntl.Reset();
                for (auto it: segment_response.result())
                {
                    recall_request.add_tokens(it.token());
                }
                recall_stub.recall(&req_cntl, &recall_request, &recall_response, NULL);
                // for (auto it: recall_response.result())
                // {
                //     LOG(INFO) << "word = " << it.word() << " df = " << it.df();
                //     for (auto i: it.index_item())
                //     {
                //         LOG(INFO) << "docid = " << i.docid() << " dtf = " << i.tf() << " dl = " << i.dl();
                //     }
                // }
                // 排序处理

                NLP::SortRequest sort_request;
                NLP::SortResponse sort_response;
                NLP::SortService_Stub sort_stub(&nlp_client);
                for(auto it : recall_response.docids())
                {
                    sort_request.add_docids(it);
                }

                for (auto it: segment_response.result())
                {
                    auto res = sort_request.add_cut_result();
                    res->set_token(it.token());
                    res->set_tf(it.tf());
                }
                req_cntl.Reset();
                sort_stub.sort(&req_cntl, &sort_request, &sort_response, NULL);

                for(auto it : sort_response.docids())
                {
                    auto res = response->add_result();
                    res->set_docid(it);
                    res->set_title(pageLib_[it].getTitle());
                    //std::vector<std::string> querywords = recall_request.tokens();
                    //res->set_content(pageLib_[it].summary());
                    res->set_url(pageLib_[it].getUrl());
                }

                                    
                //MYSQL* conn;
                //SqlConnRAII(&conn,  SqlConnPool::getInstance());

                /*
                for (auto item: sort_response.result())
                {
                    LOG(INFO) << "DocId = " << item.docid() << " Score = " << item.score();
                    // 查询 mysql
                    MYSQL_RES *res = nullptr;
                    mysql_query(conn, "SET NAMES UTF8");
                    std::string sql = "select * from news where id = " + intToString(item.docid()) + ";";
                    if(mysql_query(conn, sql.c_str())) { 
                        mysql_free_result(res);
                        continue;
                    }
                    res = mysql_store_result(conn);
                    while(MYSQL_ROW row = mysql_fetch_row(res)) {
                        // 结果返回
                        LOG(INFO) << "DocId = " << item.docid() << " title = " << row[1];
                        auto res = response->add_result();
                        res->set_docid(item.docid());
                        res->set_score(item.score());
                        res->set_title(row[1]);
                        res->set_content(row[2]);
                        res->set_url(row[3]);
                    }
                    mysql_free_result(res);
                }
                */
            }

            void hot(
                google::protobuf::RpcController *cntl_base,
                const SearchHotRequest *request,
                SearchHotResponse *response,
                google::protobuf::Closure *done
            )
            {
                /*
                brpc::ClosureGuard guard_done(done);
                brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);

                redisContext* rconn;
                RedisConnRAII(&rconn,  RedisConnPool::getInstance());

                redisReply *reply;
                std::string sql = "zrevrange myzset 0 5";
                reply = (redisReply*)redisCommand(rconn, sql.c_str());
                for (int i=0; i < reply->elements; ++i)
                {
                    auto it = reply->element[i];
                    response->add_result(it->str);
                }
                freeReplyObject(reply);
                */
            }




        private:

            void load_pages()
            {
                XMLDocument doc;
                doc.LoadFile("/home/ubuntu/tiny-search-engine/offline/data/newRipepage.lib");
                XMLElement* page = doc.FirstChildElement("doc");
                XMLError errid = doc.ErrorID();
                if (errid) {
                    LOG(INFO) << "tinyxml load ripepage error";
                    return;
                }

                do {
                    string docid = page->FirstChildElement("docid")->GetText();
                    string title = page->FirstChildElement("title")->GetText();
                    string link = page->FirstChildElement("link")->GetText();
                    string content = page->FirstChildElement("content")->GetText();

                    WebPage webpage(stoi(docid), title, link, content);
                    pageLib_.insert(std::make_pair(stoi(docid), webpage));
                } while (page = page->NextSiblingElement());
                    LOG(INFO) << "load rigepage success";
            }

            brpc::Channel nlp_client;
            brpc::ChannelOptions op;
            std::unordered_map<int, WebPage> pageLib_;
            //SqlConnPool *_sql_pool = SqlConnPool::getInstance();
            //RedisConnPool *_redis_pool = RedisConnPool::getInstance();
            std::string intToString(int v)
            {
                char buf[32] = {0};
                snprintf(buf, sizeof(buf), "%u", v);
            
                std::string str = buf;
                return str;
            }
    };
}