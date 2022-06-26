#include "src/html_parser.h"

#include <string>
#include <vector>

int main(int argc, char const* argv[]) {
    std::string data("<html><body><div><span id=\"a\" class=\"x\">a</span><span "
                     "id=\"b\">b</span></div></body></html>");

    auto root = html::HTML(data);

    std::cout << root.html() << std::endl;

    auto x = root.GetElementByClassName("x");

    if (!x.empty()) {
        std::cout << x[0]->text() << std::endl;
        std::cout << x[0]->GetName() << std::endl;
        std::cout << x[0]->GetValue() << std::endl;
    }

    return 0;
}
