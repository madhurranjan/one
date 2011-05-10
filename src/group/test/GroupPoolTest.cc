/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, OpenNebula Project Leads (OpenNebula.org)             */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include <string>
#include <iostream>
#include <stdlib.h>

#include "GroupPool.h"
#include "PoolTest.h"

using namespace std;

const int uids[] = {0,1,2};
const string names[] = {"First name", "Second name"};

const string xmls[] =
{
    "<GROUP><ID>1</ID><UID>0</UID><NAME>First name</NAME></GROUP>",
    "<GROUP><ID>2</ID><UID>1</UID><NAME>Second name</NAME></GROUP>"
};

const string group_xml_dump =
    "<GROUP_POOL><GROUP><ID>0</ID><UID>0</UID><NAME>default</NAME></GROUP><GROUP><ID>1</ID><UID>5</UID><NAME>group_a</NAME></GROUP><GROUP><ID>3</ID><UID>5</UID><NAME>group_c</NAME></GROUP><GROUP><ID>4</ID><UID>5</UID><NAME>group_d</NAME></GROUP></GROUP_POOL>";

/* ************************************************************************* */
/* ************************************************************************* */


#include "NebulaTest.h"

class NebulaTestGroup: public NebulaTest
{
public:
    NebulaTestGroup():NebulaTest()
    {
        NebulaTest::the_tester = this;

        need_group_pool= true;
    }
};

class GroupPoolTest : public PoolTest
{
    CPPUNIT_TEST_SUITE (GroupPoolTest);

    // Not all tests from PoolTest can be used. Because
    // of the initial default group added to the DB, the
    // oid_assignment would fail.
    CPPUNIT_TEST (get_from_cache);
    CPPUNIT_TEST (get_from_db);
    CPPUNIT_TEST (wrong_get);
    CPPUNIT_TEST (drop_and_get);

    CPPUNIT_TEST (duplicates);
    CPPUNIT_TEST (dump);

    CPPUNIT_TEST_SUITE_END ();

protected:

    NebulaTestGroup * tester;
    GroupPool *       gpool;



    void bootstrap(SqlDB* db)
    {
        // setUp overwritten
    };

    PoolSQL* create_pool(SqlDB* db)
    {
        // setUp overwritten
        return gpool;
    };

    int allocate(int index)
    {
        int    oid;
        string err;

        return gpool->allocate(uids[index], names[index], &oid, err);
    };

    void check(int index, PoolObjectSQL* obj)
    {
        Group * group = static_cast<Group *>(obj);

        CPPUNIT_ASSERT( obj != 0 );

        string xml_str = "";
        string name = group->get_name();

        CPPUNIT_ASSERT( name == names[index] );

        // Get the xml
        group->to_xml(xml_str);

//  A little help for debugging
/*
        if( xml_str != xmls[index] )
        {
            cout << endl << xml_str << endl << "========"
                 << endl << xmls[index];
        }
//*/
        CPPUNIT_ASSERT( xml_str == xmls[index]);
    };

private:


public:
    GroupPoolTest()
    {
        xmlInitParser();
    };

    ~GroupPoolTest()
    {
        xmlCleanupParser();
    };

    void setUp()
    {
        create_db();

        tester = new NebulaTestGroup();

        Nebula& neb = Nebula::instance();
        neb.start();

        gpool   = neb.get_gpool();

        pool    = gpool;
    };

    void tearDown()
    {
        delete_db();

        delete tester;
    };

    /* ********************************************************************* */
    /* ********************************************************************* */

    void duplicates()
    {
        int         rc, oid;
        string      err;
        GroupPool * gpool = static_cast<GroupPool*>(pool);

        // Allocate a group
        rc = gpool->allocate(uids[0], names[0], &oid, err);
        CPPUNIT_ASSERT( oid == 1 );
        CPPUNIT_ASSERT( oid == rc );

        // Try to allocate twice the same group, should fail
        rc = gpool->allocate(uids[0], names[0], &oid, err);
        CPPUNIT_ASSERT( rc  == -1 );
        CPPUNIT_ASSERT( oid == rc );

        // Try again, this time with different uid. Should fail, groups can't
        // repeat names
        rc = gpool->allocate(uids[1], names[0], &oid, err);
        CPPUNIT_ASSERT( rc  == -1 );
        CPPUNIT_ASSERT( oid == rc );
    }

    /* ********************************************************************* */

    void dump()
    {
        Group *         group;
        int             oid, rc;
        ostringstream   oss;
        string          err;

        // Allocate some groups
        rc = gpool->allocate(5, "group_a", &oid, err);
        CPPUNIT_ASSERT( rc == 1 );

        rc = gpool->allocate(5, "group_b", &oid, err);
        CPPUNIT_ASSERT( rc == 2 );

        rc = gpool->allocate(5, "group_c", &oid, err);
        CPPUNIT_ASSERT( rc == 3 );

        rc = gpool->allocate(5, "group_d", &oid, err);
        CPPUNIT_ASSERT( rc == 4 );

        // Drop one of them
        group = gpool->get(2, false);
        CPPUNIT_ASSERT( group != 0 );

        rc = gpool->drop(group);
        CPPUNIT_ASSERT( rc == 0 );

        // dump the pool
        rc = gpool->dump(oss,"");
/*
        if( oss.str() != group_xml_dump )
        {
            cout << endl << oss.str() << endl << "========"
                 << endl << group_xml_dump;
        }
//*/
        CPPUNIT_ASSERT( oss.str() == group_xml_dump );

    }
};

/* ************************************************************************* */
/* ************************************************************************* */

int main(int argc, char ** argv)
{
    return PoolTest::main(argc, argv, GroupPoolTest::suite());
}
