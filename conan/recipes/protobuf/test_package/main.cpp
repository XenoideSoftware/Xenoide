#include <iostream>
#include <string>

#include "test.pb.h"

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    xenoide::test::Greeting out;
    out.set_text("hello from protobuf");

    std::string buffer;
    if (!out.SerializeToString(&buffer)) {
        std::cerr << "SerializeToString failed\n";
        return 1;
    }

    xenoide::test::Greeting in;
    if (!in.ParseFromString(buffer)) {
        std::cerr << "ParseFromString failed\n";
        return 1;
    }

    std::cout << in.text() << '\n';

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
