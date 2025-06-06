// chatlogic.h
#ifndef CHATLOGIC_H
#define CHATLOGIC_H

#include <string>
#include <vector>

// 전역 회피 표현 벡터
extern std::vector<std::string> avoidance_phrases;

class Message {
public:
    std::string sender;
    std::string content;
    int score;
    int short_score = 0;
    int question_score = 0;
    int avoid_score = 0;

    Message(std::string s, std::string c);
    int analyze();
};

class ChatSession {
private:
    std::vector<Message> messages;
    int avoid_count_A = 0;
    int avoid_count_B = 0;

public:
    void addMessage(std::string sender, std::string content);
    std::string analyzeSession(); // GUI용 문자열 반환
    std::string analyzeSessionPerUser(); // GUI용 문자열 반환
    void clear();
};

// 회피 표현 불러오기
void loadAvoidancePhrases(const std::string& filename);

#endif // CHATLOGIC_H
