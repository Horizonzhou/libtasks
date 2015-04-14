/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>

#include "test_http_sender.h"
#include "test_disk_io_task.h"
#include "test_socket.h"
#include "test_uwsgi_thrift.h"
#include "test_uwsgi_thrift_async.h"
#include "test_bitset.h"
#include "test_exec.h"
#include "test_timer_task.h"

#include <tasks/dispatcher.h>
#include <tasks/executor.h>

#include <string>

using namespace tasks;

CPPUNIT_TEST_SUITE_REGISTRATION(test_http_sender);
CPPUNIT_TEST_SUITE_REGISTRATION(test_disk_io_task);
CPPUNIT_TEST_SUITE_REGISTRATION(test_socket);
CPPUNIT_TEST_SUITE_REGISTRATION(test_uwsgi_thrift);
CPPUNIT_TEST_SUITE_REGISTRATION(test_uwsgi_thrift_async);
CPPUNIT_TEST_SUITE_REGISTRATION(test_bitset);
CPPUNIT_TEST_SUITE_REGISTRATION(test_exec);
CPPUNIT_TEST_SUITE_REGISTRATION(test_timer_task);

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "multi") {
        dispatcher::init_run_mode(dispatcher::mode::MULTI_LOOP);
    }
    // use 4 worker threads
    dispatcher::init_workers(4);
    // reduce the idle timeout for executor threads for the tests
    executor::set_timeout(5);

    dispatcher::instance()->start();

    // informs test-listener about testresults
    CPPUNIT_NS::TestResult testresult;

    // register listener for collecting the test-results
    CPPUNIT_NS::TestResultCollector collectedresults;
    testresult.addListener(&collectedresults);

    // register listener for per-test progress output
    CPPUNIT_NS::BriefTestProgressListener progress;
    testresult.addListener(&progress);

    // insert test-suite at test-runner by registry
    CPPUNIT_NS::TestRunner testrunner;
    testrunner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    testrunner.run(testresult);

    // output results in compiler-format
    CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write();

    // Output XML for Jenkins CPPunit plugin
    std::ofstream xmlFileOut("libtasks_results.xml");
    CPPUNIT_NS::XmlOutputter xmlOut(&collectedresults, xmlFileOut);
    xmlOut.write();

    tasks::dispatcher::destroy();

    // return 0 if tests were successful
    return collectedresults.wasSuccessful() ? 0 : 1;
}
