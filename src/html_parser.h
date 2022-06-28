#ifndef __LEPT_HTML_PARSER_H__
#define __LEPT_HTML_PARSER_H__

#include <iostream>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace etree {

class HTML;

class HtmlElement
{
    friend class HTML;

public:
    HtmlElement(){};
    HtmlElement(std::shared_ptr<HtmlElement> p)
        : parent(p){};

    std::string GetAttribute(const std::string& name) const {
        if (attributes.find(name) != attributes.end()) { return attributes.at(name); }
        return "";
    }

    std::shared_ptr<HtmlElement> GetElementById(const std::string& id);
    std::vector<std::shared_ptr<HtmlElement>> GetElementsByTag(const std::string& tag);
    std::vector<std::shared_ptr<HtmlElement>> GetElementsByClass(const std::string& class_name);

    std::shared_ptr<HtmlElement> GetParent() const { return parent.lock(); }
    const std::string& GetValue();
    const std::string& GetTag() { return tag_name; }

    std::string text() {
        std::string result;
        PlainStylize(result);
        return result;
    }
    void PlainStylize(std::string& str);
    std::string html() {
        std::string result;
        HtmlStylize(result);
        return result;
    }
    void HtmlStylize(std::string& str);

private:
    void GetElementByClassName(const std::string& name,
                               std::vector<std::shared_ptr<HtmlElement>>& result);

    void GetElementByTagName(const std::string& name,
                             std::vector<std::shared_ptr<HtmlElement>>& result);
    void GetAllElement(std::vector<std::shared_ptr<HtmlElement>>& result);

private:
    std::set<std::string> SplitClassName(const std::string& class_name);
    void InsertIfNotExist(std::vector<std::shared_ptr<HtmlElement>>& vec,
                          const std::shared_ptr<HtmlElement>& ele);

    void split(const std::string& str, std::vector<std::string>& tokens, const std::string& delim);

private:
    std::string tag_name;
    std::string tag_value;
    std::map<std::string, std::string> attributes;
    std::vector<std::shared_ptr<HtmlElement>> children;
    std::weak_ptr<HtmlElement> parent;
};

class HTML
{
public:
    HTML(const std::string& html_string, bool is_file = false) {
        if (is_file) {
            doc_ = htmlReadFile(
                html_string.c_str(),
                "UTF-8",
                HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
        }
        else {
            doc_ = htmlReadMemory(
                html_string.data(),
                html_string.size(),
                NULL,
                "UTF-8",
                HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
        }
        if (doc_ == NULL) {
            std::cout << "htmlReadMemory error" << std::endl;
            return;
        }
        root_ = std::make_shared<HtmlElement>();
        xmlNodePtr root = xmlDocGetRootElement(doc_);
        parse(root, root_);
    }

    ~HTML() {
        xmlFreeDoc(doc_);
        xmlCleanupParser();
    }

    std::string text() { return root_->text(); }

    std::string html() { return root_->html(); }

    std::shared_ptr<HtmlElement> GetElementById(const std::string& id) {
        return root_->GetElementById(id);
    }

    std::vector<std::shared_ptr<HtmlElement>> GetElementsByTag(const std::string& tag) {
        return root_->GetElementsByTag(tag);
    }

    std::vector<std::shared_ptr<HtmlElement>> GetElementsByClass(const std::string& class_name) {
        return root_->GetElementsByClass(class_name);
    }

    std::vector<std::shared_ptr<HtmlElement>> xpath(const std::string& xpath);

private:
    void parse(xmlNodePtr ptr, std::shared_ptr<HtmlElement> parent);

private:
    std::shared_ptr<HtmlElement> root_;
    htmlDocPtr doc_;
};

}   // namespace html

#endif