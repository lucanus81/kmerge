# export SEASTAR=/home/splunker/seastar
if [ -z "$SEASTAR" ] ; then
  echo "SEASTAR is not set: I need it to look for include files"
  exit 1
fi

CUSTOM_FLAGS="-DSEASTAR_API_LEVEL=7 -DSEASTAR_SSTRING -DSEASTAR_SCHEDULING_GROUPS_COUNT=16 -DSEASTAR_LOGGER_TYPE_STDOUT -DBOOST_NO_CXX98_FUNCTION_BASE -DFMT_LOCALE -DFMT_SHARED -g"
EXTRA_LIBS="-lboost_program_options -lboost_thread -lcares -lcryptopp -lfmt -lboost_thread -lsctp -luring -lnuma -latomic -lunistring -lhwloc -ludev -lyaml-cpp"
SEASTARLIB="$SEASTAR/build/release/libseastar.a"

g++ main.cpp utilities.cpp -o kmerger $CUSTOM_FLAGS -I "$SEASTAR/build/release/gen/include" -I "$SEASTAR/include" $SEASTARLIB $EXTRA_LIBS
