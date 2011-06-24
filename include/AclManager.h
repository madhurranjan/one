/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, OpenNebula Project Leads (OpenNebula.org)             */
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

#ifndef ACL_MANAGER_H_
#define ACL_MANAGER_H_

#include "AuthManager.h"
#include "AclRule.h"

using namespace std;

class AclManager : public ObjectSQL
{
public:
    AclManager(){};

    ~AclManager();

    /* ---------------------------------------------------------------------- */
    /* Rule management                                                        */
    /* ---------------------------------------------------------------------- */

    const bool authorize(int uid, const set<int> &user_groups,
            AuthRequest::Object obj_type, int obj_id, int obj_gid,
            AuthRequest::Operation op);

    /* ---------------------------------------------------------------------- */

    int add_rule(long long user, long long resource, long long rights,
                string& error_str);

    /* ---------------------------------------------------------------------- */

    int del_rule(long long user, long long resource, long long rights,
                string& error_str);

    /* ---------------------------------------------------------------------- */
    /* DB management                                                          */
    /* ---------------------------------------------------------------------- */

    /**
     *  Callback function to unmarshall a PoolObjectSQL
     *    @param num the number of columns read from the DB
     *    @param names the column names
     *    @param vaues the column values
     *    @return 0 on success
     */
    int select_cb(void *nil, int num, char **values, char **names)
    {
        if ( (!values[0]) || (num != 1) )
        {
            return -1;
        }

        // TODO: from_xml

        return 0;
    };

    /**
     *  Reads the ACL rule set from the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int select(SqlDB *db)
    {
        return 0;
    };

    int insert(SqlDB*, std::string&)
    {
        return 0;
    };

    int update(SqlDB*)
    {
        return 0;
    };

    int drop(SqlDB*)
    {
        return 0;
    };

    /* ---------------------------------------------------------------------- */

    int dump(ostringstream& oss);

private:
    multimap<long long, AclRule*> acl_rules;
};

#endif /*ACL_MANAGER_H*/

