//  Hello World server

#include <zmqpp/zmqpp.hpp>

#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "message.pb.h"

using namespace std;

int main(int argc, char *argv[]) {

    const string endpoint = "tcp://*:5555";

    // initialize the 0MQ context
    zmqpp::context context;

    // generate a pull socket
    zmqpp::socket_type type = zmqpp::socket_type::reply;
    zmqpp::socket socket(context, type);

    int id = 0;

    // bind to the socket
    socket.bind(endpoint);
    while (1) {
        // receive the message
        zmqpp::message message;
        // decompose the message
        socket.receive(message);

        string text;
        message >> text;

        // Do some 'work'
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << "Received Hello" << endl;

        // construct the message
        Message protoMessageToSend;
        protoMessageToSend.set_id(++id);
        protoMessageToSend.set_content("World");

        // serialize to a std::vector
        std::vector<char> buffer;
        buffer.resize(protoMessageToSend.ByteSizeLong());
        protoMessageToSend.SerializeToArray(buffer.data(), buffer.size());

        // send it through 0MQ
        socket.send(buffer.data());
    }
}
