/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   jsontest.h
 * Author: dev
 *
 * Created on 18.11.2015, 20:41:37
 */

#ifndef JSONTEST_H
#define JSONTEST_H

#include <cppunit/extensions/HelperMacros.h>

class jsontest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(jsontest);

    CPPUNIT_TEST(testReadConfig);
    CPPUNIT_TEST_SUITE_END();

public:
    jsontest();
    virtual ~jsontest();
    void setUp();
    void tearDown();

private:
    void testReadConfig();

};

#endif /* JSONTEST_H */

