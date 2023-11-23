#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <iostream>
#include "file.h"
#include "utilities.h"
#include <vector>

#include <seastar/core/future.hh>

/*    create_test_file("/home/splunker/test.in", 10);
    size_t chunks = split_into_chunks("/home/splunker/test.in", 3);
    in_memory_file_manager manager{"/home/splunker/test.in", chunks};
    for (auto i = manager.begin(); i!=manager.end(); ++i) {
      std::cout <<"===== START =========\n";
      size_t b{0};
      for (auto j=i->begin(); j!=i->end(); ++j)
        std::cout <<b++ <<": " <<j->key <<'\n';
      std::cout <<"===== END ========\n";
    }

seastar::future<std::vector<int>> f() {
  std::vector<int> v{1,2,3,4,5};
  return seastar::make_ready_future<std::vector<int>>(std::move(v));
}

seastar::future<std::vector<int>> init() {
  std::vector<int> v{1,2,3,4,5};
  return seastar::make_ready_future<std::vector<int>>(std::move(v));
}

seastar::future<> process_elements(std::vector<int> v) {
  return seastar::do_with(std::move(v), [](std::vector<int>& v) {
      return seastar::do_for_each(v, [](int i) { std::cout <<"Processing " <<i <<'\n'; });
      }).then([]{ std::cout <<"Finishd to process all elements in a loop\n"; });
}
*/

namespace bpo = boost::program_options;

struct file_with_chunk_info {
  std::string source_file_path;
  size_t tot_created_files;
};

seastar::future<file_with_chunk_info> wrapper_split_into_chunks(std::string source_file_path, size_t max_records_per_file) {
  size_t total_created_files = split_into_chunks(source_file_path.c_str(), max_records_per_file);
  file_with_chunk_info result{ std::move(source_file_path), total_created_files };
  return seastar::make_ready_future<file_with_chunk_info>(std::move(result));
}

int main(int argc, char** argv) {
    create_test_file("/home/splunker/test.in", 10);
    size_t chunks = split_into_chunks("/home/splunker/test.in", 3);
    in_memory_file_manager manager{"/home/splunker/test.in", chunks};
    manager.merge_to("/home/splunker/test.in.merged");

    /*
    for (auto i = manager.begin(); i!=manager.end(); ++i) {
      std::cout <<"===== START =========\n";
      size_t b{0};
      for (auto j=i->begin(); j!=i->end(); ++j)
        std::cout <<b++ <<": " <<j->key <<'\n';
      std::cout <<"===== END ========\n";
    }*/

  /*
    seastar::app_template app;
    app.add_options()
      ("source_file_path", bpo::value<std::string>(), "Large binary file path")
      ("max_records_per_file", bpo::value<size_t>()->default_value(100), "Max records in each intermediate file created");
    app.run(argc, argv, [&] {
      auto&& config = app.configuration();
      std::string source_file_path = config["source_file_path"].as<std::string>();
      size_t max_records_per_file = config["max_records_per_file"].as<size_t>();
      std::cout << "Processing " <<source_file_path <<", and max_records = " <<max_records_per_file <<'\n';
      return wrapper_split_into_chunks(std::move(source_file_path), max_records_per_file)
        .then([](file_with_chunk_info result) {
          std::cout <<"Finished to process " <<result.source_file_path <<": created " <<result.tot_created_files <<" temporary files\n"; 
        })
        .then([]{ return seastar::make_ready_future<>(); });
    });
    */
}

