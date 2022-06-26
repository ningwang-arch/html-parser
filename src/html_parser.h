#ifndef __HTML_PARSER_H__
#define __HTML_PARSER_H__

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace html {
class HtmlParser;

class HtmlElement : public std::enable_shared_from_this<HtmlElement>
{
    friend class HtmlParser;

public:
    typedef std::shared_ptr<HtmlElement> Ptr;

public:
    HtmlElement() {}
    HtmlElement(std::shared_ptr<HtmlElement> p)
        : parent(p) {}

    std::string GetAttribute(const std::string& str);
    std::shared_ptr<HtmlElement> GetElementById(const std::string& id);
    std::vector<std::shared_ptr<HtmlElement>> GetElementByClassName(const std::string& name);
    std::vector<std::shared_ptr<HtmlElement>> GetElementByTagName(const std::string& tag);
    std::shared_ptr<HtmlElement> GetParent() { return parent.lock(); }
    const std::string& GetValue();
    const std::string& GetName() { return tag; }
    std::string text();
    void PlainStylize(std::string& str);
    std::string html();
    void HtmlStylize(std::string& str);

private:
    void GetElementByClassName(const std::string& name,
                               std::vector<std::shared_ptr<HtmlElement>>& result);

    void GetElementByTagName(const std::string& name,
                             std::vector<std::shared_ptr<HtmlElement>>& result);

    void GetAllElement(std::vector<std::shared_ptr<HtmlElement>>& result);

    void Parse(const std::string& attr);

    std::set<std::string> SplitClassName(const std::string& name);

    void InsertIfNotExists(std::vector<std::shared_ptr<HtmlElement>>& result,
                           const std::shared_ptr<HtmlElement>& ele);

private:
    void split(const std::string& str, std::vector<std::string>& tokens, const std::string delim);

private:
    enum State
    {
        ATTR_KEY,
        ATTR_VALUE_BEGIN,
        ATTR_VALUE_END,
    };

    std::string tag;
    std::string value;
    std::map<std::string, std::string> attributes;
    std::vector<std::shared_ptr<HtmlElement>> children;
    std::weak_ptr<HtmlElement> parent;
};

class HtmlParser
{
public:
    HtmlParser();
    std::shared_ptr<HtmlElement> Parse(const char* html, std::size_t len);
    std::shared_ptr<HtmlElement> Parse(const std::string& html);

private:
    std::size_t ParseElement(std::size_t index, std::shared_ptr<HtmlElement>& element);
    std::size_t SkipUntil(std::size_t index, const char* data);
    std::size_t SkipUntil(std::size_t index, const char data);

private:
    enum State
    {
        TAG_BEGIN,
        ATTR,
        VALUE,
        TAG_END
    };

    const char* _stream;
    std::size_t _length;
    std::set<std::string> _self_closing_tags;
    std::shared_ptr<HtmlElement> _root;
};

class HTML
{
public:
    HTML(std::shared_ptr<HtmlElement>& root)
        : _root(root) {}
    HTML(const std::string& html) { _root = HtmlParser().Parse(html); }

    std::shared_ptr<HtmlElement> GetElementById(const std::string& id) {
        return _root->GetElementById(id);
    }
    std::vector<std::shared_ptr<HtmlElement>> GetElementByClassName(const std::string& name) {
        return _root->GetElementByClassName(name);
    }
    std::vector<std::shared_ptr<HtmlElement>> GetElementByTagName(const std::string& tag) {
        return _root->GetElementByTagName(tag);
    }
    std::string text() { return _root->text(); }
    std::string html() { return _root->html(); }

private:
    std::shared_ptr<HtmlElement> _root;
};


}   // namespace html

#endif