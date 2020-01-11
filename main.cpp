#include <iostream>
#include <boost/log/trivial.hpp>

#include <boost/config.hpp>

#if !defined(BOOST_WINDOWS)
#define BOOST_LOG_USE_NATIVE_SYSLOG
#endif

#include <stdexcept>
#include <string>
#include <iostream>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/core/null_deleter.hpp>


#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>

#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/syslog_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

//! Define application-specific severity levels
enum severity_levels {
    normal,
    warning,
    error
};

int main(int argc, char **argv) {
    std::cout << "Hello, World!" << std::endl;

    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    // Create a syslog sink
    shared_ptr<sinks::synchronous_sink<sinks::syslog_backend> > sink(
            new sinks::synchronous_sink<sinks::syslog_backend>());

    char format[80];

    char *name;

    name = strrchr(argv[0], '/');

    sprintf(format, "%s: %%1%% :%%2%%:", ++name);

    sink->set_formatter
            (
//                    expr::format("syslog.exe: %1%: %2%")
    expr::format(format)
                    % expr::attr<unsigned int>("RecordID")
                    % expr::smessage
            );

    // We'll have to map our custom levels to the syslog levels
    sinks::syslog::custom_severity_mapping<severity_levels> mapping("Severity");
    mapping[normal] = sinks::syslog::info;
    mapping[warning] = sinks::syslog::warning;
    mapping[error] = sinks::syslog::critical;

    sink->locked_backend()->set_severity_mapper(mapping);

#if !defined(BOOST_LOG_NO_ASIO)
    // Set the remote address to sent syslog messages to
//    sink->locked_backend()->set_target_address("localhost");
    sink->locked_backend()->set_target_address("darkstar");
    std::cout << "Goober" << std::endl;
#endif

    // Add the sink to the core
    logging::core::get()->add_sink(sink);

    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> tsink = boost::make_shared<text_sink>();

    boost::shared_ptr<std::ostream> stream{&std::clog,
                                           boost::null_deleter{}};
    tsink->locked_backend()->add_stream(stream);

    logging::core::get()->add_sink(tsink);

    // Add some attributes too
    logging::core::get()->add_global_attribute("RecordID", attrs::counter<unsigned int>());

    // Do some logging
    src::severity_logger<severity_levels> lg(keywords::severity = normal);
    BOOST_LOG_SEV(lg, normal) << "A syslog record with normal level";
    BOOST_LOG_SEV(lg, warning) << "A syslog record with warning level";
    BOOST_LOG_SEV(lg, error) << "A syslog record with error level";

    return 0;
}
