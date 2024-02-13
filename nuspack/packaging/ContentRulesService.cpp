#include "ContentRulesService.hpp"

#include <regex>

static constexpr uint64_t MAX_CONTENT_LENGTH = 0xBFFFFFFFULL * 0.975; //Hashes take about 2 - 2.5%. Depending on the number of files the alignment/padding is added too. Needs further checks at packing

static uint64_t cur_content_size = 0L;    
static Content* cur_content = nullptr;
static Content* cur_content_first = nullptr;


Content* setNewContentRecursiveRule(std::string path, const std::string& pattern, FSTEntry& cur_entry, Contents& targetContents, const ContentRule& rule) {
    path += cur_entry.name + "/";
    std::regex p(pattern);
    Content* result = nullptr;
            
    if(cur_entry.children.size() == 0){
        std::string filePath = path;
        if(std::regex_match(filePath, p)) {
            result = targetContents.GetNewContent(rule.details);
        }
    }
    for(FSTEntry& child : cur_entry.children){
        if(child.isDir()) {
            result = setNewContentRecursiveRule(path,pattern,child,targetContents,rule);
        }
        else {
            std::string filePath = path + child.name;
            if(std::regex_match(filePath, p)){    
                result = targetContents.GetNewContent(rule.details);
                child.setContent(result);
            }
        }           
    }

    if(result != nullptr){            
        cur_entry.setContent(result);
    }

    return result;
}

bool setContentRecursiveRule(std::string path, const std::string& pattern, FSTEntry& cur_entry, Contents& targetContents, const ContentDetails& contentDetails) {
    path += cur_entry.name + "/";
    std::regex p(pattern);
    bool result = false;
    if(cur_entry.children.size() == 0) {
        std::string filePath = path;
        if(std::regex_match(filePath, p)) {
            if(cur_entry.children.empty()) {
                cur_entry.setContent(cur_content);
            }
           
            return true;
        }
        else {
            return false;
        }
    }
    for(FSTEntry& child : cur_entry.children){
        if(child.isDir()) {
            bool child_result = setContentRecursiveRule(path,pattern,child,targetContents,contentDetails);
            if(child_result) {
                cur_entry.setContent(cur_content_first);
                result = true;
            }
        }
        else {
            std::string filePath = path + child.name;
            if(std::regex_match(filePath, p)) {
                const auto& entry = std::get<FSTEntry::FileEntry>(child.entry);
                if(cur_content_size > 0 && (cur_content_size + entry.fileSize) > MAX_CONTENT_LENGTH) {
                    cur_content = targetContents.GetNewContent(contentDetails);
                    cur_content_size = 0;
                }
                cur_content_size += entry.fileSize;
                
                child.setContent(cur_content);
                result = true;
            }
        }           
    }

    if(result) {
        cur_entry.setContent(cur_content_first);
    }
    
    return result;
}

void applyRules(FSTEntry& root, Contents& targetContents, const ContentRules& rules) {
    for(const ContentRule& rule : rules.rules) {
        if(rule.contentPerMatch) {
            setNewContentRecursiveRule("", rule.pattern, root, targetContents, rule);
        }
        else {
            cur_content = targetContents.GetNewContent(rule.details);
            cur_content_first = cur_content;
            cur_content_size = 0L;

            bool result = setContentRecursiveRule("", rule.pattern, root, targetContents, rule.details);
            if(!result){
                targetContents.DeleteContent(cur_content->index);
            }
            cur_content_first = nullptr;
        }
    }
}
