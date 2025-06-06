#include <iostream>
#include <string>
#include <vector>
#include <sstream>      // 단어 수 세기용
#include <iomanip>      // setw() / left / fixed / setprecision()
#include <fstream>      // 파일 입출력
#include <algorithm>    // 공백 제거(trim)를 위해 필요
using namespace std;

vector<string> avoidance_phrases;

// ==================== Message 클래스 ====================
class Message {
public:
    string sender;    // A or B
    string content;   // 메시지 내용
    int score;        // 감정 점수

    // 세부 항목별 점수 저장 (for later analysis) (항목별 가중치)
    int short_score = 0;
    int question_score = 0;
    int avoid_score = 0;

    Message(string s, string c);
    int analyze();    // 감정 점수 분석 함수
};

// ==================== ChatSession 클래스 ====================
class ChatSession {
private:
    vector<Message> messages;
    int avoid_count_A = 0; // Added: A의 회피 표현 카운트
    int avoid_count_B = 0; // Added: B의 회피 표현 카운트

public:
    void addMessage(string sender, string content);  // 메시지 추가
    void analyzeSession();                           // 전체 대화 분석
    void analyzeSessionPerUser();  // A와 B 각각 분석
};

// ==================== 구체적 생성자, 함수 구현부 ====================

// Message 생성자
Message::Message(string s, string c) : sender(s), content(c) {
    score = analyze();
}

// Message클래스의 점수 계산 함수<analyze()>
int Message::analyze() {
    int penalty = 0;

    // 1. 단답형
    stringstream ss(content);
    string word;
    int word_count = 0;
    while (ss >> word) word_count++;
    if (word_count == 1 && content.find('?') == string::npos) {  // ?가 없거나 단어가 1개일 때 단답형
        short_score = 1;
        penalty += short_score;
    }

    // 2. 질문 없음
    if (content.find('?') == string::npos) {
        question_score = 2;
        penalty += question_score;
    }

    // 3. 회피 표현
    // 회피 표현 감지 (단어 기준으로 정확히 비교)
    stringstream wordstream(content);
    string token;
    while (wordstream >> token) {
        for (const string& avoid : avoidance_phrases) {
            if (token == avoid) {  // 정확히 일치하는 단어만 감지
                avoid_score = 3;
                penalty += avoid_score;
                break;
            }
        }
        if (avoid_score > 0) break;  // 이미 감지되었으면 중단
    }


    return penalty;
}


// ChatSession클래스의 메시지 추가 함수 (입력한 메시지를 실제로 저장하는 핵심 함수) <addMessage()>
void ChatSession::addMessage(string sender, string content) {
    Message m(sender, content);     // 메시지 객체 생성 → 점수 자동 분석됨
    messages.push_back(m);          // 리스트에 추가

    // Added: 회피 표현 사용 시 사용자별 카운트 증가
    if (m.avoid_score > 0) {
        if (sender == "A") {
            avoid_count_A++;
        }
        else if (sender == "B") {
            avoid_count_B++;
        }
    }
}

// ChatSession클래스의 심리 거리 분석 결과출력 함수 <analyzeSession()>
void ChatSession::analyzeSession() {
    int total_messages = messages.size();
    if (total_messages == 0) {
        cout << "입력된 메시지가 없습니다.\n";
        return;
    }

    int total_penalty = 0;
    int max_possible_score = 0;  // 이론적으로 받을 수 있는 최대 점수
    int short_reply = 0;
    int no_question = 0;
    // int avoidance = 0;

    for (const Message& msg : messages) {
        total_penalty += msg.score;
        max_possible_score += 1 + 2 + 3;  // 메시지 하나가 받을 수 있는 최대 점수

        if (msg.short_score > 0) short_reply++;
        if (msg.question_score > 0) no_question++;
        // if (msg.avoid_score > 0) avoidance++;
    }

    double distance = (double)total_penalty / max_possible_score * 100;
    if (distance > 100) distance = 100;

    int total_avoidance = avoid_count_A + avoid_count_B; // Added: 총 회피 표현 계산

    cout << "\n========================== 심리 거리 분석 결과 ==========================\n";
    cout << left << setw(15) << "총 메시지" << ": " << total_messages << " 개\n";
    cout << left << setw(15) << "심리 거리" << ": " << fixed << setprecision(1) << distance << " %\n";

    cout << "\n[ 거리 상승 원인 ]\n";
    if (short_reply > 0)
        cout << "- " << left << setw(10) << "단답형" << ": " << short_reply << " 회\n";
    if (no_question > 0)
        cout << "- " << left << setw(10) << "질문 없음" << ": " << no_question << " 회\n";
    if (total_avoidance > 0) // Modified: 총 회피 표현 사용
        cout << "- " << left << setw(10) << "회피 표현" << ": " << total_avoidance << " 회\n";

    cout << "\n[ 피드백 ]\n";
    if (no_question > 0)
        cout << "- 질문을 더 자주 해보세요.\n";
    if (total_avoidance > 0)   // Modified: 총 회피 표현 사용
        cout << "- 공감어 또는 이모티콘을 사용해보세요.\n";
    if (short_reply > 0 && no_question == 0 && total_avoidance == 0)
        cout << "- 대화를 좀 더 길게 해보세요.\n";
}

// A와 B 각각의 대화를 분석하고 피드백까지 출력하는 함수 <analyzeSessionPerUser()>
void ChatSession::analyzeSessionPerUser() {
    vector<Message> messagesA, messagesB;

    // A와 B의 메시지를 분리
    for (const Message& msg : messages) {
        if (msg.sender == "A") messagesA.push_back(msg);
        else if (msg.sender == "B") messagesB.push_back(msg);
    }

    // 내부 분석 함수
    // Modified: 람다 함수에서 사용자별 회피 카운트를 받도록 수정
    auto analyzeUser = [](const string& name, const vector<Message>& user_msgs, int user_avoid_count) {  // 람다 함수 활용 ★★★★★★★★★★
        int total = user_msgs.size();
        int score = 0, questions = 0, shorts = 0;
        // avoids = 0;

        for (const Message& m : user_msgs) {
            score += m.score;
            if (m.content.find('?') != string::npos) questions++;
            if (m.short_score > 0) shorts++;
            // if (m.avoid_score > 0) avoids++;
        }

        cout << "\n====== [" << name << "] 개인 분석 결과 ======\n";
        cout << "총 메시지      : " << total << " 개\n";
        cout << "질문한 횟수   : " << questions << " 회\n";
        cout << "감정 점수 총합: " << score << " 점\n";
        cout << "단답형         : " << shorts << " 회\n";
        cout << "회피 표현      : " << user_avoid_count << " 회\n";

        // 피드백
        cout << "[피드백]\n";
        if (questions == 0)
            cout << "- 질문을 한 번도 하지 않았어요. 관심 표현을 시도해보세요.\n";
        if (shorts > 0)
            cout << "- 짧은 응답이 많아요. 대화를 길게 이어가면 좋아요.\n";
        if (user_avoid_count > 0)
            cout << "- 회피 표현이 사용됐어요. 감정을 더 표현해보세요.\n";
        if (score == 0)
            cout << "- 매우 긍정적인 대화 스타일이에요!\n";
        };

    // 각각 출력
    analyzeUser("A", messagesA, avoid_count_A);
    analyzeUser("B", messagesB, avoid_count_B);
}

// ==================== 파일에서 회피 표현 로드 함수 ====================
void loadAvoidancePhrases(const string& filename) { // Added: 회피 표현 로드 함수
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[오류] " << filename << " 파일을 열 수 없습니다. 기본 회피 표현을 사용합니다." << endl;
        // 파일 열기 실패 시, 기존 하드코딩된 값 사용 (선택적)
        //avoidance_phrases = { "아무거나", "몰라", "상관없어" };
        return;
    }
    string line;
    int count = 0; // Added: 로드된 개수 세기용 변수
    while (getline(file, line)) {
        // ================== Added: 공백 제거 시작 ==================
        // 오른쪽 공백 제거
        size_t end = line.find_last_not_of(" \t\n\r\f\v");
        if (string::npos != end) {
            line.erase(end + 1);
        }
        else {
            // 줄 전체가 공백이면 빈 문자열로 만듦
            line.clear();
        }

        // 왼쪽 공백 제거
        size_t start = line.find_first_not_of(" \t\n\r\f\v");
        if (string::npos != start) {
            line = line.substr(start);
        }
        else {
            // 줄 전체가 공백이면 빈 문자열로 만듦 (오른쪽에서 이미 처리되었을 수 있음)
            line.clear();
        }
        // ================== Added: 공백 제거 끝 ====================
        if (!line.empty()) {
            avoidance_phrases.push_back(line);
        }
    }
    file.close();
    cout << "[정보] " << avoidance_phrases.size() << "개의 회피 표현을 로드했습니다." << endl; // Added: 로드된 개수 출력
}

// ==================== 메인 함수 ====================
int main() {
    loadAvoidancePhrases("avoid.txt"); // Added: 프로그램 시작 시 회피 표현 로드

    ChatSession session;
    string sender, content;

    cout << "[대화 입력 시작] (종료 시 'p' 입력)\n";

    while (true) {
        // 발신자 입력 받기
        while (true) {
            cout << "누가 말하나요? (A/B): ";
            getline(cin, sender);
            if (sender == "p") {  // 종료조건
                session.analyzeSession();
                session.analyzeSessionPerUser();
                return 0;
            }
            if (sender == "A" || sender == "B") break;
            cout << "[오류] A 또는 B 중에서 선택해주세요.\n";
        }

        cout << "> ";
        getline(cin, content);
        if (content == "p") break;

        session.addMessage(sender, content);
    }

    session.analyzeSession();           // 기존 전체 분석
    session.analyzeSessionPerUser();   // A, B 따로 분석
    return 0;
}
