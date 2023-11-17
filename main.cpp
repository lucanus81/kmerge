#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <iostream>
#include "file.h"
#include "utilities.h"

int main(int argc, char** argv) {
    file f{123};
    split_into_chunks("/opt/test.in", 10, file::FILESIZE);

    seastar::app_template app;
    app.run(argc, argv, [] {
            std::cout << "Hello world\n";
            return seastar::make_ready_future<>();
    });
}

