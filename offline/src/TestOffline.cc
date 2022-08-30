#include "DirScanner.h"
#include "PageLib.h"
#include "PageLibPreprocessor.h"
#include "tinyxml2.h"
using namespace wd;
using namespace tinyxml2;

int main() {
    Configuration::getInstance("/home/ubuntu/tiny-search-engine/offline/conf/offline.conf");
    DirScanner scanner;
    scanner();
    PageLib pagelib(scanner);
    pagelib.create();

    PageLibPreprocessor processer;
    processer.doProcess();

    return 0;
}
