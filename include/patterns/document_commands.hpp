/**
 * @file include/patterns/document_commands.hpp
 * @brief 검색 결과 [5] "Document editor" MetaLoki 버전
 */

#pragma once

#include <patterns/command.hpp>
#include <string>
#include <iostream>

namespace patterns::examples {
    
    /**
     * @brief 검색 결과 [5] "Document" Receiver 구현
     */
    class document : public command_receiver<document> {
    private:
        std::string content_;
        std::string clipboard_;
        
    public:
        explicit document(std::string initial_content = "") 
            : content_(std::move(initial_content)) {}
        
        // Document 작업들
        void copy() {
            clipboard_ = content_;
            std::cout << "Copied: '" << content_ << "'" << std::endl;
        }
        
        void paste() {
            content_ += clipboard_;
            std::cout << "Pasted. Content: '" << content_ << "'" << std::endl;
        }
        
        void cut() {
            clipboard_ = content_;
            content_.clear();
            std::cout << "Cut. Content cleared." << std::endl;
        }
        
        void append_text(const std::string& text) {
            content_ += text;
            std::cout << "Appended '" << text << "'. Content: '" << content_ << "'" << std::endl;
        }
        
        void clear() {
            content_.clear();
            std::cout << "Document cleared." << std::endl;
        }
        
        const std::string& content() const { return content_; }
        const std::string& clipboard() const { return clipboard_; }
        
        void set_content(const std::string& content) { content_ = content; }
    };
    
    /**
     * @brief 검색 결과 [2] "POD structs" 스타일 Commands
     */
    
    // Copy Command
    struct copy_command {
        document* doc;
        
        explicit copy_command(document* d) : doc(d) {}
        
        void execute() {
            if (doc) doc->copy();
        }
        
        std::string description() const {
            return "Copy Document";
        }
    };
    
    // Paste Command
    struct paste_command {
        document* doc;
        std::string previous_content; // Undo용
        
        explicit paste_command(document* d) : doc(d) {}
        
        void execute() {
            if (doc) {
                previous_content = doc->content(); // Undo를 위해 저장
                doc->paste();
            }
        }
        
        void undo() {
            if (doc) {
                doc->set_content(previous_content);
                std::cout << "Undo paste. Restored content: '" << previous_content << "'" << std::endl;
            }
        }
        
        bool can_undo() const { return true; }
        
        std::string description() const {
            return "Paste Document";
        }
    };
    
    // Cut Command
    struct cut_command {
        document* doc;
        std::string previous_content; // Undo용
        
        explicit cut_command(document* d) : doc(d) {}
        
        void execute() {
            if (doc) {
                previous_content = doc->content(); // Undo를 위해 저장
                doc->cut();
            }
        }
        
        void undo() {
            if (doc) {
                doc->set_content(previous_content);
                std::cout << "Undo cut. Restored content: '" << previous_content << "'" << std::endl;
            }
        }
        
        bool can_undo() const { return true; }
        
        std::string description() const {
            return "Cut Document";
        }
    };
    
    // Append Text Command
    struct append_text_command {
        document* doc;
        std::string text_to_append;
        std::string previous_content;
        
        append_text_command(document* d, std::string text) 
            : doc(d), text_to_append(std::move(text)) {}
        
        void execute() {
            if (doc) {
                previous_content = doc->content();
                doc->append_text(text_to_append);
            }
        }
        
        void undo() {
            if (doc) {
                doc->set_content(previous_content);
                std::cout << "Undo append. Restored content: '" << previous_content << "'" << std::endl;
            }
        }
        
        bool can_undo() const { return true; }
        
        std::string description() const {
            return "Append Text: '" + text_to_append + "'";
        }
    };
    
    /**
     * @brief 검색 결과 [5] "Menu_Item" 스타일 Button
     * @details Command를 실행하는 UI 요소
     */
    template<typename CommandType>
    class button {
    private:
        std::string name_;
        CommandType command_;
        
    public:
        button(std::string name, CommandType command)
            : name_(std::move(name)), command_(std::move(command)) {}
        
        void click() {
            std::cout << "Button '" << name_ << "' clicked." << std::endl;
            command_.execute();
        }
        
        const std::string& name() const { return name_; }
        const CommandType& command() const { return command_; }
    };
}
