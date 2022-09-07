# tiny-search-engine
## 功能介绍：
- 离线处理部分：使用simhash算法库对网页进行去重，使用CppJieba库进行中文分词，使用TF-IDF算法构建网页库的倒排索引
- 在线服务部分：为了实现在线服务的解耦，使用brpc框架将在线服务划分为NLP Service和Search Service，其中NLP service被细分为查询分词、召回、排序三个服务，Search Service通过调用NLP Service将检索结果返回前端展示
- 使用flask框架搭建前端展示页面，并使用Redis缓存查询结果，以及统计热点查询
