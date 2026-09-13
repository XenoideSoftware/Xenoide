
#include <string>
#include <iostream>

#include "person.pb.h"

int main(int argc, char *argv[]) {
    Person person;
    person.set_id(123);
    person.set_name("Doe");
    person.set_email("email@gmail.com");
    person.set_phone("102-241-5342");

    std::cout << "hello, world" << std::endl;
    std::cout << person.id() << ", " << person.name() << ", " << person.email() << ", " << person.phone() << std::endl;

    std::string output;
    person.SerializeToString(&output);

    std::cout << output << std::endl;

    return 0;
}
