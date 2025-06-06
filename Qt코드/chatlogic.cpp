#include "chatlogic.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

using namespace std;

vector<string> avoidance_phrases;

// ==================== Message 클래스 구현 ====================

Message::Message(string s, string c) : sender(s), content(c) {
    score = analyze();
}

int Message::analyze() {
    int penalty = 0;

    stringstream ss(content);
    string word;
    int word_count = 0;
    while (ss >> word) word_count++;
    if (word_count == 1 && content.find('?') == string::npos) {
        short_score = 1;
        penalty += short_score;
    }

    if (content.find('?') == string::npos) {
        question_score = 2;
        penalty += question_score;
    }

    stringstream wordstream(content);
    string token;
    while (wordstream >> token) {
        for (const string& avoid : avoidance_phrases) {
            if (token == avoid) {
                avoid_score = 3;
                penalty += avoid_score;
                break;
            }
        }
        if (avoid_score > 0) break;
    }

    return penalty;
}

// ==================== ChatSession 구현 ====================

void ChatSession::addMessage(string sender, string content) {
    Message m(sender, content);
    messages.push_back(m);

    if (m.avoid_score > 0) {
        if (sender == "A") avoid_count_A++;
        else if (sender == "B") avoid_count_B++;
    }
}

string ChatSession::analyzeSession() {
    ostringstream out;
    int total_messages = messages.size();
    if (total_messages == 0) {
        out << "입력된 메시지가 없습니다.\n";
        return out.str();
    }

    int total_penalty = 0;
    int max_score = 0;
    int short_reply = 0;
    int no_question = 0;
    int total_avoidance = avoid_count_A + avoid_count_B;

    for (const Message& msg : messages) {
        total_penalty += msg.score;
        max_score += 6;  // 1+2+3
        if (msg.short_score > 0) short_reply++;
        if (msg.question_score > 0) no_question++;
    }

    double distance = (double)total_penalty / max_score * 100;
    if (distance > 100) distance = 100;

    out << "\n======== 심리 거리 분석 결과 ========\n";
    out << left << setw(15) << "총 메시지" << ": " << total_messages << " 개\n";
    out << left << setw(15) << "심리 거리" << ": " << fixed << setprecision(1) << distance << " %\n";

    out << "\n[ 거리 상승 원인 ]\n";
    if (short_reply > 0)
        out << "- " << left << setw(10) << "단답형" << ": " << short_reply << " 회\n";
    if (no_question > 0)
        out << "- " << left << setw(10) << "질문 없음" << ": " << no_question << " 회\n";
    if (total_avoidance > 0)
        out << "- " << left << setw(10) << "회피 표현" << ": " << total_avoidance << " 회\n";

    out << "\n[ 피드백 ]\n";
    if (no_question > 0)
        out << "- 질문을 더 자주 해보세요.\n";
    if (total_avoidance > 0)
        out << "- 공감어 또는 이모티콘을 사용해보세요.\n";
    if (short_reply > 0 && no_question == 0 && total_avoidance == 0)
        out << "- 대화를 좀 더 길게 해보세요.\n";

    return out.str();
}

string ChatSession::analyzeSessionPerUser() {
    ostringstream out;
    vector<Message> messagesA, messagesB;

    for (const Message& msg : messages) {
        if (msg.sender == "A") messagesA.push_back(msg);
        else if (msg.sender == "B") messagesB.push_back(msg);
    }

    auto analyzeUser = [&out](const string& name, const vector<Message>& msgs, int avoid_count) {
        int total = msgs.size();
        int score = 0, questions = 0, shorts = 0;

        for (const Message& m : msgs) {
            score += m.score;
            if (m.content.find('?') != string::npos) questions++;
            if (m.short_score > 0) shorts++;
        }

        out << "\n====== [" << name << "] 개인 분석 결과 ======\n";
        out << "총 메시지      : " << total << " 개\n";
        out << "질문한 횟수   : " << questions << " 회\n";
        out << "감정 점수 총합: " << score << " 점\n";
        out << "단답형         : " << shorts << " 회\n";
        out << "회피 표현      : " << avoid_count << " 회\n";

        out << "[피드백]\n";
        if (questions == 0)
            out << "- 질문을 한 번도 하지 않았어요. 관심 표현을 시도해보세요.\n";
        if (shorts > 0)
            out << "- 짧은 응답이 많아요. 대화를 길게 이어가면 좋아요.\n";
        if (avoid_count > 0)
            out << "- 회피 표현이 사용됐어요. 감정을 더 표현해보세요.\n";
        if (score == 0)
            out << "- 매우 긍정적인 대화 스타일이에요!\n";
    };

    analyzeUser("A", messagesA, avoid_count_A);
    analyzeUser("B", messagesB, avoid_count_B);
    return out.str();
}

void ChatSession::clear() {
    messages.clear();
    avoid_count_A = 0;
    avoid_count_B = 0;
}

// ==================== 회피 표현 로드 ====================

void loadAvoidancePhrases(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[오류] " << filename << " 파일을 열 수 없습니다." << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        // 오른쪽 공백 제거
        size_t end = line.find_last_not_of(" \t\n\r\f\v");
        if (end != string::npos) line.erase(end + 1);
        else line.clear();

        // 왼쪽 공백 제거
        size_t start = line.find_first_not_of(" \t\n\r\f\v");
        if (start != string::npos) line = line.substr(start);
        else line.clear();

        if (!line.empty()) {
            avoidance_phrases.push_back(line);
        }
    }
    file.close();
}
