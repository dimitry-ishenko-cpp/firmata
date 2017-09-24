#ifdef ASIO_STANDALONE
    #include <asio.hpp>
    #include <asio/system_timer.hpp>
#else
    #include <boost/asio.hpp>
    #include <boost/asio/system_timer.hpp>
    namespace asio { using namespace boost::asio; }
    namespace asio { using boost::system::error_code; }
#endif
