export SEASTAR=/home/splunker/seastar

CUSTOM_FLAGS="-DSEASTAR_API_LEVEL=7 -DSEASTAR_SSTRING -DSEASTAR_SCHEDULING_GROUPS_COUNT=16 -DSEASTAR_LOGGER_TYPE_STDOUT -DBOOST_NO_CXX98_FUNCTION_BASE -DFMT_LOCALE -DFMT_SHARED"
EXTRA_LIBS="-lboost_program_options -lboost_thread -lcares -lcryptopp -lfmt -lboost_thread -lsctp -luring -lnuma -latomic -lunistring -lhwloc -ludev -lyaml-cpp"
SEASTARLIB="$SEASTAR/build/release/libseastar.a"

g++ main.cpp -o kmerger $CUSTOM_FLAGS -I "$SEASTAR/build/release/gen/include" -I "$SEASTAR/include" $SEASTARLIB $EXTRA_LIBS
