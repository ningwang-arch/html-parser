#include "html_parser.h"

#include <algorithm>
#include <cstring>
#include <string.h>

namespace etree {
std::shared_ptr<HtmlElement> HtmlElement::GetElementById(const std::string& id) {
    for (auto& item : children) {
        if (item->GetAttribute("id") == id) {
            return item;
        }

        auto ret = item->GetElementById(id);
        if (ret) {
            return ret;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<HtmlElement>> HtmlElement::GetElementsByTag(const std::string& tag) {
    std::vector<std::shared_ptr<HtmlElement>> result;
    GetElementByTagName(tag, result);
    return result;
}

std::vector<std::shared_ptr<HtmlElement>> HtmlElement::GetElementsByClass(
    const std::string& class_name) {
    std::vector<std::shared_ptr<HtmlElement>> result;
    GetElementByClassName(class_name, result);
    return result;
}

const std::string& HtmlElement::GetValue() {
    if (tag_value.empty() && children.size() == 1 && children[0]->GetTag() == "plain") {
        return children[0]->GetValue();
    }
    return tag_value;
}

void HtmlElement::PlainStylize(std::string& str) {
    if (tag_name == "head" || tag_name == "script" || tag_name == "style" || tag_name == "meta" ||
        tag_name == "link") {
        return;
    }
    if (tag_name == "text") {
        str += tag_value;
        return;
    }

    for (std::size_t i = 0; i < children.size();) {
        children[i]->PlainStylize(str);

        if (++i < children.size()) {
            std::string ele = children[i]->GetTag();
            if (ele == "td") {
                str.append("\t");
            }
            else if (ele == "tr" || ele == "br" || ele == "div" || ele == "p" || ele == "hr" ||
                     ele == "area" || ele == "h1" || ele == "h2" || ele == "h3" || ele == "h4" ||
                     ele == "h5" || ele == "h6" || ele == "h7") {
                str.append("\n");
            }
        }
    }
}

void HtmlElement::HtmlStylize(std::string& str) {
    if (tag_name.empty()) {
        for (auto& child : children) {
            child->HtmlStylize(str);
        }
        return;
    }
    else if (tag_name == "text") {
        str += tag_value;
        return;
    }

    str.append("<" + tag_name);
    for (auto& attr : attributes) {
        str.append(" " + attr.first + "=\"" + attr.second + "\"");
    }
    str.append(">");
    if (children.empty()) {
        str.append(tag_value);
    }
    else {
        for (auto& child : children) {
            child->HtmlStylize(str);
        }
    }
    str.append("</" + tag_name + ">");
}

void HtmlElement::GetElementByClassName(const std::string& name,
                                        std::vector<std::shared_ptr<HtmlElement>>& result) {
    for (auto& child : children) {
        auto attr_class = SplitClassName(child->GetAttribute("class"));
        auto class_name = SplitClassName(name);
        auto iter = class_name.begin();
        for (; iter != class_name.end(); ++iter) {
            if (std::find(attr_class.begin(), attr_class.end(), *iter) == attr_class.end()) {
                break;
            }
        }
        if (iter == class_name.end()) {
            InsertIfNotExist(result, child);
        }

        child->GetElementByClassName(name, result);
    }
}

void HtmlElement::GetElementByTagName(const std::string& name,
                                      std::vector<std::shared_ptr<HtmlElement>>& result) {
    for (auto& child : children) {
        if (child->GetTag() == name) {
            InsertIfNotExist(result, child);
        }
        child->GetElementByTagName(name, result);
    }
}

void HtmlElement::GetAllElement(std::vector<std::shared_ptr<HtmlElement>>& result) {
    for (auto& child : children) {
        InsertIfNotExist(result, child);
        child->GetAllElement(result);
    }
}

std::set<std::string> HtmlElement::SplitClassName(const std::string& str) {
    std::vector<std::string> result;
    split(str, result, " ");
    return std::set<std::string>(result.begin(), result.end());
}

void HtmlElement::InsertIfNotExist(std::vector<std::shared_ptr<HtmlElement>>& result,
                                   const std::shared_ptr<HtmlElement>& ele) {
    if (std::find(result.begin(), result.end(), ele) == result.end()) {
        result.push_back(ele);
    }
}

void HtmlElement::split(const std::string& str, std::vector<std::string>& result,
                        const std::string& delim) {
    result.clear();
    // use strtok_r to split the string
    char* str_copy = strdup(str.c_str());
    char* token = strtok_r(str_copy, delim.c_str(), &str_copy);
    while (token != nullptr) {
        result.push_back(token);
        token = strtok_r(nullptr, delim.c_str(), &str_copy);
    }
}


void HTML::parse(xmlNodePtr ptr, std::shared_ptr<HtmlElement> parent) {
    if (ptr == nullptr) {
        return;
    }
    std::shared_ptr<HtmlElement> ele = std::make_shared<HtmlElement>();
    ele->parent = parent;
    ele->tag_name = (char*)ptr->name == nullptr ? "" : (char*)ptr->name;
    ele->tag_value = (char*)ptr->content == nullptr ? "" : (char*)ptr->content;
    if (ptr->properties) {
        xmlAttrPtr attr = ptr->properties;
        while (attr) {
            ele->attributes[(char*)attr->name] =
                (char*)xmlNodeListGetString(ptr->doc, attr->children, 1) == nullptr
                    ? ""
                    : (char*)xmlNodeListGetString(ptr->doc, attr->children, 1);
            attr = attr->next;
        }
    }
    if (ptr->children) {
        xmlNodePtr child = ptr->children;
        while (child) {
            parse(child, ele);
            child = child->next;
        }
    }
    parent->children.push_back(ele);
}

std::vector<std::shared_ptr<HtmlElement>> HTML::xpath(const std::string& xpath) {
    xmlXPathContextPtr context = xmlXPathNewContext(doc_);
    if (context == nullptr) {
        throw std::runtime_error("Unable to create new XPath context.");
    }
    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST(xpath.data()), context);
    xmlXPathFreeContext(context);
    std::vector<std::shared_ptr<HtmlElement>> eles;
    if (result->type == XPATH_NODESET) {
        xmlNodeSetPtr nodes = result->nodesetval;
        for (int i = 0; i < nodes->nodeNr; ++i) {
            std::shared_ptr<HtmlElement> ele = std::make_shared<HtmlElement>();
            ele->tag_name =
                (char*)nodes->nodeTab[i]->name == nullptr ? "" : (char*)nodes->nodeTab[i]->name;
            ele->tag_value = (char*)nodes->nodeTab[i]->content == nullptr
                                 ? ""
                                 : (char*)nodes->nodeTab[i]->content;
            if (nodes->nodeTab[i]->properties) {
                xmlAttrPtr attr = nodes->nodeTab[i]->properties;
                while (attr) {
                    ele->attributes[(char*)attr->name] =
                        (char*)xmlNodeListGetString(nodes->nodeTab[i]->doc, attr->children, 1);
                    attr = attr->next;
                }
            }
            if (nodes->nodeTab[i]->children) {
                xmlNodePtr child = nodes->nodeTab[i]->children;
                while (child) {
                    parse(child, ele);
                    child = child->next;
                }
            }
            eles.push_back(ele);
        }
    }
    xmlXPathFreeObject(result);
    return eles;
}

}   // namespace etree