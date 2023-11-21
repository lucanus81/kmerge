#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <iostream>
#include "file.h"
#include "utilities.h"

int main(int argc, char** argv) {
    create_test_file("/home/splunker/test.in", 10);
    size_t chunks = split_into_chunks("/home/splunker/test.in", 4);
    in_memory_file_manager manager{"/home/splunker/test.in", chunks};
/*
    seastar::app_template app;
    app.run(argc, argv, [] {
            std::cout << "Hello world\n";
            return seastar::make_ready_future<>();
    });
  */
}

