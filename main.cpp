#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

enum Type {
	warning,
	error,
	fatal_error,
	unknown_message
};

class LogMessage {
public:
	LogMessage(Type type, std::string& messageString) : type_(type), messageString_(messageString) {};
	Type type() const {
		return type_;
	};
	const std::string& message() const {
		return messageString_;
	};
private:
	Type type_;
	std::string& messageString_;
};

class Handler {
public:
	virtual void handling(LogMessage logMessage) = 0;
	virtual ~Handler() {};
};

class Chain : public Handler {
private:
	Chain* next_chain;
public:
	Chain(Chain* chain) : next_chain(chain) {};

	void handling(LogMessage logMessage) override {
		if (next_chain)
			next_chain->handling(logMessage);
	}
	Chain* addNext(Chain* chain) {
		if (next_chain)
			addNext(chain);
		else
			next_chain = chain;
		return next_chain;
	}
};

class Warning : public Chain {
public:
	Warning(Chain* c = nullptr) : Chain(c) {};
	void handling(LogMessage logMessage) override {
		if (logMessage.type() == warning)
			std::cout << "Warning: " << logMessage.message() << std::endl;
		else
			Chain::handling(logMessage);
	}
};

class Error : public Chain {
public:
	Error(std::ofstream& file, Chain* chain = nullptr) : file_(file), Chain(chain) {};

	void handling(LogMessage logMessage) override {
		if (logMessage.type() == error)
			if (file_.is_open()) {
				file_ << "Error: " << logMessage.message() << std::endl;
			}
			else
				std::cout << "File not open" << std::endl;
		else
			Chain::handling(logMessage);
	}
private:
	std::ofstream& file_;
};

class FatalError : public Chain {
public:
	FatalError(Chain* c = nullptr) : Chain(c) {};
	void handling(LogMessage logMessage) override {
		if (logMessage.type() == fatal_error)
			throw std::runtime_error{ "Fatal error: " + logMessage.message() };
		else
			Chain::handling(logMessage);
	}
};

class UnknownMessage : public Chain {
public:
	UnknownMessage(Chain* c = nullptr) : Chain(c) {};
	void handling(LogMessage logMessage) override {
		if (logMessage.type() != error && logMessage.type() != warning && logMessage.type() != fatal_error)
			throw std::runtime_error{ "Unknown message: " + logMessage.message() };
	}
};


int main(int argc, char* argv[]) {

	std::ofstream file("file.txt");
	std::string message = "Hi for everybody!";
	LogMessage logMessage(unknown_message, message);

	Warning warning;
	Error error(file);
	FatalError fatalEror;
	UnknownMessage unknownMessage;

	try {
		warning.addNext(&error)->addNext(&fatalEror)->addNext(&unknownMessage);
		warning.handling(logMessage);
	}
	catch (const std::runtime_error& e) {
		std::cout << e.what();
	}

	return 0;
}

