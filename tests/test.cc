#include "src/html_parser.h"


void test_string() {
    std::string html =
        "<html><head></head><body><div class=\"a\"><p id=\"id\">hello</p></div></body></html>";
    auto root = etree::HTML(html);
    std::cout << root.html() << std::endl;
    auto p = root.GetElementsByTag("p");
    for (auto& item : p) { std::cout << item->html() << std::endl; }
    auto a = root.GetElementsByClass("a");
    for (auto& item : a) { std::cout << item->html() << std::endl; }
    auto id = root.GetElementById("id");
    std::cout << id->html() << std::endl;
    std::string xpath = "//p//text()";
    auto p2 = root.xpath(xpath);
    for (auto& item : p2) { std::cout << item->html() << std::endl; }
}

void test_file() {
    auto root = etree::HTML("tests/index.html", true);
    auto ret = root.xpath("//*[@id=\"main\"]/div[3]/ul/li/a/@href");
    for (auto& item : ret) { std::cout << item->text() << std::endl; }
}

int main() {
    test_file();
    return 0;
}