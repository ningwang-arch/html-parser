#include "html_parser.h"

#include <algorithm>
#include <cstring>
#include <string.h>

namespace html {
std::string HtmlElement::GetAttribute(const std::string& str) {
    auto it = attributes.find(str);
    if (it != attributes.end()) { return it->second; }
    return "";
}

std::shared_ptr<HtmlElement> HtmlElement::GetElementById(const std::string& id) {
    if (id == GetAttribute("id")) { return shared_from_this(); }
    for (auto& child : children) {
        auto ele = child->GetElementById(id);
        if (ele) { return ele; }
    }
    return nullptr;
}

std::vector<std::shared_ptr<HtmlElement>> HtmlElement::GetElementByClassName(
    const std::string& name) {
    std::vector<std::shared_ptr<HtmlElement>> result;
    GetElementByClassName(name, result);
    return result;
}

std::vector<std::shared_ptr<HtmlElement>> HtmlElement::GetElementByTagName(const std::string& tag) {
    std::vector<std::shared_ptr<HtmlElement>> result;
    GetElementByTagName(tag, result);
    return result;
}


const std::string& HtmlElement::GetValue() {
    if (value.empty() && children.size() == 1 && children[0]->GetName() == "plain") {
        return children[0]->GetValue();
    }

    return value;
}

std::string HtmlElement::text() {
    std::string result;
    PlainStylize(result);
    return result;
}

void HtmlElement::PlainStylize(std::string& str) {
    if (tag == "head" || tag == "meta" || tag == "style" || tag == "link" || tag == "script") {
        return;
    }
    if (tag == "plain") {
        str += value;
        return;
    }
    for (size_t i = 0; i < children.size();) {
        children[i]->PlainStylize(str);
        if (++i < children.size()) {
            std::string element = children[i]->GetName();
            if (element == "td") { str.append("\t"); }
            else if (element == "tr" || element == "br" || element == "div" || element == "p" ||
                     element == "hr" || element == "area" || element == "h1" || element == "h2" ||
                     element == "h3" || element == "h4" || element == "h5" || element == "h6" ||
                     element == "h7") {
                str.append("\n");
            }
        }
    }
}

std::string HtmlElement::html() {
    std::string html;
    HtmlStylize(html);
    return html;
}

void HtmlElement::HtmlStylize(std::string& str) {
    if (tag.empty()) {
        for (size_t i = 0; i < children.size(); i++) { children[i]->HtmlStylize(str); }

        return;
    }
    else if (tag == "plain") {
        str.append(value);
        return;
    }

    str.append("<" + tag);
    std::map<std::string, std::string>::const_iterator it = attributes.begin();
    for (; it != attributes.end(); it++) {
        str.append(" " + it->first + "=\"" + it->second + "\"");
    }
    str.append(">");

    if (children.empty()) { str.append(value); }
    else {
        for (size_t i = 0; i < children.size(); i++) { children[i]->HtmlStylize(str); }
    }

    str.append("</" + tag + ">");
}

void HtmlElement::GetElementByClassName(const std::string& name,
                                        std::vector<std::shared_ptr<HtmlElement>>& result) {
    for (auto& item : children) {
        std::set<std::string> attr_class = SplitClassName(item->GetAttribute("class"));
        std::set<std::string> name_class = SplitClassName(name);

        for (auto& item_class : name_class) {
            if (attr_class.find(item_class) != attr_class.end()) {
                InsertIfNotExists(result, item);
                break;
            }
        }
        item->GetElementByClassName(name, result);
    }
}

void HtmlElement::GetElementByTagName(const std::string& tag,
                                      std::vector<std::shared_ptr<HtmlElement>>& result) {
    for (auto& item : children) {
        if (item->GetName() == tag) { InsertIfNotExists(result, item); }
        item->GetElementByTagName(tag, result);
    }
}

void HtmlElement::Parse(const std::string& str) {
    std::size_t index = 0;
    std::string attr_name;
    std::string attr_value;
    char split = ' ';
    bool quote = false;


    State state = ATTR_KEY;
    while (index < str.size()) {
        char ch = str[index];
        switch (state) {
        case ATTR_KEY:
        {
            if (ch == '\t' || ch == '\r' || ch == '\n') {}
            else if (ch == '\'' || ch == '"') {
                std::cerr << "error: attribute value is not supported" << std::endl;
            }
            else if (ch == ' ') {
                if (!attr_name.empty()) {
                    attributes[attr_name] = attr_value;
                    attr_name.clear();
                }
            }
            else if (ch == '=') {
                state = ATTR_VALUE_BEGIN;
            }
            else {
                attr_name.append(1, ch);
            }
        } break;
        case ATTR_VALUE_BEGIN:
        {
            if (ch == '\t' || ch == '\r' || ch == '\n' || ch == ' ') {
                if (!attr_name.empty()) {
                    attributes[attr_name] = attr_value;
                    attr_name.clear();
                }
                state = ATTR_KEY;
            }
            else if (ch == '\'' || ch == '"') {
                quote = true;
                split = ch;
                state = ATTR_VALUE_END;
            }
            else {
                attr_value.append(1, ch);
                quote = false;
                state = ATTR_VALUE_END;
            }
        } break;
        case ATTR_VALUE_END:
        {
            if ((quote && ch == split) ||
                (!quote && (ch == '\t' || ch == '\r' || ch == '\n' || ch == ' '))) {
                attributes[attr_name] = attr_value;
                attr_name.clear();
                attr_value.clear();
                state = ATTR_KEY;
            }
            else {
                attr_value.append(1, ch);
            }

        } break;
        }
        index++;
    }
    if (!attr_name.empty()) { attributes[attr_name] = attr_value; }
    if (!value.empty()) {
        value.erase(0, value.find_first_not_of(" "));
        value.erase(value.find_last_not_of(" ") + 1);
    }
}

std::set<std::string> HtmlElement::SplitClassName(const std::string& str) {
    std::vector<std::string> result;
    split(str, result, " ");
    std::set<std::string> result_set;
    for (auto& item : result) { result_set.insert(item); }
    return result_set;
}


void HtmlElement::InsertIfNotExists(std::vector<std::shared_ptr<HtmlElement>>& result,
                                    const std::shared_ptr<HtmlElement>& ele) {
    for (auto& item : result) {
        if (item == ele) { return; }
    }
    result.push_back(ele);
}

void HtmlElement::split(const std::string& str, std::vector<std::string>& tokens,
                        const std::string delim) {
    if (str.empty()) { return; }
    tokens.clear();

    char* buffer = new char[str.size() + 1];
    std::strcpy(buffer, str.c_str());

    char* tmp;
    char* p = strtok_r(buffer, delim.c_str(), &tmp);
    do { tokens.push_back(p); } while ((p = strtok_r(nullptr, delim.c_str(), &tmp)) != nullptr);
    delete[] buffer;
}

HtmlParser::HtmlParser() {
    static const std::string self_close_tags[] = {"area",
                                                  "base",
                                                  "br",
                                                  "col",
                                                  "command",
                                                  "embed",
                                                  "hr",
                                                  "img",
                                                  "input",
                                                  "keygen",
                                                  "link",
                                                  "meta",
                                                  "param",
                                                  "source",
                                                  "track",
                                                  "wbr"};
    for (auto& item : self_close_tags) { _self_closing_tags.insert(item); }
}

std::shared_ptr<HtmlElement> HtmlParser::Parse(const std::string& html) {
    return Parse(html.data(), html.size());
}

std::shared_ptr<HtmlElement> HtmlParser::Parse(const char* html, std::size_t len) {
    _stream = html;
    _length = len;
    std::size_t index = 0;
    _root.reset(new HtmlElement());
    while (index < _length) {
        char ch = _stream[index];
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ') {
            index++;
            continue;
        }
        else if (ch == '<') {
            index = ParseElement(index, _root);
        }
        else {
            break;
        }
    }
    return std::shared_ptr<HtmlElement>(_root);
}

std::size_t HtmlParser::ParseElement(std::size_t index, std::shared_ptr<HtmlElement>& element) {
    while (_length > index) {
        char input = _stream[index + 1];
        if (input == '!') {
            if (strncmp(_stream + index, "<!--", 4) == 0) { return SkipUntil(index + 2, "-->"); }
            else {
                return SkipUntil(index + 2, '>');
            }
        }
        else if (input == '/') {
            return SkipUntil(index, '>');
        }
        else if (input == '?') {
            return SkipUntil(index, "?>");
        }

        std::shared_ptr<HtmlElement> self(new HtmlElement(element));


        State state = TAG_BEGIN;
        index++;
        std::string attr;

        while (_length > index) {
            switch (state) {
            case TAG_BEGIN:
            {
                char input = _stream[index];
                if (input == ' ' || input == '\r' || input == '\n' || input == '\t') {
                    if (!self->tag.empty()) { state = ATTR; }
                    index++;
                }
                else if (input == '/') {
                    self->Parse(attr);
                    element->children.push_back(self);
                    return SkipUntil(index, '>');
                }
                else if (input == '>') {
                    if (_self_closing_tags.find(self->tag) != _self_closing_tags.end()) {
                        element->children.push_back(self);
                        return ++index;
                    }
                    state = VALUE;
                    index++;
                }
                else {
                    self->tag.append(_stream + index, 1);
                    index++;
                }
            } break;

            case ATTR:
            {
                char input = _stream[index];
                if (input == '>') {
                    if (_stream[index - 1] == '/') {
                        attr.erase(attr.size() - 1);
                        self->Parse(attr);
                        element->children.push_back(self);
                        return ++index;
                    }
                    else if (_self_closing_tags.find(self->tag) != _self_closing_tags.end()) {
                        self->Parse(attr);
                        element->children.push_back(self);
                        return ++index;
                    }
                    state = VALUE;
                    index++;
                }
                else {
                    attr.append(_stream + index, 1);
                    index++;
                }
            } break;

            case VALUE:
            {
                if (self->tag == "script" || self->tag == "noscript" || self->tag == "style") {
                    std::string close = "</" + self->tag + ">";

                    size_t pre = index;
                    index = SkipUntil(index, close.c_str());
                    if (index > (pre + close.size()))
                        self->value.append(_stream + pre, index - pre - close.size());

                    self->Parse(attr);
                    element->children.push_back(self);
                    return index;
                }

                char input = _stream[index];
                if (input == '<') {
                    if (!self->value.empty()) {
                        std::shared_ptr<HtmlElement> child(new HtmlElement(self));
                        child->tag = "plain";
                        child->value.swap(self->value);
                        self->children.push_back(child);
                    }

                    if (_stream[index + 1] == '/') { state = TAG_END; }
                    else {
                        index = ParseElement(index, self);
                    }
                }
                else if (input != '\r' && input != '\n' && input != '\t') {
                    self->value.append(_stream + index, 1);
                    index++;
                }
                else {
                    index++;
                }
            } break;

            case TAG_END:
            {
                index += 2;
                std::string selfname = self->tag + ">";
                if (strncmp(_stream + index, selfname.c_str(), selfname.size())) {
                    size_t pre = index;
                    index = SkipUntil(index, ">");
                    std::string value;
                    if (index > (pre + 1))
                        value.append(_stream + pre, index - pre - 1);
                    else
                        value.append(_stream + pre, index - pre);

                    std::shared_ptr<HtmlElement> parent = self->GetParent();
                    while (parent) {
                        if (parent->tag == value) {
                            std::cerr << "WARN : element not closed <" << self->tag << "> "
                                      << std::endl;
                            self->Parse(attr);
                            element->children.push_back(self);
                            return pre - 2;
                        }

                        parent = parent->GetParent();
                    }

                    std::cerr << "WARN : unexpected closed element </" << value << "> for <"
                              << self->tag << ">" << std::endl;
                    state = VALUE;
                }
                else {
                    self->Parse(attr);
                    element->children.push_back(self);
                    return SkipUntil(index, '>');
                }
            } break;
            }
        }
    }

    return index;
}

std::size_t HtmlParser::SkipUntil(std::size_t index, const char* data) {
    while (index < _length) {
        if (strncmp(_stream + index, data, strlen(data)) == 0) { return index + strlen(data); }
        else {
            index++;
        }
    }
    return index;
}

std::size_t HtmlParser::SkipUntil(std::size_t index, const char data) {
    while (index < _length) {
        if (_stream[index] == data) { return ++index; }
        else {
            index++;
        }
    }
    return index;
}

}   // namespace html