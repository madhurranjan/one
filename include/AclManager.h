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

#include "SqlDB.h"

using namespace std;

/**
 *  This class manages the ACL rules and the authorization engine
 */
class AclManager : public Callbackable
{
public:
    AclManager(SqlDB * _db);

    ~AclManager();

    /**
     *  Loads the ACL rule set from the DB
     *    @return 0 on success.
     */
    int start();

    /* ---------------------------------------------------------------------- */
    /* Rule management                                                        */
    /* ---------------------------------------------------------------------- */

    /**
     *  Takes an authorization request and checks if any rule in the ACL
     *  authorizes the operation.
     *
     *    @param uid The user ID requesting to be authorized
     *    @param user_groups Set of group IDs that the user is part of
     *    @param obj_type The object over which the operation will be performed
     *    @param obj_id The object ID
     *    @param obj_gid The object's group ID
     *    @param op The operation to be authorized
     *    @return true if the authorization is granted by any rule
     */
    const bool authorize(int uid, const set<int> &user_groups,
            AuthRequest::Object obj_type, int obj_id, int obj_gid,
            AuthRequest::Operation op);

    /* ---------------------------------------------------------------------- */

    /**
     *  Adds a new rule to the ACL rule set
     *
     *    @param user 64 bit ID and flags
     *    @param resource 64 bit ID and flags
     *    @param rights 64 bit flags
     *    @param error_str Returns the error reason, if any
     *
     *    @return the oid assigned to the rule on success,
     *    -1 if the rule exists,
     *    -2 if the rule is malformed,
     *    -3 if the DB insert failed
     */
    int add_rule(long long user, long long resource, long long rights,
                string& error_str);

    /* ---------------------------------------------------------------------- */

    /**
     *  Deletes a rule from the ACL rule set
     *
     *    @param oid Rule id
     *    @param error_str Returns the error reason, if any
     *    @return 0 on success
     */
    int del_rule(int oid, string& error_str);

    /* ---------------------------------------------------------------------- */
    /* DB management                                                          */
    /* ---------------------------------------------------------------------- */

    /**
     *  Bootstraps the database table(s) associated to the ACL Manager
     */
    static void bootstrap(SqlDB * _db)
    {
        ostringstream oss(db_bootstrap);

        _db->exec(oss);
    };

    /* ---------------------------------------------------------------------- */

    /**
     *  Dumps the rule set in XML format.
     *    @param oss The output stream to dump the rule set contents
     *    @return 0 on success
     */
    int dump(ostringstream& oss);

private:

    // ----------------------------------------
    // ACL rules management
    // ----------------------------------------

    /**
     *  ACL rules. Each rule is indexed by its 'user' long long attibute,
     *  several rules can apply to the same user
     */
    multimap<long long, AclRule*> acl_rules;

    /**
     *  Rules indexed by oid. Stores the same rules as acl_rules
     */
    map<int, AclRule *> acl_rules_oids;

    /**
     *  Gets all rules that apply to the user_req and, if any of them grants
     *  permission, returns true.
     *
     *    @param user_req user/group id and flags
     *    @param resource_oid_req 64 bit request, ob. type and individual oid
     *    @param resource_gid_req 64 bit request, ob. type and group id
     *    @param resource_all_req 64 bit request, ob. type and all flag
     *    @param rights_req Requested rights
     *    @param individual_obj_type Mask with ob. type and individual flags
     *    @param group_obj_type Mask with ob. type and gropu flags
     *
     *    @return true if any rule grants permission
     */
    bool match_rules(
            long long user_req,
            long long resource_oid_req,
            long long resource_gid_req,
            long long resource_all_req,
            long long rights_req,
            long long individual_obj_type,
            long long group_obj_type);

    // ----------------------------------------
    // Mutex synchronization
    // ----------------------------------------

    pthread_mutex_t mutex;

    /**
     *  Function to lock the manager
     */
    void lock()
    {
        pthread_mutex_lock(&mutex);
    };

    /**
     *  Function to unlock the manager
     */
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    };

    // ----------------------------------------
    // DataBase implementation variables
    // ----------------------------------------

    /**
     *  Pointer to the database.
     */
    SqlDB * db;

    /**
     *  Last object ID assigned to a rule.
     */
    int lastOID;

    /**
     *  Tablename for the ACL rules
     */
    static const char * table;

    static const char * db_names;

    static const char * db_bootstrap;

    /**
     *  Inserts the last oid into the pool_control table
     */
    void update_lastOID();

    /**
     *  Callback function to unmarshall the ACL rules
     *    @param num the number of columns read from the DB
     *    @param names the column names
     *    @param vaues the column values
     *    @return 0 on success
     */
    int select_cb(void *nil, int num, char **values, char **names);

    /**
     *  Reads the ACL rule set from the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int select();

    /**
     *  Inserts the ACL rule in the database.
     *    @param rule to insert
     *    @return 0 on success
     */
    int insert(AclRule * rule);

    /**
     *  Drops an ACL rule from the database
     *
     *    @param oid Rule id
     *    @return 0 on success
     */
    int drop(int oid);

    /**
     *  Callback to set the lastOID
     */
    int  init_cb(void *nil, int num, char **values, char **names);
};

#endif /*ACL_MANAGER_H*/

