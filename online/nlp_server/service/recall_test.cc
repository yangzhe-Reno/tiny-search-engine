#include "base_service.pb.h"
#include <brpc/controller.h>
#include <brpc/channel.h>
#include <iostream>
int main()
{
    brpc::Channel nlp_client;
    brpc::ChannelOptions op;
    nlp_client.Init("127.0.0.1:8003", &op);
    NLP::RecallRequest recall_request;
    NLP::RecallResponse recall_response;
    NLP::RecallService_Stub recall_stub(&nlp_client);
    brpc::Controller req_cntl;
    recall_request.add_tokens("北京");
    recall_stub.recall(&req_cntl, &recall_request, &recall_response, NULL);
    for(auto it : recall_response.docids())
    {
        LOG(INFO) << it;
    }
    std::cout << "sdfads" << std::endl;
}