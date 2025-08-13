#pragma once
#include <string>
#include <ctime>
#include <fstream>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

enum class importances { LOW, MEDIUM, HIGH };
importances parse_importance(const string& msg);

class LogOutput {
public:
    virtual ~LogOutput() = default;
    virtual void write(const string& message) = 0;
    virtual bool is_connected() const = 0;
};

class FileOutput : public LogOutput {
public:
    FileOutput(const string& filename);
    ~FileOutput() override;
    void write(const string& message) override;
    bool is_connected() const override;

private:
    string filename;
    ofstream log_file;
    void reopen();
};

class SocketOutput : public LogOutput {
public:
    SocketOutput(const string& host, int port);
    ~SocketOutput() override;
    void write(const string& message) override;
    bool is_connected() const override;

private:
    string host;
    int port;
#ifdef _WIN32
    SOCKET sockfd = INVALID_SOCKET;
#else
    int sockfd = -1;
#endif
    void connect();
    void disconnect();
};

class Journal_logger {
public:
    Journal_logger(const string& filename, importances importance);
    Journal_logger(const string& host, int port, importances importance);
    ~Journal_logger() = default;

    void message_log(const string& message, importances importance);
    void set_default_importance(importances new_importance);

private:
    unique_ptr<LogOutput> output;
    importances default_importance;

    string format_log(
        const string& message, 
        importances importance, 
        time_t timestamp
    ) const;
};